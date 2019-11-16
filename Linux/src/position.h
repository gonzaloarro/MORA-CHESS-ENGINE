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

#ifndef INCLUDE_POSITION_H_
#define INCLUDE_POSITION_H_

#include <string>

#include "bitboards.h"
#include "move.h"
#include "types.h"

// Constants
const std::string INITIAL_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/*
 * Class that represents a position on the board.
 */
class Position {
public:

	// Default: Initial Position.
	Position(const std::string fen = INITIAL_POSITION_FEN);

	// Initialization for Zobrist Hashing
	static void init();

	// Position representation
	Bitboard get_piece_bitboard(Color side, Piece_type piece_type) const;
	Bitboard get_occupied_squares(Color side) const;
	Key get_position_key() const;
	Key get_pawns_key() const;
	Color get_side_to_move() const;
	int get_fifty_count() const;
	int get_castling_rights() const;
	int get_enpassant_square() const;
	int get_history_ply() const;
	int get_search_ply() const;
	int get_material(Color side) const;
	Bitboard get_occupancy() const;
	Bitboard get_empty_squares() const;
	int get_piece(int square) const;

	// FEN Notation
	void load_FEN(std::string s);

	// Doing and undoing moves
	bool make_move(Move &move);
	void undo_move();
	void make_null_move();
	void undo_null_move();

	// Attacks
	bool is_attacked(int square, Color side) const;
	bool in_check() const;

	// Draw detection
	bool is_repetition() const;

	// Search
	void reset_search_ply();
	bool endgame() const;

private:

	// Constants
	static constexpr int MAX_GAME_MOVES = 512;

	/*
	 * To set flags representing different castling rights.
	 */
	enum Castling_right {
		WHITE_SHORT = 1,
		WHITE_LONG = 2,
		BLACK_SHORT = 4,
		BLACK_LONG = 8
	};

	/*
	 * Neccesary info to undo a move.
	 */
	struct History_move {
		Move move;
		Key position_key;
		Key pawns_key;
		int fifty_count;
		int castling_rights;
		int enpassant_square;
		int captured_piece;
	};

	// Data members
	Bitboard piece_bitboards[PLAYERS][PIECE_TYPES / PLAYERS];
	Bitboard occupied_squares[2];
	Key position_key;
	Key pawns_key;
	Color side_to_move;
	int board_mailbox[SQUARES];
	int fifty_count;
	int castling_rights;
	int enpassant_square;
	int history_ply;
	int search_ply;
	History_move moves_history[MAX_GAME_MOVES];
	int material[PLAYERS];

	// Initialization helpers
	void set_position_key(int * color);
	void set_pawns_key(int * color);
	void init_material(int * color);

	// load_FEN helpers
	void reset_board();
	void reset_mailbox();
	void reset_bitboards();

	// Zobrist hashing
	static void init_hash_keys();
};

inline Bitboard Position::get_piece_bitboard(Color side, Piece_type piece_type) const {
	return piece_bitboards[side][piece_type];
}

inline Bitboard Position::get_occupied_squares(Color side) const {
	return occupied_squares[side];
}

inline Key Position::get_position_key() const {
	return position_key;
}

inline Key Position::get_pawns_key() const {
	return pawns_key;
}

inline Color Position::get_side_to_move() const {
	return side_to_move;
}

inline int Position::get_fifty_count() const {
	return fifty_count;
}

inline int Position::get_castling_rights() const {
	return castling_rights;
}

inline int Position::get_enpassant_square() const {
	return enpassant_square;
}

inline int Position::get_history_ply() const {
	return history_ply;
}

inline int Position::get_search_ply() const {
	return search_ply;
}

inline int Position::get_material(Color side) const {
	return material[side];
}

inline Bitboard Position::get_occupancy() const {
	return occupied_squares[WHITE] | occupied_squares[BLACK];
}

inline Bitboard Position::get_empty_squares() const {
	return ~(occupied_squares[WHITE] | occupied_squares[BLACK]);
}

inline int Position::get_piece(int square) const {
	return board_mailbox[square];
}

inline void Position::reset_search_ply() {
	search_ply = 0;
}

#endif /* INCLUDE_POSITION_H_ */
