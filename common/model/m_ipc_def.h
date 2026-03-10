#ifndef CLT_OPTIC_M_IPC_DEF_H
#define CLT_OPTIC_M_IPC_DEF_H

#include <cstdlib>
#include <memory>
#include <chrono>

using ipc_time_point = std::chrono::time_point<std::chrono::steady_clock>;

//оповіщення від camerapro на datapro1 про готовність наступного кадру
struct ipc_next_frame_notify
{
	size_t sh_mem_size;			//повний розмір shared-mem
	size_t frame_index;			//номер кадру - сквозна нумерація
	size_t frame_size;			//розмір кадру. по факту розмір самого кадру + структура Чайковського
	size_t frame_ptr_offset;	//оффсет відносно початку shared-mem, де розміщено цей кадр
	ipc_time_point  cam_pro_time;		//мітка часу що зроблена в камерапро (буде використана для підрахунку часу обробки)
};

class malloc_auto_free_t{
public:
	void operator() (void* ptr){free(ptr);}
};

using ipc_rc_data_store = std::unique_ptr<void, malloc_auto_free_t>;

#endif //CLT_OPTIC_M_IPC_DEF_H
