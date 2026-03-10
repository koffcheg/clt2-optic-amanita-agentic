//
// Created by merich on 09.10.24.
//

#ifndef CLT_OPTIC_M_NAIL_PATH_H
#define CLT_OPTIC_M_NAIL_PATH_H

#include <vector>

namespace ns_datapro1 {

	class snail_path {
		std::size_t num_rows_;
		std::size_t num_cols_;
		std::vector<bool> marked_rows_, marked_cols_;

		bool can_move_left(std::size_t curr_col) {
			if (!curr_col)
				return false;
			--curr_col;
			return !marked_cols_[curr_col];
		}

		bool can_move_up(std::size_t curr_row) {
			if (!curr_row)
				return false;
			--curr_row;
			return !marked_rows_[curr_row];
		}

		bool can_move_right(std::size_t curr_col) {
			++curr_col;
			if (curr_col >= num_cols_)
				return false;
			return !marked_cols_[curr_col];
		}

		bool can_move_down(std::size_t curr_row) {
			++curr_row;
			if (curr_row >= num_rows_)
				return false;
			return !marked_rows_[curr_row];
		}

		enum class en_mov_dir {
			move_dir_left,
			move_dir_up,
			move_dir_right,
			move_dir_down
		};
	public:
		snail_path(std::size_t num_rows, std::size_t num_cols);

		std::vector<std::pair<std::size_t, std::size_t>> get_path();
	};
}
#endif //CLT_OPTIC_M_NAIL_PATH_H
