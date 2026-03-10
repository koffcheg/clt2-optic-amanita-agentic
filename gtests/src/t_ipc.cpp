#include <gtest/gtest.h>
#include "m_ipc_def.h"

//code for testing automatic freing memory
//just like prog-freer, but with counter
static unsigned int num_my_free_func_calls{};

static void tmp_free_like_func(void *ptr) {
	++num_my_free_func_calls;
	free(ptr);
}

class malloc_auto_test_free_t {
public:
	void operator()(void *ptr) { tmp_free_like_func(ptr); }
};

using ipc_rc_test_data_store = std::unique_ptr<void, malloc_auto_test_free_t>;

TEST(test_ipc, data_types) {
	{
		const size_t frame_size = 60 * 1024 * 1024;
		ipc_rc_data_store frame_data{malloc(frame_size)};
		EXPECT_TRUE(frame_data.get() != nullptr);
	}
	unsigned int num_test_free = num_my_free_func_calls;
	{
		const size_t frame_size = 60 * 1024 * 1024;
		ipc_rc_test_data_store frame_data{malloc(frame_size)};
		EXPECT_TRUE(frame_data.get() != nullptr);
	}
	++num_test_free;
	EXPECT_EQ(num_my_free_func_calls, num_test_free);
}
