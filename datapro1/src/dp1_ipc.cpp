#include <log4cxx/logger.h>
#include "dp1_ipc.h"
#include <fcntl.h>        /* Defines O_* constants */
#include <sys/stat.h>    /* Defines mode constants */
#include <sys/mman.h>
#include <mqueue.h>
#include <cstring>
#include <unistd.h>
#include <thread>

using std::string;
static log4cxx::LoggerPtr logger;

namespace ns_datapro1 {

	void init_ipc_logger(int cam_index) {
		logger = log4cxx::Logger::getLogger("dp1-" + std::to_string(cam_index) + ".ipc");
	}

	class ipc_data_rc_impl : public ipc_data_rc {

		string shmem_obj_name_;
		string queue_obj_name_;

		mqd_t queue_obj_ = 0;
		size_t queue_msg_size_ = 0;

		int shmem_obj_ = 0;
		void *sh_mem_addr_ = nullptr;
		size_t shmem_size_ = 0;
		std::thread rc_thread_;
		std::function<void(ipc_rc_data_store, std::size_t frame_size, std::size_t frame_ipc_index, ipc_time_point)> new_frame_notifier_;
		std::atomic<bool> rc_stop;

		bool (*need_prg_stop_)();

		void open_shmem_object();

		void open_queue_object();

		void rc_data_thread_func();

		void proc_rd_msg_from_queue(const char *q_msg_data, size_t msg_size);

		bool remap_sh_mem(size_t new_sh_mem_size);

	public:
		explicit ipc_data_rc_impl(const prg_config &cfg, bool (*need_prg_stop)());

		~ipc_data_rc_impl() override;

		void receive_data(std::function<void(ipc_rc_data_store, std::size_t frame_size, std::size_t frame_ipc_index, ipc_time_point)>) override;

	};

	static std::unique_ptr<ipc_data_rc_impl> data_receiver;

	std::unique_ptr<ipc_data_rc> get_ipc_data_receiver(const prg_config &cfg, bool (*need_stop)()) {
		return std::make_unique<ipc_data_rc_impl>(cfg, need_stop);
	}

	ipc_data_rc_impl::ipc_data_rc_impl(const prg_config &cfg, bool (*need_prg_stop)()) :
			shmem_obj_name_{cfg.ipc_cfg.get_shmem_obj_name()},
			queue_obj_name_{cfg.ipc_cfg.get_queue_obj_name()},
			need_prg_stop_(need_prg_stop){
		LOG4CXX_INFO(logger, "shmem-name: " << shmem_obj_name_ << ", queue-name: " << queue_obj_name_);
		open_shmem_object();
		if(need_prg_stop_())
			return;
		open_queue_object();
	}

	ipc_data_rc_impl::~ipc_data_rc_impl() {
		//after we obtained addr, shmem_obj_ doesn't need anymore, so we can close it
		if (close(shmem_obj_) == -1)
			LOG4CXX_ERROR(logger, "on close shared-mem object");
		else
			LOG4CXX_DEBUG(logger, "shared-mem object closed");
		if (rc_thread_.joinable()) {
			rc_stop = true;
			LOG4CXX_INFO(logger, "wait for stop rc_thread");
			rc_thread_.join();
			LOG4CXX_INFO(logger, "rc_thread joined");
		}
	}

	void ipc_data_rc_impl::open_shmem_object() {
		using namespace std::chrono_literals;
		while ((shmem_obj_ = shm_open(shmem_obj_name_.c_str(), O_RDONLY, S_IRUSR | S_IWUSR)) == -1) {
			if(errno == ENOENT){
				LOG4CXX_WARN(logger, "can't open sh-mem object: " << shmem_obj_name_ << " due to it's absent, wait some time and try again");
				std::this_thread::sleep_for(500ms);
				if(need_prg_stop_())
					return;
			}else {
				string errstr{strerror(errno)};
				LOG4CXX_FATAL(logger, "can't open shared-mem object: " << shmem_obj_name_ << " - " << errstr);
				throw std::logic_error("can't open shared-mem object: " + shmem_obj_name_ + " - " + errstr);
			}
		}
		LOG4CXX_INFO(logger, "shared-mem object created, id: " << shmem_obj_);
	}

	void ipc_data_rc_impl::open_queue_object() {
		using namespace std::chrono_literals;
		bool was_open_on_first_try{true};
		while((queue_obj_ = mq_open(queue_obj_name_.c_str(), O_NONBLOCK | O_RDONLY, S_IRUSR | S_IWUSR, nullptr)) == -1)
		{
			if(errno == ENOENT){
				LOG4CXX_WARN(logger, "can't open posix-queue: " << queue_obj_name_ << " due to it's absent, wait some time and try again");
				was_open_on_first_try = false;
				std::this_thread::sleep_for(500ms);
				if(need_prg_stop_())
					return;
			}else {
				string errstr{strerror(errno)};
				LOG4CXX_FATAL(logger, "can't open posix-queue object: " << queue_obj_name_ << " - " << errstr);
				throw std::logic_error("can't open posix-queue object: " + queue_obj_name_ + " - " + errstr);
			}
		}
		LOG4CXX_INFO(logger, "posix-queue object created, id: " << queue_obj_);

		mq_attr attr{};
		if (mq_getattr(queue_obj_, &attr) == -1) {
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't get attr for posix-queue object, err.: " << errstr);
			throw std::logic_error("can't get attr for posix-queue object, err.: " + errstr);
		}
		queue_msg_size_ = attr.mq_msgsize;

		if (attr.mq_curmsgs > 1 || (was_open_on_first_try && attr.mq_curmsgs)) {
			LOG4CXX_INFO(logger, "there already are " << attr.mq_curmsgs << " old messages in queue, try to read them");
			std::vector<char> tmp_rd_buff(attr.mq_msgsize);
			unsigned int num_rd_old_msg{0};
			ssize_t numRead;
			while ((numRead = mq_receive(queue_obj_, tmp_rd_buff.data(), attr.mq_msgsize, nullptr)) != -1) {
				++num_rd_old_msg;
				LOG4CXX_DEBUG(logger, "read message " << numRead << " bytes");
			}
			LOG4CXX_INFO(logger, "read " << num_rd_old_msg << " old messages from queue");
		}

		//set queue in blocking mode
		attr.mq_flags &= ~O_NONBLOCK;
		if (mq_setattr(queue_obj_, &attr, nullptr) == -1) {
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't set attr for posix-queue object, err.: " << errstr);
			throw std::logic_error("can't set attr for posix-queue object, err.: " + errstr);
		}
	}

	void ipc_data_rc_impl::receive_data(std::function<void(ipc_rc_data_store, std::size_t frame_size, std::size_t frame_ipc_index, ipc_time_point)> notifier) {
		if (rc_thread_.joinable()) {
			LOG4CXX_FATAL(logger, "try to enter into receive func for second time");
			throw std::logic_error("try to enter into receive func for second time");
		}
		if(need_prg_stop_()) {
			LOG4CXX_INFO(logger, "stop detected, don't start rc thread");
			return;
		}
		new_frame_notifier_ = notifier;
		rc_stop = false;
		rc_thread_ = std::thread([=, this]() { rc_data_thread_func(); });
	}

	void ipc_data_rc_impl::rc_data_thread_func() {
		LOG4CXX_INFO(logger, "start reading thread");
		timespec time{};
		std::vector<char> tmp_rd_buff(queue_msg_size_);
		//тайм-аут читання з черги. Ставим трохи більше, ніж плануємо інтервал між кадрами
		const long int read_time_out_ns = 250'000'000;    //250 ms
		while (!rc_stop) {
			clock_gettime(CLOCK_REALTIME, &time);
			time.tv_nsec += read_time_out_ns;
			if (time.tv_nsec > 999'999'999) {
				time.tv_nsec -= 1'000'000'000;
				time.tv_sec += 1;
			}
			ssize_t numRead = mq_timedreceive(queue_obj_, tmp_rd_buff.data(), queue_msg_size_, nullptr, &time);
			if(rc_stop)
				break;
			if (numRead == -1) {
				if (errno == ETIMEDOUT) {
					LOG4CXX_WARN(logger, "time-out on read from queue");
				} else {
					string errstr{strerror(errno)};
					LOG4CXX_ERROR(logger, "error on read data from queue: " << errstr);
					throw std::logic_error("error on read data from queue: " + errstr);
				}
			} else
				proc_rd_msg_from_queue(tmp_rd_buff.data(), numRead);
		}
		LOG4CXX_INFO(logger, "reading thread - stop detected");
	}

	void ipc_data_rc_impl::proc_rd_msg_from_queue(const char *q_msg_data, size_t msg_size) {
		using namespace std::chrono;
		if (sizeof(ipc_next_frame_notify) != msg_size) {
			LOG4CXX_ERROR(logger, "rc wrong size msg: " << msg_size << ", expected: " << sizeof(ipc_next_frame_notify));
			return;
		}

		const auto *notify = reinterpret_cast<const ipc_next_frame_notify *>(q_msg_data);
		auto ipc_start_time = steady_clock::now();
		uint64_t delay{0};
		if(ipc_start_time < notify->cam_pro_time) {
			delay = duration_cast<microseconds>(notify->cam_pro_time - ipc_start_time).count();
			LOG4CXX_WARN(logger, "rc frame time point earlier than create on (mcs): " << delay);
		}else{
			uint64_t delay = duration_cast<microseconds>(ipc_start_time - notify->cam_pro_time).count();
			if(delay < 100000)	//якщо дуже велика різниця - не будемо використовувати, рахуємо, що ці steady_clock різні
				ipc_start_time = notify->cam_pro_time;
		}
		LOG4CXX_INFO(logger, "rc next frame notify, sh-mem-size: " << notify->sh_mem_size
																   << ", fr-index: " << notify->frame_index
																   << ", fr-size: " << notify->frame_size
																   << ", fr-offset: " << notify->frame_ptr_offset
																   << ", time (мкс): " << delay);
		if(notify->sh_mem_size != shmem_size_) {
			if(!remap_sh_mem(notify->sh_mem_size))
				return;
		}
		const auto* frame_sh_mem_addr = static_cast<const uint8_t *>(sh_mem_addr_);
		frame_sh_mem_addr += notify->frame_ptr_offset;
		ipc_rc_data_store frame_data{malloc(notify->frame_size)};
		if(!frame_data){
			LOG4CXX_FATAL(logger, "can't get mem to copy frame");
			return;
		}
		LOG4CXX_INFO(logger, "ready to copy frame to local mem");
		memcpy(frame_data.get(), frame_sh_mem_addr, notify->frame_size);
		LOG4CXX_INFO(logger, "copied");
		new_frame_notifier_(std::move(frame_data), notify->frame_size, notify->frame_index, ipc_start_time);
	}

	bool ipc_data_rc_impl::remap_sh_mem(size_t new_sh_mem_size){
		if(shmem_size_) {
			int res = munmap(sh_mem_addr_, shmem_size_);
			if(res == -1){
				string errstr{strerror(errno)};
				LOG4CXX_FATAL(logger, "munmap returned error: " << errstr);
				return false;
			}
		}
		shmem_size_ = 0;
		sh_mem_addr_ = mmap(nullptr, new_sh_mem_size, PROT_READ, MAP_SHARED, shmem_obj_, 0);
		if (sh_mem_addr_ == MAP_FAILED) {
			sh_mem_addr_ = nullptr;
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't get shared-mem addr: " << errstr);
			return false;
		}
		LOG4CXX_INFO(logger, "got pointer to shared-mem object: " << sh_mem_addr_);
		shmem_size_ = new_sh_mem_size;
		return true;
	}
}
