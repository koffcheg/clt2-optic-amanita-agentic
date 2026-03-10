#include "cp_ipc_cam2dp1_if.h"
#include <memory>
#include <log4cxx/logger.h>
#include <fcntl.h>        /* Defines O_* constants */
#include <sys/stat.h>    /* Defines mode constants */
#include <sys/mman.h>
#include <mqueue.h>
#include <unistd.h>
#include <cstring>
#include "m_ipc_def.h"

using std::string;

//todo: param!!!
const size_t max_num_frame_in_sh_mem = 4;
static auto logger = log4cxx::Logger::getLogger("cam-pro.ipc");

namespace cam_pro {
	class ipc_data_tr_impl : public ipc_data_tr {
		enum class en_last_action {
			la_got_mem_addr,
			la_fwd_request,
		};
		size_t shmem_size_ = 0;
		size_t frame_size_;
		string shmem_obj_name_;
		string queue_obj_name_;
		mqd_t queue_obj_ = 0;
		void *sh_mem_addr = nullptr;
		en_last_action last_action = en_last_action::la_fwd_request;
		size_t next_frame_index = 0;
		size_t last_chunk_index = 0;

		void open_shmem_object();

		void open_queue_object();

	public:
		ipc_data_tr_impl(std::size_t frame_size, const Config &cfg);

		~ipc_data_tr_impl() override;

		void *get_prt_next_frame() override;

		bool tr_formed_frame() override;

		std::size_t frame_size() override;
	};

	static std::unique_ptr<ipc_data_tr_impl> data_forwarder;

	ipc_data_tr *get_ipc_data_forwarder(std::size_t frame_size, const Config &cfg) {
		if (!data_forwarder)
			data_forwarder = std::make_unique<ipc_data_tr_impl>(frame_size, cfg);
		return data_forwarder.get();
	}

	ipc_data_tr_impl::ipc_data_tr_impl(std::size_t frame_size, const Config &cfg) :
			frame_size_{frame_size},
			shmem_obj_name_{cfg.ipc_cfg.get_shmem_obj_name()},
			queue_obj_name_{cfg.ipc_cfg.get_queue_obj_name()} {
		LOG4CXX_INFO(logger, "frame-size: " << frame_size << ", shmem-name: " << shmem_obj_name_ << ", queue-name: "
											<< queue_obj_name_);
		open_shmem_object();
		open_queue_object();
	}

	ipc_data_tr_impl::~ipc_data_tr_impl() {
		if (queue_obj_ != -1) {
			if (mq_close(queue_obj_) == -1)
				LOG4CXX_ERROR(logger, "on close queue object");
			else
				LOG4CXX_DEBUG(logger, "queue object closed");
		}
	}

	void *ipc_data_tr_impl::get_prt_next_frame() {
		last_chunk_index = next_frame_index++ % max_num_frame_in_sh_mem;
		char *res = static_cast<char *>(sh_mem_addr);
		res += frame_size_ * last_chunk_index;
		LOG4CXX_INFO(logger,
					 "next mem-request, fr-index: " << next_frame_index - 1 << ", chunk-index: " << last_chunk_index
													<< ", ptr: " << static_cast<void*>(res));
		if (last_action != en_last_action::la_fwd_request)
			LOG4CXX_ERROR(logger, "wrong actions-order");
		last_action = en_last_action::la_got_mem_addr;
		return res;
	}

	bool ipc_data_tr_impl::tr_formed_frame() {
		LOG4CXX_INFO(logger, "next frame fwd-request, fr-index: " << next_frame_index - 1 << ", chank-index "
																  << last_chunk_index);
		if (last_action != en_last_action::la_got_mem_addr)
			LOG4CXX_ERROR(logger, "wrong actions-order");
		last_action = en_last_action::la_fwd_request;
		ipc_next_frame_notify notify{shmem_size_, next_frame_index - 1, frame_size_, frame_size_ * last_chunk_index, std::chrono::steady_clock::now()};
		int send_res = mq_send(queue_obj_, reinterpret_cast<const char *>(&notify), sizeof(notify), 0);
		if (send_res == -1) {
			if (errno == EAGAIN)
				LOG4CXX_ERROR(logger, "ipc queue is full");
			else {
				string errstr{strerror(errno)};
				LOG4CXX_ERROR(logger, "can't put next-frame notify into queue: " << errstr);
			}
		}
		return send_res == 0;
	}

	std::size_t ipc_data_tr_impl::frame_size(){
		return frame_size_;
	}

	void ipc_data_tr_impl::open_shmem_object() {
		int shmem_obj = shm_open(shmem_obj_name_.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		if (shmem_obj == -1) {
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't open shared-mem object: " << shmem_obj_name_ << " - " << errstr);
			throw std::logic_error("can't open shared-mem object: " + shmem_obj_name_ + " - " + errstr);
		}
		LOG4CXX_INFO(logger, "shared-mem object created, id: " << shmem_obj);
		shmem_size_ = frame_size_ * max_num_frame_in_sh_mem;
		if (ftruncate(shmem_obj, static_cast<off_t>(shmem_size_)) == -1) {
			LOG4CXX_FATAL(logger, "can't resize shared-mem object to size: " << shmem_size_);
			throw std::logic_error("can't resize shared-mem object to size: " + std::to_string(shmem_size_));
		}
		LOG4CXX_INFO(logger, "changed shared-mem object size to " << shmem_size_);
		sh_mem_addr = mmap(nullptr, shmem_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shmem_obj, 0);
		if (sh_mem_addr == MAP_FAILED) {
			sh_mem_addr = nullptr;
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't get shared-mem addr: " << errstr);
			throw std::logic_error("can't get shared-mem addr: " + errstr);
		}
		LOG4CXX_INFO(logger, "got pointer to shared-mem object: " << sh_mem_addr);
		//after we obtained addr, shmem_obj_ doesn't need anymore, so we can close it
		if (close(shmem_obj) == -1)
			LOG4CXX_ERROR(logger, "on close shared-mem object");
		else
			LOG4CXX_DEBUG(logger, "shared-mem object closed");
	}

	void ipc_data_tr_impl::open_queue_object() {
		mq_attr attr{};
		attr.mq_maxmsg = 10;
		attr.mq_msgsize = 1024;
		queue_obj_ = mq_open(queue_obj_name_.c_str(), O_NONBLOCK | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
		if (queue_obj_ == -1) {
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't open posix-queue object: " << queue_obj_name_ << " - " << errstr);
			throw std::logic_error("can't open posix-queue object: " + queue_obj_name_ + " - " + errstr);
		}
		LOG4CXX_INFO(logger, "posix-queue object created, id: " << queue_obj_);

		if (mq_getattr(queue_obj_, &attr) == -1) {
			string errstr{strerror(errno)};
			LOG4CXX_FATAL(logger, "can't get attr for posix-queue object, err.: " << errstr);
			throw std::logic_error("can't get attr for posix-queue object, err.: " + errstr);
		}

		if(attr.mq_curmsgs){
			LOG4CXX_INFO(logger, "there already are " << attr.mq_curmsgs << " old messages in queue, try to read them");
			std::vector<char> tmp_rd_buff(attr.mq_msgsize);
			unsigned int num_rd_old_msg{0};
			ssize_t numRead{};
			while((numRead = mq_receive(queue_obj_, tmp_rd_buff.data(), attr.mq_msgsize, nullptr)) != -1) {
				++num_rd_old_msg;
				LOG4CXX_DEBUG(logger, "read message " << numRead << " bytes");
			}
			LOG4CXX_INFO(logger, "read " << num_rd_old_msg << " old messages from queue");
		}
	}
}    //for namespace