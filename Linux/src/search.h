/*
 * MORA CHESS ENGINE (MCE).
 * Copyright (C) 2019 Gonzalo Arró.
 *
 * This file is part of MORA CHESS ENGINE.
 *
 * MORA CHESS ENGINE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MORA CHESS ENGINE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MORA CHESS ENGINE. If not, see <https://www.gnu.org/licenses/>
 *
 * Author: gonzalo.arro@gmail.com
 */

#ifndef SRC_SEARCH_H_
#define SRC_SEARCH_H_

#include "position.h"

namespace Search {

	// Constants
	constexpr int MATE_SCORE = 99000;
	constexpr int MAX_DEPTH = 32;

	/*
	 * Search info struct.
	 */
	struct Search_info {
		int depth;
		int time_to_search;
		long long start_time;
		long long nodes;
		bool stop;
	};

	/*
	 * Seach the position to a certain depth
	 * depending on the options specified in the
	 * search info.
	 * Prints the best move found.
	 */
	void search(Position &pos, Search_info &search_info);
}

#endif /* SRC_SEARCH_H_ */
