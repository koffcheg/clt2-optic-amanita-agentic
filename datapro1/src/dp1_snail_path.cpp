//
// Created by merich on 09.10.24.
//

#include <cassert>
#include "dp1_snail_path.h"
#include <algorithm>

using std::vector;
using std::pair;

namespace ns_datapro1 {
	snail_path::snail_path(std::size_t num_rows, std::size_t num_cols) :
			num_rows_{num_rows},
			num_cols_{num_cols},
			marked_rows_(num_rows_, false),
			marked_cols_(num_cols_, false) {
		assert(num_rows_ && num_cols_);
	}

	vector <pair<std::size_t, std::size_t>> snail_path::get_path() {
		std::fill(marked_rows_.begin(), marked_rows_.end(), false);
		std::fill(marked_cols_.begin(), marked_cols_.end(), false);
		vector<pair<std::size_t, std::size_t>> res;
		std::size_t curr_row = num_rows_ - 1;
		std::size_t curr_col = num_cols_ - 1;
		res.emplace_back(curr_row, curr_col);
		en_mov_dir move_dir = en_mov_dir::move_dir_left;
		bool move_on = true;
		while (move_on) {
			switch (move_dir) {
				case en_mov_dir::move_dir_left:
					if (can_move_left(curr_col)) {
						--curr_col;
						res.emplace_back(curr_row, curr_col);
					} else {
						marked_rows_[curr_row] = true;
						move_dir = en_mov_dir::move_dir_up;
						move_on = can_move_up(curr_row);
					}
					break;
				case en_mov_dir::move_dir_up:
					if (can_move_up(curr_row)) {
						--curr_row;
						res.emplace_back(curr_row, curr_col);
					} else {
						marked_cols_[curr_col] = true;
						move_dir = en_mov_dir::move_dir_right;
						move_on = can_move_right(curr_col);
					}
					break;
				case en_mov_dir::move_dir_right:
					if (can_move_right(curr_col)) {
						++curr_col;
						res.emplace_back(curr_row, curr_col);
					} else {
						marked_rows_[curr_row] = true;
						move_dir = en_mov_dir::move_dir_down;
						move_on = can_move_down(curr_row);
					}
					break;
				case en_mov_dir::move_dir_down:
					if (can_move_down(curr_row)) {
						++curr_row;
						res.emplace_back(curr_row, curr_col);
					} else {
						marked_cols_[curr_col] = true;
						move_dir = en_mov_dir::move_dir_left;
						move_on = can_move_left(curr_col);
					}
					break;
			}
		}
		std::reverse(res.begin(), res.end());
		return res;
	}
}