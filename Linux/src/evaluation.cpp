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

#include "evaluation.h"
#include "bitboards.h"
#include "movegenerator.h"
#include "attacks.h"
#include "pawnhashtable.h"

using namespace Attacks;

namespace Evaluation {

	// Bitboards used to detect isolated pawns
	const Bitboard neighbor_files[FILES] = {
		0x0202020202020202,
		0x0505050505050505,
		0x0A0A0A0A0A0A0A0A,
		0x1414141414141414,
		0x2828282828282828,
		0x5050505050505050,
		0xA0A0A0A0A0A0A0A0,
		0x4040404040404040
	};

	// Bonuses and penalties
	// Mobility
	constexpr int piece_mobility_factor = 8;
	constexpr int pawn_mobility_factor = 4;
	// Pawns
	constexpr int isolated_pawn = -16;
	constexpr int doubled_pawn = -16;
	constexpr int backward_pawn = -8;
	constexpr int passed_pawn = 32;
	// Bonuses
	constexpr int bishop_pair = 32;
	constexpr int move_bonus = 25;
	constexpr int rook_in_semiopen_file = 20;
	constexpr int rook_in_open_file = 40;
	// King safety
	constexpr int king_in_the_center = 40;
	constexpr int king_zone_attacked = 30;
	constexpr int pawn_shelter_bonus = 20;
	constexpr int pawn_shelter_small_bonus = 10;
	constexpr int pawn_storm_close_range = 25;
	constexpr int pawn_storm_medium_range = 15;
	constexpr int pawn_storm_long_range = 5;
	constexpr int enemy_semiopen_king_file = 50;
	constexpr int own_semiopen_king_file = 60;
	constexpr int enemy_semiopen_file_next_to_king = 30;
	constexpr int own_semiopen_file_next_to_king = 20;

	// Piece values in centipawns
	constexpr int pawn_value = 100;
	constexpr int knight_value = 310;
	constexpr int bishop_value = 320;
	constexpr int rook_value = 500;
	constexpr int queen_value = 900;
	constexpr int king_value = 20000;

	// To compute game phase
	constexpr int initial_material = 16 * pawn_value + 4 * (knight_value + bishop_value + rook_value) + 2 * queen_value;

	/*
	 * Piece tables format:
	 *
	 * A1 B1 C1 D1 E1 F1 G1 H1
	 * A2 B2 C2 D2 E2 F2 G2 H2
	 * A3 B3 C3 D3 E3 F3 G3 H3
	 * A4 B4 C4 D4 E4 F4 G4 H4
	 * A5 B5 C5 D5 E5 F5 G5 H5
	 * A6 B6 C6 D6 E6 F6 G6 H6
	 * A7 B7 C7 D7 E7 F7 G7 H7
	 * A8 B8 C8 D8 E8 F8 G8 H8
	 */

	/*
	 * Pawns value depending on the square.
	 */
	const int pawn_table[SQUARES] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			 5, 10, 10,-20,-20, 10, 10,  5,
			 5, -5,-10,  0,  0,-15, -5,  5,
			 0,  0,  0, 25, 25,  0,  0,  0,
			 5,  5, 10, 25, 25, 10,  5,  5,
			10, 10, 20, 30, 30, 20, 10, 10,
			20, 30, 30, 35, 35, 30, 30, 20,
			 0,  0,  0,  0,  0,  0,  0,  0
	};

	/*
	 * Knights value depending on the square.
	 */
	const int knight_table[SQUARES] = {
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-50,-40,-30,-30,-30,-30,-40,-50
	};

	/*
	 * Bishops value depending on the square.
	 */
	const int bishop_table[SQUARES] = {
			-20,-10,-10,-10,-10,-10,-10,-20,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-20,-10,-10,-10,-10,-10,-10,-20
	};

	/*
	 * Rooks value depending on the square.
	 */
	const int rook_table[SQUARES] = {
			  0,  0,  5,  10, 10, 5,  0,  0,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			 -5,  0,  0,  0,  0,  0,  0, -5,
			  5, 15, 15, 15, 15, 15, 15,  5,
			  0,  0,  0,  0,  0,  0,  0,  0
	};

	/*
	 * Queens value depending on the square.
	 */
	const int queen_table[SQUARES] = {
			-20,-10,-10, -5, -5,-10,-10,-20,
			-10,  0,  5,  0,  0,  0,  0,-10,
			-10,  5,  5,  5,  5,  5,  0,-10,
			  0,  0,  5,  5,  5,  5,  0, -5,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			-10,  0,  5,  5,  5,  5,  0,-10,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-20,-10,-10, -5, -5,-10,-10,-20
	};

	/*
	 * King value in the middlegame depending on the square.
	 */
	const int king_table_middlegame[SQUARES] = {
			 15, 30, 10,  -5,  -5, 10, 40, 15,
			 20, 20,  0, -10, -10,  0, 20, 20,
			-10,-20,-20, -20, -20,-20,-20,-10,
			-20,-30,-30, -40, -40,-30,-30,-20,
			-30,-40,-40, -50, -50,-40,-40,-30,
			-30,-40,-40, -50, -50,-40,-40,-30,
			-30,-40,-40, -50, -50,-40,-40,-30,
			-30,-40,-40, -50, -50,-40,-40,-30
	};

	/*
	 * King value in the endgame depending on the square.
	 */
	const int king_table_endgame[SQUARES] = {
			-50,-30,-30,-30,-30,-30,-30,-50,
			-30,-20,  0,  0,  0,  0,-20,-30,
			-30,-10, 20, 30, 30, 20,-10,-30,
			-30,-10, 30, 40, 40, 30,-10,-30,
			-30,-10, 30, 40, 40, 30,-10,-30,
			-30,-10, 20, 30, 30, 20,-10,-30,
			-30,-20,-10,  0,  0,-10,-20,-30,
			-50,-40,-30,-20,-20,-30,-40,-50
	};

	/*
	 * Mirror square table to apply the same
	 * piece tables for black.
	 */
	const int mirror_square[SQUARES] = {
			56, 57, 58, 59, 60, 61, 62, 63,
			48, 49, 50, 51, 52, 53, 54, 55,
			40, 41, 42, 43, 44, 45, 46, 47,
			32, 33, 34, 35, 36, 37, 38, 39,
			24, 25, 26, 27, 28, 29, 30, 31,
			16, 17, 18, 19, 20, 21, 22, 23,
			 8,  9, 10, 11, 12, 13, 14, 15,
			 0,  1,  2,  3,  4,  5,  6,  7
	};

	/*
	 * Returns the pawn structure score.
	 */
	void compute_pawns_info(Position &pos, Pawns_info &pawns_info);

	/*
	 * Returns true if the material left on the board is
	 * not enough for either side to give mate.
	 */
	bool insufficient_material(Position &pos);

	/*
	 * Returns true if side has at least two bishops or
	 * a bishop and a knight (not having pawns, rooks or queens).
	 */
	bool mating_material(Position &pos, Color side);

	/*
	 * Receives a score from white point of view
	 * and returns a score from side_to_move point
	 * of view.
	 */
	int adjust_score_sign(Position &pos, int score);

	/*
	 * Returns a piece value.
	 */
	int get_piece_value(int piece) {
		switch(piece) {
		case(KNIGHT) : return knight_value; break;
		case(BISHOP) : return bishop_value; break;
		case(ROOK) : return rook_value; break;
		case(QUEEN) : return queen_value; break;
		case(PAWN) : return pawn_value; break;
		case(KING) : return king_value; break;
		default: return 0;
		}
	}

	/*
	 * Returns a piece value depending of the location
	 * on the board.
	 */
	int get_piece_value(int piece, int square, Color side) {
		square = side == WHITE ? square : mirror_square[square];
		switch(piece) {
		case(KNIGHT) : return knight_table[square] + knight_value; break;
		case(BISHOP) : return bishop_table[square] + bishop_value; break;
		case(ROOK) : return rook_table[square] + rook_value; break;
		case(QUEEN) : return queen_table[square] + queen_value; break;
		case(PAWN) : return pawn_table[square] + pawn_value; break;
		case(KING) : return king_value; break;
		default: return 0;
		}
	}

	/*
	 * Receives a score based on white point of view and
	 * returns a score based on side_to_move point of view.
	 */
	inline int adjust_score_sign(Position &pos, int score) {
		return score * (~pos.get_side_to_move() - pos.get_side_to_move());
	}

	/*
	 * Returns the material and piece location score.
	 */
	int evaluate_material(Position &pos) {
		// Set the material score for a player to 0 if it has a piece
		// combination that's not enough for giving mate.
		int score = pos.get_material(WHITE) * mating_material(pos, WHITE) -
					pos.get_material(BLACK) * mating_material(pos, BLACK);
		// Side to move point of view.
		return adjust_score_sign(pos, score);
	}

	/*
	 * Returns true if the material left on the board is
	 * not enough for either side to give mate.
	 */
	bool insufficient_material(Position &pos) {
		Bitboard pawns_rooks_queens = 	pos.get_piece_bitboard(WHITE, PAWN) |
										pos.get_piece_bitboard(WHITE, ROOK) |
										pos.get_piece_bitboard(WHITE, QUEEN) |
										pos.get_piece_bitboard(BLACK, PAWN) |
										pos.get_piece_bitboard(BLACK, ROOK) |
										pos.get_piece_bitboard(BLACK, QUEEN);
		if (pawns_rooks_queens == 0) { // only minor pieces
			Color stronger_side = pos.get_material(WHITE) - pos.get_material(BLACK) >= 0 ? WHITE : BLACK;
			Color weaker_side = ~stronger_side;
			int stronger_side_bishops = Bitboards::population_count(pos.get_piece_bitboard(stronger_side, BISHOP));
			int stronger_side_knights = Bitboards::population_count(pos.get_piece_bitboard(stronger_side, KNIGHT));
			int weaker_side_bishops = Bitboards::population_count(pos.get_piece_bitboard(weaker_side, BISHOP));
			int weaker_side_knights = Bitboards::population_count(pos.get_piece_bitboard(weaker_side, KNIGHT));
			// Basic scenarios
			if (stronger_side_bishops == 2)
				return (weaker_side_bishops > 0 || weaker_side_knights == 2);

			if (stronger_side_bishops == 1 && stronger_side_knights == 1)
				return (weaker_side_bishops == 1 || weaker_side_knights > 0);

			return ((stronger_side_bishops + stronger_side_knights == 1) || stronger_side_knights == 2);
		}
		return false;
	}

	/*
	 * Returns true if side has at least two bishops or
	 * a bishop and a knight (not having pawns, rooks or queens).
	 */
	bool mating_material(Position &pos, Color side) {
		Bitboard pawns_rooks_queens = 	pos.get_piece_bitboard(WHITE, PAWN) |
										pos.get_piece_bitboard(WHITE, ROOK) |
										pos.get_piece_bitboard(WHITE, QUEEN) |
										pos.get_piece_bitboard(BLACK, PAWN) |
										pos.get_piece_bitboard(BLACK, ROOK) |
										pos.get_piece_bitboard(BLACK, QUEEN);
		if (pawns_rooks_queens == 0) {
			// only minor pieces
			int bishops_count = Bitboards::population_count(pos.get_piece_bitboard(side, BISHOP));
			int knights_count = Bitboards::population_count(pos.get_piece_bitboard(side, KNIGHT));
			// Basic scenarios
			if (bishops_count == 1)
				return knights_count > 0;
			if (bishops_count == 0)
				return knights_count > 2;
		}
		return true;
	}


	/*
	 * Returns the positional evaluation score.
	 */
	int evaluate_positional_factors(Position &pos) {
		/*
		 * TODO: this is a really long function and there's
		 * duplicated code... I should add auxiliary functions
		 */
		Bitboard empty_squares = pos.get_empty_squares();

		// Computes the game phase based on the material left on the board.
		// ***********************************************************
		int position_material = pos.get_material(WHITE) + pos.get_material(BLACK) - 2 * king_value;
		int middlegame_percentage = (position_material * 100) / initial_material;
		int endgame_percentage = 100 - middlegame_percentage;
		// ***********************************************************

		// Pawns info
		// ***********************************************************
		Pawns_info pawns_info;
		if (!probe_hash_pawns(pos.get_pawns_key(), pawns_info)) {
			compute_pawns_info(pos, pawns_info);
		}
		// ***********************************************************

		// Score
		int score = pawns_info.score;

		// Evaluate pieces
		// ***********************************************************
		int piece_mobility = 0;
		Bitboard white_knights = pos.get_piece_bitboard(WHITE, KNIGHT);
		while(white_knights) {
			int knight_index = Bitboards::bit_scan_forward(white_knights);
			Bitboard targets = knight_attacks[knight_index] & (empty_squares | pos.get_occupied_squares(BLACK));
			piece_mobility += Bitboards::population_count(targets & ~pawns_info.pawn_targets[BLACK]);
			score += pawns_info.number_of_pawns[WHITE] + pawns_info.number_of_pawns[BLACK]; // bonus for having a knight and many pawns on the board
			white_knights &= white_knights - 1;
		}

		Bitboard black_knights = pos.get_piece_bitboard(BLACK, KNIGHT);
		while(black_knights) {
			int knight_index = Bitboards::bit_scan_forward(black_knights);
			Bitboard targets = knight_attacks[knight_index] & (empty_squares | pos.get_occupied_squares(WHITE));
			piece_mobility -= Bitboards::population_count(targets & ~pawns_info.pawn_targets[WHITE]);
			score -= pawns_info.number_of_pawns[WHITE] + pawns_info.number_of_pawns[BLACK]; // bonus for having a knight and many pawns on the board
			black_knights &= black_knights - 1;
		}

		Bitboard white_bishops = pos.get_piece_bitboard(WHITE, BISHOP);
		int bishops = 0;
		while(white_bishops) {
			bishops++;
			int bishop_index = Bitboards::bit_scan_forward(white_bishops);
			Bitboard occupancy = pos.get_occupancy();
			occupancy &= bishop_magic_table[bishop_index].mask;
			occupancy *= bishop_magic_table[bishop_index].magic_number;
			occupancy >>= 55;
			piece_mobility += bishop_mobility[occupancy][bishop_index];
			white_bishops &= white_bishops - 1;
		}
		if (bishops >= 2)
			score += bishop_pair;

		Bitboard black_bishops = pos.get_piece_bitboard(BLACK, BISHOP);
		bishops = 0;
		while(black_bishops) {
			bishops++;
			int bishop_index = Bitboards::bit_scan_forward(black_bishops);
			Bitboard occupancy = pos.get_occupancy();
			occupancy &= bishop_magic_table[bishop_index].mask;
			occupancy *= bishop_magic_table[bishop_index].magic_number;
			occupancy >>= 55;
			piece_mobility -= bishop_mobility[occupancy][bishop_index];
			black_bishops &= black_bishops - 1;
		}
		if (bishops >= 2)
			score -= bishop_pair;

		Bitboard white_rooks = pos.get_piece_bitboard(WHITE, ROOK);
		while (white_rooks) {
			int rook_index = Bitboards::bit_scan_forward(white_rooks);
			Bitboard occupancy = pos.get_occupancy();
			occupancy &= rook_magic_table[rook_index].mask;
			occupancy *= rook_magic_table[rook_index].magic_number;
			occupancy >>= 52;
			piece_mobility += rook_mobility[occupancy][rook_index];
			// Semi open file bonus
			if (!(Bitboards::files_bb[rook_index & 7] & pos.get_piece_bitboard(WHITE, PAWN))) {
				if (!(Bitboards::files_bb[rook_index & 7] & pos.get_piece_bitboard(BLACK, PAWN)))
					score += rook_in_open_file;
				else
					score += rook_in_semiopen_file;
			}
			white_rooks &= white_rooks - 1;
		}

		Bitboard black_rooks = pos.get_piece_bitboard(BLACK, ROOK);
		while (black_rooks) {
			int rook_index = Bitboards::bit_scan_forward(black_rooks);
			Bitboard occupancy = pos.get_occupancy();
			occupancy &= rook_magic_table[rook_index].mask;
			occupancy *= rook_magic_table[rook_index].magic_number;
			occupancy >>= 52;
			piece_mobility -= rook_mobility[occupancy][rook_index];
			// Semi open file bonus
			if (!(Bitboards::files_bb[rook_index & 7] & pos.get_piece_bitboard(BLACK, PAWN))) {
				if (!(Bitboards::files_bb[rook_index & 7] & pos.get_piece_bitboard(WHITE, PAWN)))
					score -= rook_in_open_file;
				else
					score -= rook_in_semiopen_file;
			}
			black_rooks &= black_rooks - 1;
		}

		Bitboard white_queens = pos.get_piece_bitboard(WHITE, QUEEN);
		Bitboard white_queens_attacks = 0;
		while (white_queens) {
			int queen_index = Bitboards::bit_scan_forward(white_queens);

			Bitboard occupancy = pos.get_occupancy();
			occupancy &= rook_magic_table[queen_index].mask;
			occupancy *= rook_magic_table[queen_index].magic_number;
			occupancy >>= 52;
			piece_mobility += rook_mobility[occupancy][queen_index];
			white_queens_attacks |= rook_attacks[occupancy][queen_index];

			occupancy = pos.get_occupancy();
			occupancy &= bishop_magic_table[queen_index].mask;
			occupancy *= bishop_magic_table[queen_index].magic_number;
			occupancy >>= 55;
			piece_mobility += bishop_mobility[occupancy][queen_index];
			white_queens_attacks |= bishop_attacks[occupancy][queen_index];

			white_queens &= white_queens - 1;
		}

		Bitboard black_queens = pos.get_piece_bitboard(BLACK, QUEEN);
		Bitboard black_queens_attacks = 0;
		while (black_queens) {
			int queen_index = Bitboards::bit_scan_forward(black_queens);

			Bitboard occupancy = pos.get_occupancy();
			occupancy &= rook_magic_table[queen_index].mask;
			occupancy *= rook_magic_table[queen_index].magic_number;
			occupancy >>= 52;
			piece_mobility -= rook_mobility[occupancy][queen_index];
			black_queens_attacks |= rook_attacks[occupancy][queen_index];

			occupancy = pos.get_occupancy();
			occupancy &= bishop_magic_table[queen_index].mask;
			occupancy *= bishop_magic_table[queen_index].magic_number;
			occupancy >>= 55;
			piece_mobility -= bishop_mobility[occupancy][queen_index];
			black_queens_attacks |= bishop_attacks[occupancy][queen_index];

			black_queens &= black_queens - 1;
		}

		score += piece_mobility * piece_mobility_factor;
		// ***********************************************************

		// Passed pawns
		// ***********************************************************
		Bitboard white_passed_pawns = pawns_info.passed_pawns[WHITE];
		while (white_passed_pawns) {
			int pawn_index = Bitboards::bit_scan_forward(white_passed_pawns);
			score += (passed_pawn * (pawn_index >> 3) * endgame_percentage) / 100;
			white_passed_pawns &= white_passed_pawns - 1;
		}

		Bitboard black_passed_pawns = pawns_info.passed_pawns[BLACK];
		while (black_passed_pawns) {
			int pawn_index = Bitboards::bit_scan_forward(black_passed_pawns);
			score -= (passed_pawn * (7 - (pawn_index >> 3)) * endgame_percentage) / 100;
			black_passed_pawns &= black_passed_pawns - 1;
		}
		// ***********************************************************

		// Pawns mobility
		// ***********************************************************
		Bitboard white_single_push_targets = (pos.get_piece_bitboard(WHITE, PAWN) << 8) & empty_squares;
		score += Bitboards::population_count(white_single_push_targets) * pawn_mobility_factor;
		Bitboard black_single_push_targets = (pos.get_piece_bitboard(BLACK, PAWN) >> 8) & empty_squares;
		score -= Bitboards::population_count(black_single_push_targets) * pawn_mobility_factor;
		// ***********************************************************

		// Bonus for having the move
		// ***********************************************************
		if (endgame_percentage < 80) {
			score += (~pos.get_side_to_move() - pos.get_side_to_move()) * move_bonus;
		}
		// ***********************************************************

		// King position
		// ***********************************************************
		int white_king_square = Bitboards::bit_scan_forward(pos.get_piece_bitboard(WHITE, KING));
		int black_king_square = Bitboards::bit_scan_forward(pos.get_piece_bitboard(BLACK, KING));

		int king_middlegame_score = king_table_middlegame[white_king_square] - king_table_middlegame[mirror_square[black_king_square]];
		int king_endgame_score = king_table_endgame[white_king_square] - king_table_endgame[mirror_square[black_king_square]];
		// ***********************************************************

		// King safety
		// ***********************************************************
		int king_safety = 0;
		if (FILE_E < (white_king_square & 7))
			king_safety += pawns_info.king_wing_safety[WHITE];
		else if ((white_king_square & 7) < FILE_D)
			king_safety += pawns_info.queen_wing_safety[WHITE];
		else
			king_safety -= king_in_the_center;

		if (FILE_E < (black_king_square & 7))
			king_safety -= pawns_info.king_wing_safety[BLACK];
		else if ((black_king_square & 7) < FILE_D)
			king_safety -= pawns_info.queen_wing_safety[BLACK];
		else
			king_safety += king_in_the_center;

		if (king_attacks[white_king_square] & black_queens_attacks)
			king_safety -= king_zone_attacked;

		if (king_attacks[black_king_square] & white_queens_attacks)
			king_safety += king_zone_attacked;

		score += ((king_middlegame_score + king_safety) * middlegame_percentage + king_endgame_score * endgame_percentage) / 100;
		// ***********************************************************

		// Side to move point of view.
		return adjust_score_sign(pos, score);
	}

	/*
	 * Returns the pawn structure score for a certain side.
	 */
	void compute_pawns_info(Position &pos, Pawns_info &pawns_info) {
		/*
		 * TODO: this is a really long function and there's
		 * duplicated code... I should add auxiliary functions
		 */
		Bitboard white_pawns = pos.get_piece_bitboard(WHITE, PAWN);
		Bitboard black_pawns = pos.get_piece_bitboard(BLACK, PAWN);

		// Pawn attacks
		// ***********************************************************
		Bitboard pawn_targets[2];
		pawn_targets[WHITE] = 	((white_pawns << 7) & Bitboards::not_H_file) |
								((white_pawns << 9) & Bitboards::not_A_file);
		pawn_targets[BLACK] = 	((black_pawns >> 7) & Bitboards::not_A_file) |
								((black_pawns >> 9) & Bitboards::not_H_file);
		// ***********************************************************

		// King safety
		// ***********************************************************

		int white_king_wing_safety = 0;
		int white_queen_wing_safety = 0;
		int black_king_wing_safety = 0;
		int black_queen_wing_safety = 0;

		// Pawn shelter
		// ***********************************************************
		Bitboard white_king_wing_shelter = 0x0000000000E0E000 & white_pawns;
		Bitboard white_queen_wing_shelter = 0x0000000000070700 & white_pawns;
		Bitboard black_king_wing_shelter = 0x00E0E00000000000 & black_pawns;
		Bitboard black_queen_wing_shelter = 0x0007070000000000 & black_pawns;

		while(white_king_wing_shelter) {
			int pawn_index = Bitboards::bit_scan_forward(white_king_wing_shelter);
			switch(pawn_index) {
			case(F2): white_king_wing_safety += pawn_shelter_bonus; break;
			case(G2): white_king_wing_safety += pawn_shelter_bonus; break;
			case(H2): white_king_wing_safety += pawn_shelter_bonus; break;
			case(F3): white_king_wing_safety += pawn_shelter_small_bonus; break;
			case(G3): white_king_wing_safety += pawn_shelter_small_bonus; break;
			case(H3): white_king_wing_safety += pawn_shelter_small_bonus; break;
			}
			white_king_wing_shelter &= white_king_wing_shelter - 1;
		}

		while(white_queen_wing_shelter) {
			int pawn_index = Bitboards::bit_scan_forward(white_queen_wing_shelter);
			switch(pawn_index) {
			case(A2): white_queen_wing_safety += pawn_shelter_bonus; break;
			case(B2): white_queen_wing_safety += pawn_shelter_bonus; break;
			case(C2): white_queen_wing_safety += pawn_shelter_bonus; break;
			case(A3): white_queen_wing_safety += pawn_shelter_small_bonus; break;
			case(B3): white_queen_wing_safety += pawn_shelter_small_bonus; break;
			case(C3): white_queen_wing_safety += pawn_shelter_small_bonus; break;
			}
			white_queen_wing_shelter &= white_queen_wing_shelter - 1;
		}

		while(black_king_wing_shelter) {
			int pawn_index = Bitboards::bit_scan_forward(black_king_wing_shelter);
			switch(pawn_index) {
			case(F7): black_king_wing_safety += pawn_shelter_bonus; break;
			case(G7): black_king_wing_safety += pawn_shelter_bonus; break;
			case(H7): black_king_wing_safety += pawn_shelter_bonus; break;
			case(F6): black_king_wing_safety += pawn_shelter_small_bonus; break;
			case(G6): black_king_wing_safety += pawn_shelter_small_bonus; break;
			case(H6): black_king_wing_safety += pawn_shelter_small_bonus; break;
			}
			black_king_wing_shelter &= black_king_wing_shelter - 1;
		}

		while(black_queen_wing_shelter) {
			int pawn_index = Bitboards::bit_scan_forward(black_queen_wing_shelter);
			switch(pawn_index) {
			case(A7): black_queen_wing_safety += pawn_shelter_bonus; break;
			case(B7): black_queen_wing_safety += pawn_shelter_bonus; break;
			case(C7): black_queen_wing_safety += pawn_shelter_bonus; break;
			case(A6): black_queen_wing_safety += pawn_shelter_small_bonus; break;
			case(B6): black_queen_wing_safety += pawn_shelter_small_bonus; break;
			case(C6): black_queen_wing_safety += pawn_shelter_small_bonus; break;
			}
			black_queen_wing_shelter &= black_queen_wing_shelter - 1;
		}
		// ***********************************************************

		// Pawn storms
		// ***********************************************************
		Bitboard white_king_wing_pawn_storm = 0x0000E0E0E0000000 & white_pawns;
		Bitboard white_queen_wing_pawn_storm = 0x0000070707000000 & white_pawns;
		Bitboard black_king_wing_pawn_storm = 0x000000E0E0E00000 & black_pawns;
		Bitboard black_queen_wing_pawn_storm = 0x0000000707070000 & black_pawns;

		while(white_king_wing_pawn_storm) {
			int pawn_index = Bitboards::bit_scan_forward(white_king_wing_pawn_storm);
			switch(pawn_index >> 3) {
			case(RANK_6): black_king_wing_safety -= pawn_storm_close_range; break;
			case(RANK_5): black_king_wing_safety -= pawn_storm_medium_range; break;
			case(RANK_4): black_king_wing_safety -= pawn_storm_long_range; break;
			}
			white_king_wing_pawn_storm &= white_king_wing_pawn_storm - 1;
		}

		while(white_queen_wing_pawn_storm) {
			int pawn_index = Bitboards::bit_scan_forward(white_queen_wing_pawn_storm);
			switch(pawn_index >> 3) {
			case(RANK_6): black_queen_wing_safety -= pawn_storm_close_range; break;
			case(RANK_5): black_queen_wing_safety -= pawn_storm_medium_range; break;
			case(RANK_4): black_queen_wing_safety -= pawn_storm_long_range; break;
			}
			white_queen_wing_pawn_storm &= white_queen_wing_pawn_storm - 1;
		}

		while(black_king_wing_pawn_storm) {
			int pawn_index = Bitboards::bit_scan_forward(black_king_wing_pawn_storm);
			switch(pawn_index >> 3) {
			case(RANK_3): white_king_wing_safety -= pawn_storm_close_range; break;
			case(RANK_4): white_king_wing_safety -= pawn_storm_medium_range; break;
			case(RANK_5): white_king_wing_safety -= pawn_storm_long_range; break;
			}
			black_king_wing_pawn_storm &= black_king_wing_pawn_storm - 1;
		}

		while(black_queen_wing_pawn_storm) {
			int pawn_index = Bitboards::bit_scan_forward(black_queen_wing_pawn_storm);
			switch(pawn_index >> 3) {
			case(RANK_3): white_queen_wing_safety -= pawn_storm_close_range; break;
			case(RANK_4): white_queen_wing_safety -= pawn_storm_medium_range; break;
			case(RANK_5): white_queen_wing_safety -= pawn_storm_long_range; break;
			}
			black_queen_wing_pawn_storm &= black_queen_wing_pawn_storm - 1;
		}
		// ***********************************************************

		// Open files next to the king
		// ***********************************************************
		if (!(Bitboards::files_bb[FILE_F] & black_pawns)) {
			white_king_wing_safety -= enemy_semiopen_file_next_to_king;
			black_king_wing_safety -= own_semiopen_file_next_to_king;
		}
		if (!(Bitboards::files_bb[FILE_G] & black_pawns)) {
			white_king_wing_safety -= enemy_semiopen_king_file;
			black_king_wing_safety -= own_semiopen_king_file;
		}
		if (!(Bitboards::files_bb[FILE_H] & black_pawns)) {
			white_king_wing_safety -= enemy_semiopen_file_next_to_king;
			black_king_wing_safety -= own_semiopen_file_next_to_king;
		}
		if (!(Bitboards::files_bb[FILE_A] & black_pawns)) {
			white_queen_wing_safety -= enemy_semiopen_file_next_to_king;
			black_queen_wing_safety -= own_semiopen_file_next_to_king;
		}
		if (!(Bitboards::files_bb[FILE_B] & black_pawns)) {
			white_queen_wing_safety -= enemy_semiopen_king_file;
			black_queen_wing_safety -= own_semiopen_king_file;
		}
		if (!(Bitboards::files_bb[FILE_C] & black_pawns)) {
			white_queen_wing_safety -= enemy_semiopen_file_next_to_king;
			black_queen_wing_safety -= own_semiopen_file_next_to_king;
		}
		if (!(Bitboards::files_bb[FILE_F] & white_pawns)) {
			black_king_wing_safety -= enemy_semiopen_file_next_to_king;
			white_king_wing_safety -= own_semiopen_file_next_to_king;
		}
		if (!(Bitboards::files_bb[FILE_G] & white_pawns)) {
			black_king_wing_safety -= enemy_semiopen_king_file;
			white_king_wing_safety -= own_semiopen_king_file;
		}
		if (!(Bitboards::files_bb[FILE_H] & white_pawns)) {
			black_king_wing_safety -= enemy_semiopen_file_next_to_king;
			white_king_wing_safety -= own_semiopen_file_next_to_king;
		}

		if (!(Bitboards::files_bb[FILE_A] & white_pawns)) {
			black_queen_wing_safety -= enemy_semiopen_file_next_to_king;
			white_queen_wing_safety -= own_semiopen_file_next_to_king;
		}
		if (!(Bitboards::files_bb[FILE_B] & white_pawns)) {
			black_queen_wing_safety -= enemy_semiopen_king_file;
			white_queen_wing_safety -= own_semiopen_king_file;
		}
		if (!(Bitboards::files_bb[FILE_C] & white_pawns)) {
			black_queen_wing_safety -= enemy_semiopen_file_next_to_king;
			white_queen_wing_safety -= own_semiopen_file_next_to_king;
		}
		// ***********************************************************

		// Pawn structure and properties
		// ***********************************************************
		int score = 0;
		Bitboard passed_pawns[2] = { 0, 0};
		int number_of_pawns[2] = {0, 0};
		// White pawns
		while(white_pawns) {
			number_of_pawns[WHITE]++;
			int pawn_index = Bitboards::bit_scan_forward(white_pawns);

			// Isolated pawn
			if ((neighbor_files[pawn_index & 7] & white_pawns) == 0) // pawn_index & 7 == pawn_file
				score += isolated_pawn;

			// Doubled pawn
			Bitboard pawns_on_file = Bitboards::files_bb[pawn_index & 7] & white_pawns;
			if (pawns_on_file & (pawns_on_file - 1))
				score += doubled_pawn;

			// Passed pawn
			if ((Bitboards::passed_pawn_mask[WHITE][pawn_index] & black_pawns) == 0) {
				Bitboards::set_bit(passed_pawns[WHITE], pawn_index);
			}

			// Backward pawn
			if (pawn_attacks[WHITE][pawn_index + 8] && black_pawns) {
				if (!(Bitboards::passed_pawn_mask[BLACK][pawn_index] & pos.get_piece_bitboard(WHITE, PAWN))) {
					score += backward_pawn;
				}
			}

			white_pawns &= white_pawns - 1;
		}

		// Black pawns
		while(black_pawns) {
			number_of_pawns[BLACK]++;
			int pawn_index = Bitboards::bit_scan_forward(black_pawns);

			// Isolated pawn
			if ((neighbor_files[pawn_index & 7] & black_pawns) == 0) // pawn_index & 7 == pawn_file
				score -= isolated_pawn;

			// Doubled pawn
			Bitboard pawns_on_file = Bitboards::files_bb[pawn_index & 7] & black_pawns;
			if (pawns_on_file & (pawns_on_file - 1))
				score -= doubled_pawn;

			// Passed pawn
			if ((Bitboards::passed_pawn_mask[BLACK][pawn_index] & pos.get_piece_bitboard(WHITE, PAWN)) == 0) {
				Bitboards::set_bit(passed_pawns[BLACK], pawn_index);
			}

			// Backward pawn
			if (pawn_attacks[BLACK][pawn_index - 8] && pos.get_piece_bitboard(WHITE, PAWN)) {
				if (!(Bitboards::passed_pawn_mask[WHITE][pawn_index] & pos.get_piece_bitboard(BLACK, PAWN))) {
					score -= backward_pawn;
				}
			}

			black_pawns &= black_pawns - 1;
		}
		// ***********************************************************

		// Saves the info in the pawn hash table
		pawns_info.number_of_pawns[WHITE] = number_of_pawns[WHITE];
		pawns_info.number_of_pawns[BLACK] = number_of_pawns[BLACK];
		pawns_info.passed_pawns[WHITE] = passed_pawns[WHITE];
		pawns_info.passed_pawns[BLACK] = passed_pawns[BLACK];
		pawns_info.score = score;
		pawns_info.pawn_targets[WHITE] = pawn_targets[WHITE];
		pawns_info.pawn_targets[BLACK] = pawn_targets[BLACK];
		pawns_info.king_wing_safety[WHITE] = white_king_wing_safety;
		pawns_info.king_wing_safety[BLACK] = black_king_wing_safety;
		pawns_info.queen_wing_safety[WHITE] = white_queen_wing_safety;
		pawns_info.queen_wing_safety[BLACK] = black_queen_wing_safety;
		store_hash_pawns(pos.get_pawns_key(), pawns_info);
	}
}
