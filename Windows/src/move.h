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

#ifndef SRC_MOVE_H_
#define SRC_MOVE_H_

#include <string>

/*
 * Class that represents a move.
 * This structures uses an int to store in the first half the move code
 * and the score in the second half.
 * Move encoding:
 * 6 bits for to square
 * 6 bits for from square
 * 4 bits for flags
 */
class Move {

public:
	/*
	0000 quiet move
	0001 castling
	0010 double push
	0100 capture
	0101 enpassant
	1000 knight promoted
	1001 bishop promoted
	1010 rook promoted
	1011 queen promoted
	 */
	enum Move_type {
		QuietMove = 0,
		Castling = 1,
		DoublePawnPush = 2,
		Capture = 4,
		Enpassant = 5,
		Promotion = 8,
		PromotedKnight = 8,
		PromotedBishop = 9,
		PromotedRook = 10,
		PromotedQueen = 11
	};

	// Default = 0 (invalid move)
	Move() {
		move = 0;
	}
	// Regular constructor
	Move(unsigned int info, unsigned int from, unsigned int to) {
		move = (info << 12) | (from << 6) | (to & 0x3f);
	}

	// Score assigned
	Move(unsigned int info, unsigned int from, unsigned int to, int score) {
		move = (score << 16) | (info << 12) | (from << 6) | (to & 0x3f);
	}

	// Move info
	unsigned int get_move() const;
	unsigned int get_to() const;
	unsigned int get_from() const;
	bool is_capture() const;
	bool is_enpassant() const;
	bool is_double_pawn_push() const;
	bool is_promotion() const;
	unsigned int get_promoted_piece() const;
	bool is_castling() const;
	bool is_null() const;

	// Operators
	inline bool operator==(Move m) {
		return (move & 0xFFFF) == m.get_move();
	}
	inline bool operator!=(Move m) {
		return (move & 0xFFFF) != m.get_move();
	}

	// Move score
	unsigned int get_score() const;
	void set_score(unsigned int s);

	// Notation
	std::string long_algebraic_notation() const;

private:
	// Data members
	unsigned int move;
};

inline unsigned int Move::get_move() const {
	return move & 0xFFFF;
}

inline unsigned int Move::get_to() const {
	return move & 0x3f;
}

inline unsigned int Move::get_from() const {
	return (move >> 6) & 0x3f;
}

inline bool Move::is_capture() const {
	return (move >> 12 & 0xF) & Capture;
}

inline bool Move::is_enpassant() const {
	return (move >> 12 & 0xF) == Enpassant;
}

inline bool Move::is_double_pawn_push() const {
	return (move >> 12 & 0xF) == DoublePawnPush;
}

inline bool Move::is_promotion() const {
	return (move >> 12 & 0xF) & Promotion;
}

inline unsigned int Move::get_promoted_piece() const {
	return (move >> 12 & 0xF) & 3;
}

inline bool Move::is_castling() const {
	return (move >> 12 & 0xF) == Castling;
}

inline bool Move::is_null() const {
	return move == 0;
}

inline unsigned int Move::get_score() const {
	return (move >> 16);
}

inline void Move::set_score(unsigned int s) {
	move |= (s << 16);
}

#endif /* SRC_MOVE_H_ */
