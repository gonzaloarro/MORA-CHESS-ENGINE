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

#ifndef SRC_BITBOARDS_H_
#define SRC_BITBOARDS_H_

#include "types.h"

// Bitboard type
typedef unsigned long long Bitboard;

namespace Bitboards {

	// Initialization
	void init();

	// Bitboard manipulation
	void set_bit(Bitboard &bitboard, int i);
	void clear_bit(Bitboard &bitboard, int i);

	// Bitboard info
	int bit_scan_forward(Bitboard bitboard);
	int population_count(Bitboard bitboard);

	// Data
	extern Bitboard files_bb[FILES];
	extern Bitboard ranks_bb[RANKS];
	const Bitboard not_A_file = 0xfefefefefefefefe;
	const Bitboard not_H_file = 0x7f7f7f7f7f7f7f7f;
	const Bitboard not_8_rank = 0x00FFFFFFFFFFFFFF;
	const Bitboard not_1_rank = 0xFFFFFFFFFFFFFF00;
	/*
	 * Masks to evaluate passed pawns.
	 */
	extern Bitboard passed_pawn_mask[PLAYERS][SQUARES];
}

#endif /* SRC_BITBOARDS_H_ */
