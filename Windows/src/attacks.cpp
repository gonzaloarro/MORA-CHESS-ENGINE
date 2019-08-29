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

#include <cstdlib>
#include "attacks.h"

namespace {

// Precomputed magic multipliers.
// TODO: It is possible to write clean code to initialize these numbers instead of hardcode them.
const Bitboard bishop_magics[SQUARES] = {
	2305922208415879456,
	2450002178294317056,
	2328361231763636234,
	2315993846590472192,
	2305983888440166400,
	2306001339429195776,
	2308174008291491856,
	2882866730865295488,
	2378399798844721168,
	2308099379008913420,
	2305862835336421376,
	2305844125905356304,
	2956693763453379267,
	2326109487787675650,
	2310348812706648064,
	2314868900839393792,
	2326109216395493408,
	2497282289609749632,
	2900335766238412810,
	2594144871891601408,
	2595209464378425344,
	3602917162634715201,
	2332882203459477568,
	2315413176206009346,
	2307039278183613456,
	2325002549754864896,
	2344688759330832392,
	2632362778293305376,
	2882449447680221184,
	2306969046760947776,
	2551289326449297472,
	2308175073376538656,
	2305896369888166016,
	2378182152322818146,
	2310351011249782800,
	2305915579146961024,
	3476779050306240640,
	2308253139913410560,
	2307144901848203289,
	2306173413011505188,
	2306969562022674692,
	2333014140861809024,
	2379590552761991424,
	3026419087571421184,
	2643657034747085824,
	2316116991893574144,
	2314852408599117840,
	2459000722720556545,
	2307567090695933952,
	3170542959533752836,
	2330624352307388480,
	2341871877234370560,
	2306160836861822976,
	2314991227353174080,
	2307267985145866240,
	3611966071361276032,
	2306986527093692672,
	2306408433337447424,
	2594154768561635460,
	2305896065148190784,
	2594075034700022794,
	2341874349258179218,
	2305845345676959936,
	2306126694052004227
};

const Bitboard rook_magics[SQUARES] {
	2630102251644911749,
	2792231804940076050,
	3602884142894944384,
	3602884374854371376,
	2882444567859183648,
	3819054718207131904,
	3746996548371547144,
	2449960544522816515,
	2305852356146827264,
	2305966189949489152,
	2379615842603114496,
	2459106686540066816,
	2308939302711132416,
	2306405976905515520,
	2388044697546260608,
	2603093847498080384,
	2378252721852685312,
	3463833468584069376,
	2306142084968230912,
	2306166265848270912,
	2327236207106461952,
	2306408708013297792,
	2405216870149407236,
	2615834953870508704,
	2306142523321647104,
	2315703431672631296,
	2310346652193398784,
	3242802898069496320,
	2305984846348157440,
	2451227035855553536,
	3458765621922381956,
	2317102567165804800,
	2668382856794996992,
	2346393272928176160,
	2324446814945674304,
	2324983316221005824,
	2306126691820374032,
	2310815000929256448,
	2305915578071646340,
	2305855654671680544,
	2305860876279775232,
	2594091287325982720,
	2319353825462329344,
	2306986501442898432,
	2307127307648635392,
	2310347725575684098,
	2305844385785053192,
	2379307980819550216,
	2305896962601263616,
	2377918231982703104,
	2306406234884243968,
	2305852923074707520,
	2639180042441130560,
	2310364275249545728,
	3891673308382626304,
	3459934412181930080,
	3189534595729261058,
	2346415026938709010,
	3458773585062167042,
	2308099216470315530,
	2630104381961797906,
	2310364759377121322,
	3458770022251497540,
	2341878970410083586
};
}

namespace Attacks {

	// Non sliding pieces
	Bitboard knight_attacks[SQUARES];
	Bitboard pawn_attacks[PLAYERS][SQUARES];
	Bitboard king_attacks[SQUARES];
	Bitboard king_castling[PLAYERS][SQUARES];

	// Magic multipliers
	Magic bishop_magic_table[SQUARES];
	Magic rook_magic_table[SQUARES];

	// Sliding pieces
	Bitboard bishop_attacks[512][SQUARES]; // 256 KB
	Bitboard rook_attacks[4096][SQUARES]; // 2MB

	// Mobility
	int bishop_mobility[512][SQUARES];
	int rook_mobility[4096][SQUARES];

	// Attack tables initialization
	void init_knight_attacks();
	void init_pawn_attacks();
	void init_king_attacks();
	void init_magic_bitboards();

	/*
	 * Initializes the attack tables for move generation
	 * and evaluation.
	 */
	void init() {
		init_knight_attacks();
		init_pawn_attacks();
		init_king_attacks();
		init_magic_bitboards();
	}

	/*
	 * Initializes the knight attacks table.
	 */
	void init_knight_attacks() {
		for (int rank = RANK_1; rank <= RANK_8; rank++) {
			for (int file = FILE_A; file <= FILE_H; file++) {
				int square = rank * 8 + file;
				if (rank >= RANK_3 && file >= FILE_B)
					Bitboards::set_bit(knight_attacks[square], square + SOUTH + SOUTH_WEST);
				if (rank >= RANK_3 && file <= FILE_G)
					Bitboards::set_bit(knight_attacks[square], square + SOUTH + SOUTH_EAST);
				if (rank >= RANK_2 && file >= FILE_C)
					Bitboards::set_bit(knight_attacks[square], square + WEST + SOUTH_WEST);
				if (rank >= RANK_2 && file <= FILE_F)
					Bitboards::set_bit(knight_attacks[square], square + EAST + SOUTH_EAST);
				if (rank <= RANK_7 && file >= FILE_C)
					Bitboards::set_bit(knight_attacks[square], square + WEST + NORTH_WEST);
				if (rank <= RANK_7 && file <= FILE_F)
					Bitboards::set_bit(knight_attacks[square], square + EAST + NORTH_EAST);
				if (rank <= RANK_6 && file >= FILE_B)
					Bitboards::set_bit(knight_attacks[square], square + NORTH + NORTH_WEST);
				if (rank <= RANK_6 && file <= FILE_G)
					Bitboards::set_bit(knight_attacks[square], square + NORTH + NORTH_EAST);
			}
		}
	}

	/*
	 * Initializes the pawn attacks table for each side.
	 */
	void init_pawn_attacks() {
		for (int rank = RANK_1; rank <= RANK_8; rank++) {
			for (int file = FILE_A; file <= FILE_H; file++) {
				int square = rank * 8 + file;
				if (rank <= RANK_7) {
					if (file >= FILE_B)
						Bitboards::set_bit(pawn_attacks[WHITE][square], square + NORTH_WEST);
					if (file <= FILE_G)
						Bitboards::set_bit(pawn_attacks[WHITE][square], square + NORTH_EAST);
				}
				if (rank >= RANK_2) {
					if (file >= FILE_B)
						Bitboards::set_bit(pawn_attacks[BLACK][square], square + SOUTH_WEST);
					if (file <= FILE_G)
						Bitboards::set_bit(pawn_attacks[BLACK][square], square + SOUTH_EAST);
				}
			}
		}
	}

	/*
	 * Initializes the king attacks table.
	 */
	void init_king_attacks() {
		for (int rank = RANK_1; rank <= RANK_8; rank++) {
			for (int file = FILE_A; file <= FILE_H; file++) {
				int square = rank * 8 + file;
				if (rank >= RANK_2 && file >= FILE_B)
					Bitboards::set_bit(king_attacks[square], square + SOUTH_WEST);
				if (rank >= RANK_2)
					Bitboards::set_bit(king_attacks[square], square + SOUTH);
				if (rank >= RANK_2 && file <= FILE_G)
					Bitboards::set_bit(king_attacks[square], square + SOUTH_EAST);
				if (file >= FILE_B)
					Bitboards::set_bit(king_attacks[square], square + WEST);
				if (file <= FILE_G)
					Bitboards::set_bit(king_attacks[square], square + EAST);
				if (rank <= RANK_7 && file >= FILE_B)
					Bitboards::set_bit(king_attacks[square], square + NORTH_WEST);
				if (rank <= RANK_7)
					Bitboards::set_bit(king_attacks[square], square + NORTH);
				if (rank <= RANK_7 && file <= FILE_G)
					Bitboards::set_bit(king_attacks[square], square + NORTH_EAST);
			}
		}
	}

	// Auxiliary functions for magic bitboards
	Bitboard sliding_attacks(int square, Bitboard occupied_squares, Direction directions[4]);
	int distance(int square_1, int square_2);
	void load_magic_table(Magic magic_table[], Bitboard attack_table[][SQUARES], int mobility_table[][SQUARES], Direction directions[4], Piece_type piece);
	void load_magic_numbers();

	/*
	 * Loads the magic multipliers and initializes
	 * the attacks tables for the sliding pieces.
	 */
	void init_magic_bitboards() {
		load_magic_numbers();
		Direction bishop_directions[] = {SOUTH_WEST, SOUTH_EAST, NORTH_WEST, NORTH_EAST};
		load_magic_table(bishop_magic_table, bishop_attacks, bishop_mobility, bishop_directions, BISHOP);
		Direction rook_directions[] = {SOUTH, EAST, NORTH, WEST};
		load_magic_table(rook_magic_table, rook_attacks, rook_mobility, rook_directions, ROOK);
	}

	/*
	 * Loads the magic multipliers.
	 */
	void load_magic_numbers() {
		for (int square = A1; square <= H8; square++) {
			bishop_magic_table[square].magic_number = bishop_magics[square];
			rook_magic_table[square].magic_number = rook_magics[square];
		}
	}

	/*
	 * Loads the attack and mobility tables for the sliding pieces.
	 */
	void load_magic_table(Magic magic_table[], Bitboard attack_table[][64], int mobility_table[][64], Direction directions[4], Piece_type piece) {
		int shift = piece == BISHOP ? 55 : 52;
		for (int square = A1; square <= H8; square++) {
			// Compute the mask
			Bitboard empty = 0;
			int file = square % 8;
			int rank = square / 8;
	        Bitboard board_edges = ((Bitboards::ranks_bb[RANK_1] | Bitboards::ranks_bb[RANK_8]) & ~Bitboards::ranks_bb[rank]) | ((Bitboards::files_bb[FILE_A]  | Bitboards::files_bb[FILE_H]) & ~Bitboards::files_bb[file]);
			magic_table[square].mask = sliding_attacks(square, empty, directions) & ~board_edges;

			// Compute the attack bitboard and mobility for each subset of the mask
			Bitboard b = 0;
			int table_size = 0;
			do {
				Bitboard index = ((b & magic_table[square].mask) * magic_table[square].magic_number) >> shift;
				attack_table[index][square] = sliding_attacks(square, b, directions);
				mobility_table[index][square] = Bitboards::population_count(attack_table[index][square]);
				table_size++;
				b = (b - magic_table[square].mask) & magic_table[square].mask;

			} while (b);
		}
	}

	/*
	 * Returns a bitboard with the attacks for a sliding piece on a certain square
	 * using the directions provided.
	 */
	Bitboard sliding_attacks(int square, Bitboard occupied_squares, Direction directions[4]) {
		Bitboard attacks = 0;
		for (int dir = 0; dir < 4; dir++) {
			int s = square + directions[dir];
			while (0 <= s && s < 64 && distance(s, s - directions[dir]) == 1) {
				Bitboard uno = 1;
				Bitboard set_mask = uno << s;
				Bitboards::set_bit(attacks, s);
				attacks |= set_mask;
				if (occupied_squares & set_mask)
					break;
				s += directions[dir];
			}
		}
		return attacks;
	}

	/*
	 * Returns the distance between two squares.
	 */
	int distance(int square_1, int square_2) {
		int file_1 = square_1 % 8;
		int RANK_1 = square_1 / 8;
		int file_2 = square_2 % 8;
		int RANK_2 = square_2 / 8;
		int files_distance = abs(file_1 - file_2);
		int ranks_distance = abs(RANK_1 - RANK_2);
		return files_distance > ranks_distance ? files_distance : ranks_distance;
	}


	/*
	 * Return bishop attacks.
	 */
	Bitboard get_bishop_attacks(Bitboard occupancy, int square) {
		occupancy &= bishop_magic_table[square].mask;
		occupancy *= bishop_magic_table[square].magic_number;
		occupancy >>= 55;
		return bishop_attacks[occupancy][square];
	}

	/*
	 * Return rook attacks.
	 */
	Bitboard get_rook_attacks(Bitboard occupancy, int square) {
		occupancy &= rook_magic_table[square].mask;
		occupancy *= rook_magic_table[square].magic_number;
		occupancy >>= 52;
		return rook_attacks[occupancy][square];
	}

	/*
	 * Return queen attacks.
	 */
	Bitboard get_queen_attacks(Bitboard occupancy, int square) {
		return get_bishop_attacks(occupancy, square) | get_rook_attacks(occupancy, square);
	}

}
