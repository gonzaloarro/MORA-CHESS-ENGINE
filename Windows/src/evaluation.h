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

#ifndef SRC_EVALUATION_H_
#define SRC_EVALUATION_H_

#include "types.h"
#include "position.h"

namespace Evaluation {

	// Constants
	constexpr int contempt_factor = -50; // a positive value to prefer draws, a negative value to avoid them. todo: not being used by the moment
	constexpr int draw_score = 0;

	/*
	 * Receives a material score for the position and returns
	 * a full evaluation adding positional factors.
	 * A positive score represents the side to move has advantage
	 * and a negative score represents the same for the other player.
	 */
	int evaluate_positional_factors(Position &pos);

	/*
	 * Returns the material and piece location score.
	 * A positive score represents the side to move has advantage
	 * and a negative score represents the same for the other player.
	 */
	int evaluate_material(Position &pos);

	/*
	 * Return the material value of a certain piece.
	 */
	int get_piece_value(int piece);

	/*
	 * Return the material value of a piece depending on the location
	 * of the piece on the board.
	 */
	int get_piece_value(int piece, int square, Color side);

	/*
	 * Returns true if there's insufficient material on the board.
	 */
	bool insufficient_material(Position &pos);
}

#endif /* SRC_EVALUATION_H_ */
