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

#ifndef SRC_TYPES_H_
#define SRC_TYPES_H_

typedef unsigned long long Key; // Key type for zobrist hashing.

constexpr int SQUARES = 64; // number of squares
constexpr int FILES = 8; // number of files
constexpr int RANKS = 8; // number of ranks
constexpr int PLAYERS = 2; // number of players (white and black)
constexpr int PIECE_TYPES = 12; // Six different types and two colors


/*
 * Files.
 */
enum File {
	FILE_A,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H
};

/*
 * Ranks.
 */
enum Rank {
	RANK_1,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8
};

/*
 * Square constants. Special code for a non valid square.
 */
enum Square {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8, NO_SQUARE
};

/*
 * Piece types. Special code for empty square.
 */
enum Piece_type {
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	PAWN,
	KING,
	EMPTY
};

/*
 * Bitboard directions.
 */
enum Direction {
	SOUTH_WEST = -9,
	SOUTH = -8,
	SOUTH_EAST = -7,
	WEST = -1,
	EAST = 1,
	NORTH_WEST = 7,
	NORTH = 8,
	NORTH_EAST = 9
};

/*
 * Players.
 */
enum Color {
	WHITE,
	BLACK
};

constexpr Color operator~(Color c) {
  return Color(c ^ 1);
}

#endif /* SRC_TYPES_H_ */
