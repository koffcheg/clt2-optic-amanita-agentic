//
// Created by u on 10.10.24.
//
#include <gtest/gtest.h>
#include "dp1_snail_path.h"
#include "dp1_config.h"
#include "dp1_calc_limit.h"

using std::vector;
using std::pair;
using std::map;
using namespace ns_datapro1;

TEST(test_snail_path, general) {
	{
		snail_path nail(1, 1);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 1);
		vector<pair<std::size_t, std::size_t>> expected{{0, 0}};
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(2, 2);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 4);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{{0, 1}, {0, 0}, {1, 0}, {1, 1}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(3, 3);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 9);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{{1, 1}, {1, 2}, {0, 2}, {0, 1}, {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(4, 4);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 16);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{
				{1, 2}, {1, 1},
				{2, 1},
				{2, 2}, {2, 3},
				{1, 3}, {0, 3},
				{0, 2}, {0, 1}, {0, 0},
				{1, 0}, {2, 0}, {3, 0},
				{3, 1}, {3, 2}, {3, 3}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(2, 3);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 6);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{{0, 2}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {1, 2}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(3, 2);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 6);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{{1, 1}, {0, 1}, {0, 0}, {1, 0}, {2, 0}, {2, 1}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(3, 4);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 12);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{{1, 1}, {1, 2}, {1, 3}, {0, 3}, {0, 2}, {0, 1}, {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
	{
		snail_path nail(5, 5);
		auto path = nail.get_path();
		EXPECT_EQ(path.size(), 25);
// @formatter:off
		vector<pair<std::size_t, std::size_t>> expected{
			{2, 2}, {2, 3},
			{1, 3},
			{1, 2}, {1, 1},
			{2, 1}, {3, 1},
			{3, 2}, {3, 3}, {3, 4},
			{2, 4}, {1, 4}, {0, 4},
			{0, 3}, {0, 2}, {0, 1}, {0, 0},
			{1, 0}, {2, 0}, {3, 0}, {4, 0},
			{4, 1}, {4, 2}, {4, 3}, {4, 4}};
// @formatter:on
		EXPECT_EQ(path, expected);
	}
}

TEST(test_proc_tile_limit, cr_statuses_func) {
	{
		//empty rates - all precess enables, low time-rate
		vector<unsigned int> frame_priorities{0, 1, 2, 3, 4, 5, 6};
		map<double, double> rates;
		auto res = calc_need_fr_proc_statuses(0.0, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		for (auto el: res) {
			EXPECT_EQ(el, true);
		}
	}
	{
		//empty rates - all process enables, big time-rate
		vector<unsigned int> frame_priorities{0, 1, 2, 3, 4, 5, 6};
		map<double, double> rates;
		auto res = calc_need_fr_proc_statuses(2.5, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		for (auto el: res) {
			EXPECT_EQ(el, true);
		}
	}
	{
		//empty rates - all process enables - inverse frame priority
		vector<unsigned int> frame_priorities{6, 5, 4, 3, 2, 1, 0};
		map<double, double> rates;
		auto res = calc_need_fr_proc_statuses(0.0, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		for (auto el: res) {
			EXPECT_EQ(el, true);
		}
	}
	{
		//there is rates - all process enables - simple priority order
		vector<unsigned int> frame_priorities{0, 1, 2, 3, 4, 5, 6};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(0.5, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		for (auto el: res) {
			EXPECT_EQ(el, true);
		}
	}
	{
		//there is rates - all process enables - simple priority order
		vector<unsigned int> frame_priorities{0, 1, 2, 3, 4, 5, 6};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(1.0, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		for (auto el: res) {
			EXPECT_EQ(el, true);
		}
	}
	{
		//there is rates - some process enables, some don't - simple priority order
		vector<unsigned int> frame_priorities{0, 1, 2, 3, 4, 5, 6};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(1.101, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		vector<bool> expected{true, true, true, true, true, true, false};
		EXPECT_EQ(res, expected);
	}
	{
		//there is rates - some process enables, some don't - inverse priority order
		vector<unsigned int> frame_priorities{6, 5, 4, 3, 2, 1, 0};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(1.101, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		vector<bool> expected{false, true, true, true, true, true, true};
		EXPECT_EQ(res, expected);
	}
	{
		//there is rates - some process enables, some don't - complex order
		vector<unsigned int> frame_priorities{2, 5, 1, 6, 0, 4, 3};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(1.101, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		vector<bool> expected{true, true, true, false, true, true, true};
		EXPECT_EQ(res, expected);
	}
	{
		//there is rates - some process enables, some don't - complex order - big time
		vector<unsigned int> frame_priorities{2, 5, 1, 6, 0, 4, 3};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(2.101, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		vector<bool> expected{false, true, true, false, false, true, true};
		EXPECT_EQ(res, expected);
	}
	{
		//there is rates - some process enables, some don't - complex order - small time
		vector<unsigned int> frame_priorities{2, 5, 1, 6, 0, 4, 3};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(1.0501, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		for (auto el: res) {
			EXPECT_EQ(el, true);
		}
	}
	{
		//there is rates - some process enables, some don't - complex order - very big time
		vector<unsigned int> frame_priorities{2, 5, 1, 6, 0, 4, 3};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6}};
		auto res = calc_need_fr_proc_statuses(110.0501, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		vector<bool> expected{false, true, true, false, false, true, true};
		EXPECT_EQ(res, expected);
	}
	{
		//there is rates - some process enables, some don't - complex order - very big time2
		vector<unsigned int> frame_priorities{2, 5, 1, 6, 0, 4, 3};
		map<double, double> rates{{1.1, 0.9},
								  {1.2, 0.8},
								  {1.5, 0.6},
								  {3.5, 0.1}};
		auto res = calc_need_fr_proc_statuses(110.0501, rates, frame_priorities);
		EXPECT_EQ(res.size(), 7);
		vector<bool> expected{false, false, true, false, false, false, false};
		EXPECT_EQ(res, expected);
	}
}

TEST(test_proc_tile_limit, test_limits) {
	{    //turn off - all calc must be
		calc_tile_lim_cfg_t cfg{false, 200.0, {{1.1, 0.9}, {1.2, 0.8}, {1.5, 0.6}, {3.5, 0.1}}};
		auto limit_manager = get_tile_calc_limiter(cfg, 3, 3);
		for (unsigned int i = 0; i < 9; ++i) {
			EXPECT_TRUE(limit_manager->need_proc_tile(i));
		}
		limit_manager->set_last_frame_proc_time_mc(20000);
		for (unsigned int i = 0; i < 9; ++i) {
			EXPECT_TRUE(limit_manager->need_proc_tile(i));
		}
		limit_manager->set_last_frame_proc_time_mc(25000);
		for (unsigned int i = 0; i < 9; ++i) {
			EXPECT_TRUE(limit_manager->need_proc_tile(i));
		}
		limit_manager->set_last_frame_proc_time_mc(1);
		for (unsigned int i = 0; i < 9; ++i) {
			EXPECT_TRUE(limit_manager->need_proc_tile(i));
		}
	}

	{    //turn on - some calc may be skipped
		calc_tile_lim_cfg_t cfg{true, 200.0, {{1.1, 0.9}, {1.2, 0.8}, {1.5, 0.6}, {3.5, 0.1}}};
		auto limit_manager = get_tile_calc_limiter(cfg, 3, 3);
		for (unsigned int i = 0; i < 9; ++i) {
			EXPECT_TRUE(limit_manager->need_proc_tile(i));
		}
		limit_manager->set_last_frame_proc_time_mc(20000);
		EXPECT_FALSE(limit_manager->need_proc_tile(0));
		EXPECT_FALSE(limit_manager->need_proc_tile(1));
		EXPECT_FALSE(limit_manager->need_proc_tile(2));
		EXPECT_FALSE(limit_manager->need_proc_tile(3));
		EXPECT_TRUE(limit_manager->need_proc_tile(4));
		EXPECT_FALSE(limit_manager->need_proc_tile(5));
		EXPECT_FALSE(limit_manager->need_proc_tile(6));
		EXPECT_FALSE(limit_manager->need_proc_tile(7));
		EXPECT_FALSE(limit_manager->need_proc_tile(8));

		limit_manager->set_last_frame_proc_time_mc(10);
		EXPECT_FALSE(limit_manager->need_proc_tile(0));
		EXPECT_FALSE(limit_manager->need_proc_tile(1));
		EXPECT_FALSE(limit_manager->need_proc_tile(2));
		EXPECT_FALSE(limit_manager->need_proc_tile(3));
		EXPECT_TRUE(limit_manager->need_proc_tile(4));
		EXPECT_FALSE(limit_manager->need_proc_tile(5));
		EXPECT_FALSE(limit_manager->need_proc_tile(6));
		EXPECT_FALSE(limit_manager->need_proc_tile(7));
		EXPECT_FALSE(limit_manager->need_proc_tile(8));

		limit_manager->set_last_frame_proc_time_mc(10);
		limit_manager->set_last_frame_proc_time_mc(10);
		limit_manager->set_last_frame_proc_time_mc(10);

		limit_manager->set_last_frame_proc_time_mc(10);
		EXPECT_TRUE(limit_manager->need_proc_tile(0));
		EXPECT_TRUE(limit_manager->need_proc_tile(1));
		EXPECT_TRUE(limit_manager->need_proc_tile(2));
		EXPECT_FALSE(limit_manager->need_proc_tile(3));
		EXPECT_TRUE(limit_manager->need_proc_tile(4));
		EXPECT_TRUE(limit_manager->need_proc_tile(5));
		EXPECT_FALSE(limit_manager->need_proc_tile(6));
		EXPECT_FALSE(limit_manager->need_proc_tile(7));
		EXPECT_FALSE(limit_manager->need_proc_tile(8));

		limit_manager->set_last_frame_proc_time_mc(10);
		EXPECT_TRUE(limit_manager->need_proc_tile(0));
		EXPECT_TRUE(limit_manager->need_proc_tile(1));
		EXPECT_TRUE(limit_manager->need_proc_tile(2));
		EXPECT_FALSE(limit_manager->need_proc_tile(3));
		EXPECT_TRUE(limit_manager->need_proc_tile(4));
		EXPECT_TRUE(limit_manager->need_proc_tile(5));
		EXPECT_FALSE(limit_manager->need_proc_tile(6));
		EXPECT_FALSE(limit_manager->need_proc_tile(7));
		EXPECT_FALSE(limit_manager->need_proc_tile(8));

		limit_manager->set_last_frame_proc_time_mc(10);
		EXPECT_TRUE(limit_manager->need_proc_tile(0));
		EXPECT_TRUE(limit_manager->need_proc_tile(1));
		EXPECT_TRUE(limit_manager->need_proc_tile(2));
		EXPECT_TRUE(limit_manager->need_proc_tile(3));
		EXPECT_TRUE(limit_manager->need_proc_tile(4));
		EXPECT_TRUE(limit_manager->need_proc_tile(5));
		EXPECT_TRUE(limit_manager->need_proc_tile(6));
		EXPECT_TRUE(limit_manager->need_proc_tile(7));
		EXPECT_FALSE(limit_manager->need_proc_tile(8));

		limit_manager->set_last_frame_proc_time_mc(10);
		EXPECT_TRUE(limit_manager->need_proc_tile(0));
		EXPECT_TRUE(limit_manager->need_proc_tile(1));
		EXPECT_TRUE(limit_manager->need_proc_tile(2));
		EXPECT_TRUE(limit_manager->need_proc_tile(3));
		EXPECT_TRUE(limit_manager->need_proc_tile(4));
		EXPECT_TRUE(limit_manager->need_proc_tile(5));
		EXPECT_TRUE(limit_manager->need_proc_tile(6));
		EXPECT_TRUE(limit_manager->need_proc_tile(7));
		EXPECT_TRUE(limit_manager->need_proc_tile(8));
	}
}
