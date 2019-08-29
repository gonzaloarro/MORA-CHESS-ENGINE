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

#include "types.h"
#include "bitboards.h"


#ifndef SRC_ATTACKS_H_
#define SRC_ATTACKS_H_

namespace Attacks {

	/*
	 * Attack tables for non sliding pieces.
	 */
	extern Bitboard knight_attacks[SQUARES];
	extern Bitboard pawn_attacks[PLAYERS][SQUARES];
	extern Bitboard king_attacks[SQUARES];
	extern Bitboard king_castling[PLAYERS][SQUARES];

	/*
	 * Struct for magic bitboards move generation.
	 */
	struct Magic {
		Bitboard mask;
		Bitboard magic_number;
	};

	/*
	 * Magic multipliers for bishops and rooks.
	 */
	extern Magic bishop_magic_table[SQUARES];
	extern Magic rook_magic_table[SQUARES];

	/*
	 * Attack tables for sliding pieces.
	 */
	extern Bitboard bishop_attacks[512][SQUARES]; // 256 KB
	extern Bitboard rook_attacks[4096][SQUARES]; // 2MB

	/*
	 * Precomputed mobility for bishops and rooks (also used for queens)
	 */
	extern int bishop_mobility[512][SQUARES];
	extern int rook_mobility[4096][SQUARES];

	// Initialization
	void init();

	/*
	 * Return queen attacks.
	 */
	Bitboard get_queen_attacks(Bitboard occupancy, int square);

	/*
	 * Return bishop attacks.
	 */
	Bitboard get_bishop_attacks(Bitboard occupancy, int square);

	/*
	 * Return rook attacks.
	 */
	Bitboard get_rook_attacks(Bitboard occupancy, int square);
}

#endif /* SRC_ATTACKS_H_ */
