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

#include "movegenerator.h"
#include "bitboards.h"
#include "attacks.h"

using namespace Attacks;
using namespace Bitboards;

namespace MoveGen {

	// Move ordering: captures and promotions
	const int MVVLVA[6][6] = {
			{22, 23, 24, 25, 21, 26}, // knight captured
			{32, 33, 34, 35, 31, 36}, // bishop captured
			{42, 43, 44, 45, 41, 46}, // rook captured
			{52, 53, 54, 55, 51, 56}, // queen captured
			{12, 13, 14, 15, 11, 16}, // pawn captured
			{62, 63, 64, 65, 61, 66} // king captured
	};
	int capture_score = 2048;
	int promotion_score = 3000;

	// Generate all moves
	void generate_knight_moves(Position &pos, Move_list &move_list);
	void generate_king_moves(Position &pos, Move_list &move_list);
	void generate_bishop_moves(Position &pos, Move_list &move_list);
	void generate_rook_moves(Position &pos, Move_list &move_list);
	void generate_queen_moves(Position &pos, Move_list &move_list);

	// Pawn moves
	void generate_pawn_moves(Position &pos, Move_list &move_list);
	void generate_white_pawns_moves(Position &pos, Move_list &move_list);
	void generate_black_pawns_moves(Position &pos, Move_list &move_list);

	// Captures
	void generate_knight_captures(Position &pos, Move_list &move_list);
	void generate_king_captures(Position &pos, Move_list &move_list);
	void generate_bishop_captures(Position &pos, Move_list &move_list);
	void generate_rook_captures(Position &pos, Move_list &move_list);
	void generate_queen_captures(Position &pos, Move_list &move_list);

	// Pawn captures
	void generate_pawn_captures(Position &pos, Move_list &move_list);
	void generate_white_pawns_captures(Position &pos, Move_list &move_list);
	void generate_black_pawns_captures(Position &pos, Move_list &move_list);

	// Promotions
	void generate_promotions(Position &pos, Move_list &move_list);
	void generate_white_promotions(Position &pos, Move_list &move_list);
	void generate_black_promotions(Position &pos, Move_list &move_list);

	// Extract moves
	void extract_moves(Position &pos, Bitboard targets, int from, int move_flags, Move_list &move_list, Piece_type piece);
	void extract_pawn_moves(Position &pos, Bitboard targets, int move_flags, Move_list &move_list, int direction);
	void extract_pawn_captures(Position &pos, Bitboard targets, int move_flags, Move_list &move_list, Direction capture_direction);
	void extract_pawn_push_promotions(Position &pos, Bitboard targets, Move_list &move_list, Direction direction);
	void extract_pawn_capture_promotions(Position &pos, Bitboard targets, Move_list &move_list, Direction direction);

	void add_move(Move_list &move_list, Move &move);

	// Normal move generator
	// ******************************************************************************
	/*
	 * Generate pseudolegal moves for this position.
	 */
	void generate_moves(Position &pos, Move_list &move_list) {
		generate_pawn_captures(pos, move_list);
		generate_knight_moves(pos, move_list);
		generate_bishop_moves(pos, move_list);
		generate_rook_moves(pos, move_list);
		generate_queen_moves(pos, move_list);
		generate_king_moves(pos, move_list);
		generate_pawn_moves(pos, move_list);
	}

	/*
	 * Generate pseudolegal knight moves for this position.
	 */
	void generate_knight_moves(Position &pos, Move_list &move_list) {
		Bitboard knights = pos.get_piece_bitboard(pos.get_side_to_move(), KNIGHT);
		while(knights) {
			int knight_index = Bitboards::bit_scan_forward(knights);
			Bitboard empty_targets = knight_attacks[knight_index] & pos.get_empty_squares();
			extract_moves(pos, empty_targets, knight_index, Move::QuietMove, move_list, KNIGHT);
			Bitboard capture_targets = knight_attacks[knight_index] & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, knight_index, Move::Capture, move_list, KNIGHT);
			knights &= knights - 1;
		}
	}

	/*
	 * Generate pseudolegal king moves for this position.
	 */
	void generate_king_moves(Position &pos, Move_list &move_list) {
		Bitboard empty_squares = pos.get_empty_squares();
		Bitboard king_bb = pos.get_piece_bitboard(pos.get_side_to_move(), KING);
		int king_index = Bitboards::bit_scan_forward(king_bb);
		Bitboard empty_targets = king_attacks[king_index] & empty_squares;
		extract_moves(pos, empty_targets, king_index, Move::QuietMove, move_list, KING);
		Bitboard capture_targets = king_attacks[king_index] & pos.get_occupied_squares(~pos.get_side_to_move());
		extract_moves(pos, capture_targets, king_index, Move::Capture, move_list, KING);
		// Castling
		// todo: improve castling moves generation
		if (pos.get_side_to_move() == WHITE) {
			if (king_index == E1) {
				Bitboard castling_moves = 0x0000000000000044 & empty_squares;
				Bitboard rook_castling = 0x0000000000000028 & empty_squares;
				castling_moves &= ((rook_castling << 1) ^ (rook_castling >> 1));
				castling_moves &= ((pos.get_piece_bitboard(pos.get_side_to_move(), ROOK) << 2) | (pos.get_piece_bitboard(pos.get_side_to_move(), ROOK) >> 1));
				extract_moves(pos, castling_moves, king_index, Move::Castling, move_list, KING);
			}
		}
		else {
			if (king_index == E8) {
				Bitboard castling_moves = 0x4400000000000000 & empty_squares;
				Bitboard rook_castling = 0x2800000000000000 & empty_squares;
				castling_moves &= ((rook_castling << 1) ^ (rook_castling >> 1));
				castling_moves &= ((pos.get_piece_bitboard(pos.get_side_to_move(), ROOK) << 2) | (pos.get_piece_bitboard(pos.get_side_to_move(), ROOK) >> 1));
				extract_moves(pos, castling_moves, king_index, Move::Castling, move_list, KING);
			}
		}
	}

	/*
	 * Generate pseudolegal bishop moves for this position.
	 */
	void generate_bishop_moves(Position &pos, Move_list &move_list) {
		Bitboard bishops = pos.get_piece_bitboard(pos.get_side_to_move(), BISHOP);
		while (bishops) {
			int bishop_index = Bitboards::bit_scan_forward(bishops);
			Bitboard bishop_attacks = get_bishop_attacks(pos.get_occupancy(), bishop_index);
			Bitboard empty_targets = bishop_attacks & pos.get_empty_squares();
			extract_moves(pos, empty_targets, bishop_index, Move::QuietMove, move_list, BISHOP);
			Bitboard capture_targets = bishop_attacks & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, bishop_index, Move::Capture, move_list, BISHOP);
			bishops &= bishops - 1;
		}
	}

	/*
	 * Generate pseudolegal rook moves for this position.
	 */
	void generate_rook_moves(Position &pos, Move_list &move_list) {
		Bitboard rooks = pos.get_piece_bitboard(pos.get_side_to_move(), ROOK);
		while (rooks) {
			int rook_index = Bitboards::bit_scan_forward(rooks);
			Bitboard rook_attacks = get_rook_attacks(pos.get_occupancy(), rook_index);
			Bitboard empty_targets = rook_attacks & pos.get_empty_squares();
			extract_moves(pos, empty_targets, rook_index, Move::QuietMove, move_list, ROOK);
			Bitboard capture_targets = rook_attacks & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, rook_index, Move::Capture, move_list, ROOK);
			rooks &= rooks - 1;
		}
	}

	/*
	 * Generate pseudolegal queen moves for this position.
	 */
	void generate_queen_moves(Position &pos, Move_list &move_list) {
		Bitboard queens = pos.get_piece_bitboard(pos.get_side_to_move(), QUEEN);
		while (queens) {
			int queen_index = Bitboards::bit_scan_forward(queens);
			Bitboard queen_attacks = get_queen_attacks(pos.get_occupancy(), queen_index);
			Bitboard empty_targets = queen_attacks & pos.get_empty_squares();
			extract_moves(pos, empty_targets, queen_index, Move::QuietMove, move_list, QUEEN);
			Bitboard capture_targets = queen_attacks & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, queen_index, Move::Capture, move_list, QUEEN);
			queens &= queens - 1;
		}
	}

	/*
	 * Generate pseudolegal pawn moves, without captures, for this position.
	 */
	void generate_pawn_moves(Position &pos, Move_list &move_list) {
		if (pos.get_side_to_move() == WHITE)
			generate_white_pawns_moves(pos, move_list);
		else
			generate_black_pawns_moves(pos, move_list);
	}

	/*
	 * Generate pseudolegal white pawn moves, without captures, for this position.
	 */
	void generate_white_pawns_moves(Position &pos, Move_list &move_list) {
		Bitboard empty_squares = pos.get_empty_squares();
		Bitboard single_push_targets = (pos.get_piece_bitboard(WHITE, PAWN) << 8) & empty_squares;
		Bitboard double_push_targets = (single_push_targets << 8) & Bitboards::ranks_bb[RANK_4] & empty_squares;
		Bitboard promotions = single_push_targets & Bitboards::ranks_bb[RANK_8];
		single_push_targets &= not_8_rank;
		extract_pawn_moves(pos, single_push_targets, Move::QuietMove, move_list, NORTH);
		extract_pawn_moves(pos, double_push_targets, Move::DoublePawnPush, move_list, NORTH+NORTH);
		extract_pawn_push_promotions(pos, promotions, move_list, NORTH);
	}

	/*
	 * Generate pseudolegal black pawn moves, without captures, for this position.
	 */
	void generate_black_pawns_moves(Position &pos, Move_list &move_list) {
		Bitboard empty_squares = pos.get_empty_squares();
		Bitboard single_push_targets = (pos.get_piece_bitboard(BLACK, PAWN) >> 8) & empty_squares;
		Bitboard double_push_targets = (single_push_targets >> 8) & Bitboards::ranks_bb[RANK_5] & empty_squares;
		Bitboard promotions = single_push_targets & Bitboards::ranks_bb[RANK_1];
		single_push_targets &= not_1_rank;
		extract_pawn_moves(pos, single_push_targets, Move::QuietMove, move_list, SOUTH);
		extract_pawn_moves(pos, double_push_targets, Move::DoublePawnPush, move_list, SOUTH+SOUTH);
		extract_pawn_push_promotions(pos, promotions, move_list, SOUTH);
	}

	/*
	 * Add pawn moves to the list.
	 */
	void extract_pawn_moves(Position &pos, Bitboard targets, int move_flags, Move_list &move_list, int direction) {
		while (targets) {
			int to = Bitboards::bit_scan_forward(targets);
			Move move(move_flags, to - direction, to);
			add_move(move_list, move);
			targets &= targets - 1;
		}
	}
	// ******************************************************************************

	// Captures move generator
	// ******************************************************************************

	/*
	 * Generate pseudolegal captures for this position.
	 */
	void generate_captures(Position &pos, Move_list &move_list) {
		generate_pawn_captures(pos, move_list);
		generate_knight_captures(pos, move_list);
		generate_bishop_captures(pos, move_list);
		generate_rook_captures(pos, move_list);
		generate_queen_captures(pos, move_list);
		generate_king_captures(pos, move_list);
	}

	/*
	 * Generate pseudolegal knight captures for this position.
	 */
	void generate_knight_captures(Position &pos, Move_list &move_list) {
		Bitboard knights = pos.get_piece_bitboard(pos.get_side_to_move(), KNIGHT);
		while(knights) {
			int knight_index = Bitboards::bit_scan_forward(knights);
			Bitboard capture_targets = knight_attacks[knight_index] & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, knight_index, Move::Capture, move_list, KNIGHT);
			knights &= knights - 1;
		}
	}

	/*
	 * Generate pseudolegal bishop captures for this position.
	 */
	void generate_bishop_captures(Position &pos, Move_list &move_list) {
		Bitboard bishops = pos.get_piece_bitboard(pos.get_side_to_move(), BISHOP);
		while(bishops) {
			int bishop_index = Bitboards::bit_scan_forward(bishops);
			Bitboard bishop_attacks = get_bishop_attacks(~pos.get_empty_squares(), bishop_index);
			Bitboard capture_targets = bishop_attacks & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, bishop_index, Move::Capture, move_list, BISHOP);
			bishops &= bishops - 1;
		}
	}

	/*
	 * Generate pseudolegal rook captures for this position.
	 */
	void generate_rook_captures(Position &pos, Move_list &move_list) {
		Bitboard rooks = pos.get_piece_bitboard(pos.get_side_to_move(), ROOK);
		while(rooks) {
			int rook_index = Bitboards::bit_scan_forward(rooks);
			Bitboard rook_attacks = get_rook_attacks(~pos.get_empty_squares(), rook_index);
			Bitboard capture_targets = rook_attacks & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, rook_index, Move::Capture, move_list, ROOK);
			rooks &= rooks - 1;
		}
	}

	/*
	 * Generate pseudolegal queen captures for this position.
	 */
	void generate_queen_captures(Position &pos, Move_list &move_list) {
		Bitboard queens = pos.get_piece_bitboard(pos.get_side_to_move(), QUEEN);
		while(queens) {
			int queen_index = Bitboards::bit_scan_forward(queens);
			Bitboard queen_attacks = get_queen_attacks(~pos.get_empty_squares(), queen_index);
			Bitboard capture_targets = queen_attacks & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, queen_index, Move::Capture, move_list, QUEEN);
			queens &= queens - 1;
		}
	}

	/*
	 * Generate pseudolegal king captures for this position.
	 */
	void generate_king_captures(Position &pos, Move_list &move_list) {
		Bitboard king_bitboard = pos.get_piece_bitboard(pos.get_side_to_move(), KING);
		while(king_bitboard) {
			int king_index = Bitboards::bit_scan_forward(king_bitboard);
			Bitboard capture_targets = king_attacks[king_index] & pos.get_occupied_squares(~pos.get_side_to_move());
			extract_moves(pos, capture_targets, king_index, Move::Capture, move_list, KING);
			king_bitboard &= king_bitboard - 1;
		}
	}

	/*
	 * Generate pseudolegal pawn captures for this position.
	 */
	void generate_pawn_captures(Position &pos, Move_list &move_list) {
		if (pos.get_side_to_move() == WHITE)
			generate_white_pawns_captures(pos, move_list);
		else
			generate_black_pawns_captures(pos, move_list);
	}

	/*
	 * Generate pseudolegal white pawn captures for this position.
	 */
	void generate_white_pawns_captures(Position &pos, Move_list &move_list) {
		Bitboard left_targets = (pos.get_piece_bitboard(WHITE, PAWN) << 7) & not_H_file;
		Bitboard right_targets = (pos.get_piece_bitboard(WHITE, PAWN) << 9) & not_A_file;
		Bitboard left_captures = left_targets & not_8_rank & pos.get_occupied_squares(BLACK);
		Bitboard right_captures = right_targets & not_8_rank & pos.get_occupied_squares(BLACK);
		Bitboard left_promotions = left_targets & ranks_bb[RANK_8] & pos.get_occupied_squares(BLACK);
		Bitboard right_promotions = right_targets & ranks_bb[RANK_8] & pos.get_occupied_squares(BLACK);
		extract_pawn_captures(pos, left_captures, Move::Capture, move_list, NORTH_WEST);
		extract_pawn_captures(pos, right_captures, Move::Capture, move_list, NORTH_EAST);
		extract_pawn_capture_promotions(pos, left_promotions, move_list, NORTH_WEST);
		extract_pawn_capture_promotions(pos, right_promotions, move_list, NORTH_EAST);
		// Enpassant
		if (pos.get_enpassant_square() != NO_SQUARE) {
			Bitboard enpassant_target = 1;
			enpassant_target <<= pos.get_enpassant_square();
			if (enpassant_target & left_targets) {
				Move move(Move::Enpassant, pos.get_enpassant_square() - 7, pos.get_enpassant_square(), MVVLVA[PAWN][PAWN] + capture_score);
				add_move(move_list, move);
			}
			if (enpassant_target & right_targets) {
				Move move(Move::Enpassant, pos.get_enpassant_square() - 9, pos.get_enpassant_square(), MVVLVA[PAWN][PAWN] + capture_score);
				add_move(move_list, move);
			}
		}
	}

	/*
	 * Generate pseudolegal black pawn captures for this position.
	 */
	void generate_black_pawns_captures(Position &pos, Move_list &move_list) {
		Bitboard left_targets = (pos.get_piece_bitboard(BLACK, PAWN) >> 7) & not_A_file;
		Bitboard right_targets = (pos.get_piece_bitboard(BLACK, PAWN) >> 9) & not_H_file;
		Bitboard left_captures = left_targets & not_1_rank & pos.get_occupied_squares(WHITE);
		Bitboard right_captures = right_targets & not_1_rank & pos.get_occupied_squares(WHITE);
		Bitboard left_promotions = left_targets & ranks_bb[RANK_1] & pos.get_occupied_squares(WHITE);
		Bitboard right_promotions = right_targets & ranks_bb[RANK_1] & pos.get_occupied_squares(WHITE);
		extract_pawn_captures(pos, left_captures, Move::Capture, move_list, SOUTH_EAST);
		extract_pawn_captures(pos, right_captures, Move::Capture, move_list, SOUTH_WEST);
		extract_pawn_capture_promotions(pos, left_promotions, move_list, SOUTH_EAST);
		extract_pawn_capture_promotions(pos, right_promotions, move_list, SOUTH_WEST);
		// Enpassant
		if (pos.get_enpassant_square() != NO_SQUARE) {
			Bitboard enpassant_target = 1;
			enpassant_target <<= pos.get_enpassant_square();
			if (enpassant_target & left_targets) {
				Move move(Move::Enpassant, pos.get_enpassant_square() + 7, pos.get_enpassant_square(), MVVLVA[PAWN][PAWN] + capture_score);
				add_move(move_list, move);
			}
			if (enpassant_target & right_targets) {
				Move move(Move::Enpassant, pos.get_enpassant_square() + 9, pos.get_enpassant_square(), MVVLVA[PAWN][PAWN] + capture_score);
				add_move(move_list, move);
			}
		}
	}

	/*
	 * Add pawn captures to the move list.
	 */
	void extract_pawn_captures(Position &pos, Bitboard targets, int move_flags, Move_list &move_list, Direction capture_direction) {
		while (targets) {
			int to = Bitboards::bit_scan_forward(targets);
			Move move(move_flags, to - capture_direction, to, MVVLVA[pos.get_piece(to)][PAWN] + capture_score);
			add_move(move_list, move);
			targets &= targets - 1;
		}
	}
	// ******************************************************************************

	// Promotions move generator
	// ******************************************************************************
	/*
	 * Generate pseudolegal promotions for this position.
	 */
	void generate_promotions(Position &pos, Move_list &move_list) {
		if (pos.get_side_to_move() == WHITE)
			generate_white_promotions(pos, move_list);
		else
			generate_black_promotions(pos, move_list);
	}

	/*
	 * Generate pseudolegal white promotions for this position.
	 */
	void generate_white_promotions(Position &pos, Move_list &move_list) {
		Bitboard single_push_targets = (pos.get_piece_bitboard(WHITE, PAWN) << 8) & pos.get_empty_squares();
		Bitboard promotions = single_push_targets & Bitboards::ranks_bb[RANK_8];
		extract_pawn_push_promotions(pos, promotions, move_list, NORTH);

	}

	/*
	 * Generate pseudolegal black promotions for this position.
	 */
	void generate_black_promotions(Position &pos, Move_list &move_list) {
		Bitboard single_push_targets = (pos.get_piece_bitboard(BLACK, PAWN) >> 8) & pos.get_empty_squares();
		Bitboard promotions = single_push_targets & Bitboards::ranks_bb[RANK_1];
		extract_pawn_push_promotions(pos, promotions, move_list, SOUTH);

	}

	/*
	 * Add pawn promotions to the move list.
	 */
	void extract_pawn_push_promotions(Position &pos, Bitboard targets, Move_list &move_list, Direction direction) {
		while (targets) {
			int to = Bitboards::bit_scan_forward(targets);
			Move move1(Move::PromotedKnight, to - direction, to, promotion_score);
			Move move2(Move::PromotedQueen, to - direction, to, promotion_score + 1);
			Move move3(Move::PromotedRook, to - direction, to, promotion_score);
			Move move4(Move::PromotedBishop, to - direction, to, promotion_score);
			add_move(move_list, move1);
			add_move(move_list, move2);
			add_move(move_list, move3);
			add_move(move_list, move4);
			targets &= targets - 1;
		}
	}

	/*
	 * Add pawn captures to the move list.
	 */
	void extract_pawn_capture_promotions(Position &pos, Bitboard targets, Move_list &move_list, Direction direction) {
		while (targets) {
			int to = Bitboards::bit_scan_forward(targets);
			Move move1(Move::Capture | Move::PromotedKnight, to - direction, to, MVVLVA[pos.get_piece(to)][PAWN] + promotion_score + capture_score);
			Move move2(Move::Capture | Move::PromotedQueen, to - direction, to, MVVLVA[pos.get_piece(to)][PAWN] + promotion_score + capture_score + 1);
			Move move3(Move::Capture | Move::PromotedRook, to - direction, to, MVVLVA[pos.get_piece(to)][PAWN] + promotion_score + capture_score);
			Move move4(Move::Capture | Move::PromotedBishop, to - direction, to, MVVLVA[pos.get_piece(to)][PAWN] + promotion_score + capture_score);
			add_move(move_list, move1);
			add_move(move_list, move2);
			add_move(move_list, move3);
			add_move(move_list, move4);
			targets &= targets - 1;
		}
	}
	// ******************************************************************************

	/*
	 * Add moves to the move list.
	 */
	void extract_moves(Position &pos, Bitboard targets, int from, int move_flags, Move_list &move_list, Piece_type piece) {
		while(targets) {
			int to = Bitboards::bit_scan_forward(targets);
			Move move(move_flags, from, to);
			if (move_flags & Move::Capture) {
				move.set_score(MVVLVA[pos.get_piece(to)][piece] + capture_score);
			}
			add_move(move_list, move);
			targets &= targets - 1;
		}
	}

	/*
	 * Add a move to the list.
	 */
	inline void add_move(Move_list &move_list, Move &move) {
		move_list.moves[move_list.size] = move;
		move_list.size++;
	}
}
