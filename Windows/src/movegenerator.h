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

#ifndef SRC_MOVEGENERATOR_H_
#define SRC_MOVEGENERATOR_H_

#include "position.h"

namespace MoveGen {

	/*
	 * There are legal positions with more moves than 80
	 * but those are unlikely to be reached, and is more efficient
	 * to have a smaller array in the Move_list structure.
	 */
	constexpr int MAX_POSSIBLE_MOVES = 80;

	/*
	 * Move list struct to save generated moves.
	 */
	struct Move_list {
		Move moves[MAX_POSSIBLE_MOVES];
		int size;
		Move_list() : size(0) {};
	};

	/*
	 * Generate pseudo-legal moves in the position.
	 */
	void generate_moves(Position &pos, Move_list &move_list);

	/*
	 * Generate pseudo-legal captures in the position.
	 */
	void generate_captures(Position &pos, Move_list &move_list);

	/*
	 * Generate pseudo-legal promotions in the position.
	 */
	void generate_promotions(Position &pos, Move_list &move_list);
}

#endif /* SRC_MOVEGENERATOR_H_ */
