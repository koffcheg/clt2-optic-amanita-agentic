#include <gtest/gtest.h>
#include "m_cfg_if.h"

TEST(test_config, ipc_naming) {
	{
		ipc_name_cfg name_cfg(0);
		EXPECT_STREQ(name_cfg.get_shmem_obj_name(), "/clt_optic_cam-dp1_0");
		EXPECT_STREQ(name_cfg.get_queue_obj_name(), "/clt_optic_cam_q_0");
	}
	{
		ipc_name_cfg name_cfg(1);
		EXPECT_STREQ(name_cfg.get_shmem_obj_name(), "/clt_optic_cam-dp1_1");
		EXPECT_STREQ(name_cfg.get_queue_obj_name(), "/clt_optic_cam_q_1");
	}
	{
		ipc_name_cfg name_cfg(10);
		EXPECT_STREQ(name_cfg.get_shmem_obj_name(), "/clt_optic_cam-dp1_10");
		EXPECT_STREQ(name_cfg.get_queue_obj_name(), "/clt_optic_cam_q_10");
	}
	{
		ipc_name_cfg name_cfg(13);
		EXPECT_STREQ(name_cfg.get_shmem_obj_name(), "/clt_optic_cam-dp1_13");
		EXPECT_STREQ(name_cfg.get_queue_obj_name(), "/clt_optic_cam_q_13");
	}
}
