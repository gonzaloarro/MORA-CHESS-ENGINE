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

#include <random>
#include <cmath>
#include <iostream>

#include "position.h"
#include "attacks.h"
#include "evaluation.h"

/*
 * Zobrist Hashing.
 */
namespace Zobrist {
	constexpr int ZOBRIST_SEED = 3596592594;

	Key pieces[PIECE_TYPES][SQUARES];
	Key enpassant_square[FILES];
	Key castling_rights[16];
	Key black_to_move;
}

/*
 * Values to update castling rights depending on
 * the squares involved in a move.
 */
const int CASTLING_RIGHT_UPDATE[SQUARES] = {
		13, 15, 15, 15, 12, 15, 15, 14,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		7, 15, 15, 15, 3, 15, 15, 11
};

/*
 * Initializes static data.
 */
void Position::init() {
	init_hash_keys();
}

/*
 * Initializes random keys for Zobrist Hashing
 */
void Position::init_hash_keys() {
	std::mt19937_64 e2(Zobrist::ZOBRIST_SEED);
    std::uniform_int_distribution<long long int> dist(
    		std::llround(std::pow(2,61)),
    		std::llround(std::pow(2,62)));

    // Initializes a random key for each piece on each square
	for (int piece_type = 0; piece_type < PIECE_TYPES; piece_type++) {
		for (int square = 0; square < SQUARES; square++)
			Zobrist::pieces[piece_type][square] = dist(e2);
	}

    // Initializes a random key for an enpassant square on each file
	for (int file = FILE_A; file <= FILE_H; file++) {
		Zobrist::enpassant_square[file] = dist(e2);
	}

	for (int i = 0; i < 16; i++) {
		Zobrist::castling_rights[i] = dist(e2);
	}

	Zobrist::black_to_move = dist(e2);
}

/*
 * Constructor for the initial setup.
 */
Position::Position(std::string fen) {
	load_FEN(fen);
}

/*
 * Loads a position in FEN.
 */
void Position::load_FEN(std::string s) {

	reset_board();

	int rank = RANK_8;
	int file = FILE_A;
	int piece = EMPTY;
	int side = WHITE;
	int color[SQUARES];

	std::string::size_type i = 0;
	// Pieces
	while (s[i] != ' ') {
		char c = s[i];
		if (isalpha(c)) {
			if (isupper(c))
				side = WHITE;
			else
				side = BLACK;

			c = tolower(c);
			switch(c) {

			case('p'): piece = PAWN; break;
			case('r'): piece = ROOK; break;
			case('n'): piece = KNIGHT; break;
			case('b'): piece = BISHOP; break;
			case('q'): piece = QUEEN; break;
			case('k'): piece = KING; break;

			}
			int square = rank * 8 + file;
			Bitboards::set_bit(piece_bitboards[side][piece], square);
			board_mailbox[square] = piece;
			color[square] = side;
			file++;
		}

		if (isdigit(c)) {
			int count = c - '0';
			file += count;
		}
		if (c == '/') {
			file = FILE_A;
			rank--;
		}
		i++;
	}
	i++;
	// Side to move
	switch(s[i]) {
		case('w'): side_to_move = WHITE; break;
		case('b'): side_to_move = BLACK; break;
	}
	i += 2;
	// Castling rights
	while (s[i] != ' ') {
		switch(s[i]) {
		case('K'): castling_rights |= WHITE_SHORT; break;
		case('Q'): castling_rights |= WHITE_LONG; break;
		case('k'): castling_rights |= BLACK_SHORT; break;
		case('q'): castling_rights |= BLACK_LONG; break;
		}
		i++;
	}
	i++;
	// Enpassant square
	if (s[i] != '-') {
		int file = s[i] - 97;
		i++;
		int rank = s[i] - 49;
		enpassant_square = rank * 8 + file;
	}
	i += 2;
	// Fifty count
	while (s[i] != ' ') {
		int digit = s[i] - 48;
		fifty_count *= 10;
		fifty_count += digit;
		i++;
	}
	i++;
	// Plies
	while (s[i] != ' ' && i < s.size()) {
		int digit = s[i] - 48;
		history_ply *= 10;
		history_ply += digit;
		i++;
	}
	// Hash keys
	set_position_key(color);
	set_pawns_key(color);
	// Aditional bitboards
	for (int piece_type = KNIGHT; piece_type <= KING; piece_type++) { // TODO: loop depends on piece order definition
		occupied_squares[WHITE] |= piece_bitboards[WHITE][piece_type];
		occupied_squares[BLACK] |= piece_bitboards[BLACK][piece_type];
	}
	// Material
	init_material(color);
}

/*
 * Resets board state.
 */
void Position::reset_board() {
	reset_mailbox();
	reset_bitboards();
	fifty_count = 0;
	castling_rights = 0;
	enpassant_square = NO_SQUARE;
	side_to_move = WHITE;
	history_ply = 0;
	search_ply = 0;
	material[WHITE] = 0;
	material[BLACK] = 0;
}

/*
 * Resets the board mailbox.
 */
void Position::reset_mailbox() {
	for (int square = A1; square <= H8; square++) {
		board_mailbox[square] = EMPTY;
	}
}

/*
 * Resets the bitboards.
 */
void Position::reset_bitboards() {
	for (int piece_type = KNIGHT; piece_type <= KING; piece_type++) { // TODO: loop depends on piece order definition
		piece_bitboards[WHITE][piece_type] = 0x00;
		piece_bitboards[BLACK][piece_type] = 0x00;
	}
	occupied_squares[WHITE] = 0;
	occupied_squares[BLACK] = 0;
}

/*
 * Sets the position key for zobrist hashing.
 */
void Position::set_position_key(int * color) {
	position_key = 0;

	position_key ^= Zobrist::castling_rights[castling_rights];

	for (int square = A1; square <= H8; square++) {
		int piece = board_mailbox[square];
		if (piece != EMPTY)
			position_key ^= Zobrist::pieces[piece + color[square] * 6][square];
	}

	if (enpassant_square != NO_SQUARE) {
		int file = enpassant_square % 8;
		position_key ^= Zobrist::enpassant_square[file];
	}

	if (side_to_move == BLACK)
		position_key ^= Zobrist::black_to_move;
}

/*
 * Sets the pawns key for zobrist hashing.
 */
void Position::set_pawns_key(int * color) {
	pawns_key = 0;
	for (int square = A1; square <= H8; square++) {
		int piece = board_mailbox[square];
		if (piece == PAWN)
			pawns_key ^= Zobrist::pieces[PAWN + color[square] * 6][square];
	}
}

/*
 * Initializes the material for each side.
 */
void Position::init_material(int * color) {
	for (int square = A1; square <= H8; square++) {
		int piece_value = Evaluation::get_piece_value(board_mailbox[square]);
		if (color[square] == WHITE)
			material[WHITE] += piece_value;
		else
			material[BLACK] += piece_value;
	}
}

/*
 * Undo the last move played in this position.
 */
void Position::undo_move() {
	search_ply--;
	history_ply--;

	// Restore irreversible aspects of the position
	fifty_count = moves_history[history_ply].fifty_count;
	position_key = moves_history[history_ply].position_key;
	pawns_key = moves_history[history_ply].pawns_key;
	castling_rights = moves_history[history_ply].castling_rights;
	enpassant_square = moves_history[history_ply].enpassant_square;

	// Get move info
	Move move = moves_history[history_ply].move;
	int from = move.get_from();
	int to = move.get_to();

	// Update side to move
	side_to_move = ~side_to_move;

	int moved_piece = board_mailbox[to];
	Bitboards::clear_bit(piece_bitboards[side_to_move][moved_piece], to);
	Bitboards::clear_bit(occupied_squares[side_to_move], to);
	board_mailbox[to] = EMPTY;
	material[side_to_move] -= Evaluation::get_piece_value(moved_piece, to, side_to_move);

	// Promotion
	if (move.is_promotion()) {
		moved_piece = PAWN;

	}
	material[side_to_move] += Evaluation::get_piece_value(moved_piece, from, side_to_move);
	board_mailbox[from] = moved_piece;
	Bitboards::set_bit(piece_bitboards[side_to_move][moved_piece], from);
	Bitboards::set_bit(occupied_squares[side_to_move], from);

	// Capture
	if (move.is_capture()) {
		int captured_piece = moves_history[history_ply].captured_piece;
		int piece_captured_square = to;

		// Enpassant
		if (move.is_enpassant())
			piece_captured_square = enpassant_square - 8 + 16 * side_to_move;

		Bitboards::set_bit(piece_bitboards[~side_to_move][captured_piece], piece_captured_square);
		Bitboards::set_bit(occupied_squares[~side_to_move], piece_captured_square);
		board_mailbox[piece_captured_square] = captured_piece;
		material[~side_to_move] += Evaluation::get_piece_value(captured_piece, piece_captured_square, ~side_to_move);
	}

	// Castling move
	if (move.is_castling()) {
		int rook_from = to + 1;
		int rook_to = to - 1;
		if (to - from < 0) { // Queen castling
			rook_from = to - 2;
			rook_to = to + 1;
		}
		material[side_to_move] += Evaluation::get_piece_value(ROOK, rook_from, side_to_move);
		material[side_to_move] -= Evaluation::get_piece_value(ROOK, rook_to, side_to_move);

		board_mailbox[rook_from] = ROOK;
		board_mailbox[rook_to] = EMPTY;

		Bitboards::set_bit(piece_bitboards[side_to_move][ROOK], rook_from);
		Bitboards::set_bit(occupied_squares[side_to_move], rook_from);
		Bitboards::clear_bit(piece_bitboards[side_to_move][ROOK], rook_to);
		Bitboards::clear_bit(occupied_squares[side_to_move], rook_to);
	}
}

/*
 * Makes a move in this position.
 * Returns true if the move is legal, false otherwise.
 */
bool Position::make_move(Move &move) {
	// Save irreversive aspects of the position
	moves_history[history_ply].fifty_count = fifty_count;
	moves_history[history_ply].castling_rights = castling_rights;
	moves_history[history_ply].enpassant_square = enpassant_square;
	moves_history[history_ply].position_key = position_key;
	moves_history[history_ply].pawns_key = pawns_key;
	moves_history[history_ply].move = move;

	search_ply++;

	int from = move.get_from();
	int to = move.get_to();

	int moved_piece = board_mailbox[from];
	int captured_piece = board_mailbox[to];

	material[side_to_move] -= Evaluation::get_piece_value(moved_piece, from, side_to_move);
	material[side_to_move] += Evaluation::get_piece_value(moved_piece, to, side_to_move);

	board_mailbox[from] = EMPTY;
	board_mailbox[to] = moved_piece;
	Bitboards::clear_bit(piece_bitboards[side_to_move][moved_piece], from);
	Bitboards::clear_bit(occupied_squares[side_to_move], from);
	Bitboards::set_bit(piece_bitboards[side_to_move][moved_piece], to);
	Bitboards::set_bit(occupied_squares[side_to_move], to);
	position_key ^= Zobrist::pieces[moved_piece + side_to_move * 6][from];
	position_key ^= Zobrist::pieces[moved_piece + side_to_move * 6][to];

	if (moved_piece == PAWN) {
		fifty_count = -1;
		pawns_key ^= Zobrist::pieces[PAWN + side_to_move * 6][from];
		pawns_key ^= Zobrist::pieces[PAWN + side_to_move * 6][to];

		// Promotion
		if (move.is_promotion()) {
			int promoted_piece = move.get_promoted_piece();
			board_mailbox[to] = promoted_piece;
			Bitboards::set_bit(piece_bitboards[side_to_move][promoted_piece], to);
			Bitboards::clear_bit(piece_bitboards[side_to_move][moved_piece], to);
			position_key ^= Zobrist::pieces[moved_piece + side_to_move * 6][to];
			position_key ^= Zobrist::pieces[promoted_piece + side_to_move * 6][to];
			pawns_key ^= Zobrist::pieces[PAWN + side_to_move * 6][to];
			material[side_to_move] -= Evaluation::get_piece_value(moved_piece, to, side_to_move);
			material[side_to_move] += Evaluation::get_piece_value(promoted_piece, to, side_to_move);
		}
	}

	// Capture
	if (move.is_capture()) {
		int capture_square = to;
		if (move.is_enpassant()) {
			captured_piece = PAWN;
			capture_square = enpassant_square - 8 + side_to_move * 16;
			board_mailbox[capture_square] = EMPTY;
		}
		Bitboards::clear_bit(piece_bitboards[~side_to_move][captured_piece], capture_square);
		Bitboards::clear_bit(occupied_squares[~side_to_move], capture_square);
		position_key ^= Zobrist::pieces[captured_piece + ~side_to_move * 6][capture_square];
		fifty_count = -1;
		material[~side_to_move] -= Evaluation::get_piece_value(captured_piece, capture_square, ~side_to_move);
		moves_history[history_ply].captured_piece = captured_piece;
		if (captured_piece == PAWN)
			pawns_key ^= Zobrist::pieces[PAWN + ~side_to_move * 6][capture_square];
	}
	history_ply++;
	// Castling
	bool illegal_castling = false;
	if (move.is_castling()) {
		int rook_from = to + 1;
		int rook_to = to - 1;
		int needed_right = side_to_move == WHITE ? WHITE_SHORT : BLACK_SHORT;
		if (to - from < 0) { // Queen castling
			rook_from = to - 2;
			rook_to = to + 1;
			needed_right = side_to_move == WHITE ? WHITE_LONG : BLACK_LONG;
			if (board_mailbox[rook_from + 1] != EMPTY)
				illegal_castling = true;
		}

		board_mailbox[rook_from] = EMPTY;
		board_mailbox[rook_to] = ROOK;

		Bitboards::clear_bit(piece_bitboards[side_to_move][ROOK], rook_from);
		Bitboards::clear_bit(occupied_squares[side_to_move], rook_from);
		position_key ^= Zobrist::pieces[ROOK + side_to_move * 6][rook_from];
		Bitboards::set_bit(piece_bitboards[side_to_move][ROOK], rook_to);
		Bitboards::set_bit(occupied_squares[side_to_move], rook_to);
		position_key ^= Zobrist::pieces[ROOK + side_to_move * 6][rook_to];

		material[side_to_move] -= Evaluation::get_piece_value(ROOK, rook_from, side_to_move);
		material[side_to_move] += Evaluation::get_piece_value(ROOK, rook_to, side_to_move);

		illegal_castling =	 illegal_castling ||
							!(castling_rights & needed_right) ||
							is_attacked(rook_to, ~side_to_move) ||
							is_attacked(from, ~side_to_move);

	}

	int king_square = Bitboards::bit_scan_forward(piece_bitboards[side_to_move][KING]);

	side_to_move = ~side_to_move;
	fifty_count++;

	if (is_attacked(king_square, side_to_move) || illegal_castling) {
		undo_move();
		return false;
	}

	if (enpassant_square != NO_SQUARE) {
		position_key ^= Zobrist::enpassant_square[enpassant_square & 7];
		enpassant_square = NO_SQUARE;
	}
	if (move.is_double_pawn_push()) {
		enpassant_square = to + 8 - side_to_move * 16;
		position_key ^= Zobrist::enpassant_square[enpassant_square & 7];
	}

	position_key ^= Zobrist::black_to_move; // remove it or add it depending on side to move

	position_key ^= Zobrist::castling_rights[castling_rights];
	castling_rights &= CASTLING_RIGHT_UPDATE[from];
	castling_rights &= CASTLING_RIGHT_UPDATE[to];
	position_key ^= Zobrist::castling_rights[castling_rights];

	return true;
}

/*
 * Returns true if the square is attacked by side in
 * the current position.
 */
bool Position::is_attacked(int square, Color side) const {
	Bitboard occupied = occupied_squares[WHITE] | occupied_squares[BLACK];
	return
			(Attacks::knight_attacks[square] & piece_bitboards[side][KNIGHT]) |
			(Attacks::king_attacks[square] & piece_bitboards[side][KING]) |
			(Attacks::pawn_attacks[~side][square] & piece_bitboards[side][PAWN]) |
			(Attacks::get_bishop_attacks(occupied, square) & (piece_bitboards[side][BISHOP] | piece_bitboards[side][QUEEN])) |
			(Attacks::get_rook_attacks(occupied, square) & (piece_bitboards[side][ROOK] | piece_bitboards[side][QUEEN]));
}

/*
 * Returns true if the current position has already appear.
 */
bool Position::is_repetition() const {
	for (int i = history_ply - fifty_count; i < history_ply; i++) {
		if (position_key == moves_history[i].position_key)
			return true;
	}
	return false;
}

/*
 * Returns true if the side to move is in check.
 */
bool Position::in_check() const {
	int king_square = Bitboards::bit_scan_forward(piece_bitboards[side_to_move][KING]);
	return is_attacked(king_square, ~side_to_move);
}

/*
 * Returns true if the position is in the endgame (based on material).
 */
bool Position::endgame() const {
	return material[WHITE] < 900 + Evaluation::get_piece_value(KING);
}

/*
 * Makes a null move for the null move pruning.
 */
void Position::make_null_move() {
	moves_history[history_ply].position_key = position_key;
	moves_history[history_ply].enpassant_square = enpassant_square;
	if (enpassant_square != NO_SQUARE) {
		position_key ^= Zobrist::enpassant_square[enpassant_square % 8];
		enpassant_square = NO_SQUARE;
	}
	history_ply++;
	search_ply++;
	position_key ^= Zobrist::black_to_move;

	side_to_move = ~side_to_move;
}

/*
 * Undo a null move.
 */
void Position::undo_null_move() {
	history_ply--;
	position_key = moves_history[history_ply].position_key;
	enpassant_square = moves_history[history_ply].enpassant_square;
	search_ply--;

	side_to_move = ~side_to_move;
}
