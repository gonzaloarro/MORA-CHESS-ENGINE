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

#include "bitboards.h"

namespace Bitboards {

	void init_bit_masks();
	void init_files_bb();
	void init_ranks_bb();
	void init_passed_pawn_masks();

	/*
	 * Initialization.
	 */
	void init() {
		init_bit_masks();
		init_files_bb();
		init_ranks_bb();
		init_passed_pawn_masks();
	}

	/*
	 * Masks to set and clear bits.
	 */
	Bitboard set_bit_masks[SQUARES];
	Bitboard clear_bit_masks[SQUARES];

	void init_bit_masks() {
		Bitboard one = 1;
		for (int i = 0; i < 64; i++) {
			set_bit_masks[i] = one << i;
			clear_bit_masks[i] = ~set_bit_masks[i];
		}
	}

	/*
	 * Files and ranks bitboards.
	 */
	Bitboard files_bb[FILES];
	Bitboard ranks_bb[RANKS];

	void init_files_bb() {
		Bitboard file_bitboard = 0x0101010101010101;
		for (int file = FILE_A; file <= FILE_H; file++) {
			files_bb[file] = file_bitboard;
			file_bitboard <<= 1;
		}
	}

	void init_ranks_bb() {
		Bitboard rank_bitboard = 0x00000000000000FF;
		for (int rank = RANK_1; rank <= RANK_8; rank++) {
			ranks_bb[rank] = rank_bitboard;
			rank_bitboard <<= 8;
		}
	}

	/*
	 * Masks to evaluate passed pawns.
	 */
	Bitboard passed_pawn_mask[PLAYERS][SQUARES];

	// Bitboard spans
	Bitboard white_front_span(Bitboard white_pawns);
	Bitboard black_front_span(Bitboard black_pawns);
	// Bitboard shifting
	Bitboard north_one (Bitboard b);
	Bitboard south_one (Bitboard b);
	Bitboard east_one (Bitboard b);
	Bitboard west_one (Bitboard b);

	void init_passed_pawn_masks() {
		for (int square = A1; square <= H8; square++) {
			Bitboard square_bitboard = 0;
			set_bit(square_bitboard, square);
			passed_pawn_mask[WHITE][square] = 	east_one(white_front_span(square_bitboard)) |
												west_one(white_front_span(square_bitboard)) |
												white_front_span(square_bitboard);
			passed_pawn_mask[BLACK][square] = 	east_one(black_front_span(square_bitboard)) |
												west_one(black_front_span(square_bitboard)) |
												black_front_span(square_bitboard);
		}
	}

	/*
	 * Bitboard shifting.
	 */
	Bitboard north_one (Bitboard b) {
		return  b << 8;
	}
	Bitboard south_one (Bitboard b) {
		return  b >> 8;
	}
	Bitboard east_one (Bitboard b) {
		return (b << 1) & not_A_file;
	}
	Bitboard west_one (Bitboard b) {
		return (b >> 1) & not_H_file;
	}

	// Fills
	Bitboard north_fill(Bitboard b);
	Bitboard south_fill(Bitboard b);

	/*
	 * Bitboard spans.
	 */
	Bitboard white_front_span(Bitboard white_pawns) {
		return north_one(north_fill(white_pawns));
	}
	Bitboard black_front_span(Bitboard black_pawns) {
		return south_one(south_fill(black_pawns));
	}

	/*
	 * Bitboard fills.
	 */
	Bitboard north_fill(Bitboard b) {
	   b |= (b <<  8);
	   b |= (b << 16);
	   b |= (b << 32);
	   return b;
	}

	Bitboard south_fill(Bitboard b) {
	   b |= (b >>  8);
	   b |= (b >> 16);
	   b |= (b >> 32);
	   return b;
	}

	/*
	 * Bitscan.
	 */
	const int index64_forward[64] = {
		0,  1, 48,  2, 57, 49, 28,  3,
	   61, 58, 50, 42, 38, 29, 17,  4,
	   62, 55, 59, 36, 53, 51, 43, 22,
	   45, 39, 33, 30, 24, 18, 12,  5,
	   63, 47, 56, 27, 60, 41, 37, 16,
	   54, 35, 52, 21, 44, 32, 23, 11,
	   46, 26, 40, 15, 34, 20, 31, 10,
	   25, 14, 19,  9, 13,  8,  7,  6
	};

	int bit_scan_forward(Bitboard bitboard) {
		const Bitboard debruijn64 = 0x03f79d71b4cb0a89;
		return index64_forward[((bitboard & -bitboard) * debruijn64) >> 58];
	}

	/*
	 * Population count.
	 */
	int population_count(Bitboard bitboard) {
		int count = 0;
		while(bitboard) {
			count++;
			bitboard &= bitboard - 1;
		}
		return count;
	}

	/*
	 * Set bits.
	 */
	void set_bit(Bitboard &bitboard, int i) {
		bitboard = bitboard | set_bit_masks[i];
	}

	/*
	 * Clear bits.
	 */
	void clear_bit(Bitboard &bitboard, int i) {
		bitboard = bitboard & clear_bit_masks[i];
	}
}
