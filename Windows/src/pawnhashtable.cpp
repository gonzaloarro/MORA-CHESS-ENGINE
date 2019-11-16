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

#include <vector>

#include "pawnhashtable.h"

namespace Evaluation {

	// Hash table size
	constexpr int pawn_hash_table_entries = 65536;

	/*
	 * Pawn hash table.
	 */
	std::vector<Pawn_hash_entry> pawns_hash_table;

	void init() {
		pawns_hash_table.reserve(pawn_hash_table_entries);
	}

	/*
	 * Stores a hash entry.
	 */
	void store_hash_pawns(Key key, Pawns_info pawns_info) {
		// Get the corresponding entry
		Pawn_hash_entry &hash_entry = pawns_hash_table[key % pawns_hash_table.capacity()];
		// Set the info
		hash_entry.zobrist_key = key;
		hash_entry.pawns_info.passed_pawns[WHITE] = pawns_info.passed_pawns[WHITE];
		hash_entry.pawns_info.passed_pawns[BLACK] = pawns_info.passed_pawns[BLACK];
		hash_entry.pawns_info.pawn_targets[WHITE] = pawns_info.pawn_targets[WHITE];
		hash_entry.pawns_info.pawn_targets[BLACK] = pawns_info.pawn_targets[BLACK];
		hash_entry.pawns_info.number_of_pawns[WHITE] = pawns_info.number_of_pawns[WHITE];
		hash_entry.pawns_info.number_of_pawns[BLACK] = pawns_info.number_of_pawns[BLACK];
		hash_entry.pawns_info.king_wing_safety[WHITE] = pawns_info.king_wing_safety[WHITE];
		hash_entry.pawns_info.king_wing_safety[BLACK] = pawns_info.king_wing_safety[BLACK];
		hash_entry.pawns_info.queen_wing_safety[WHITE] = pawns_info.queen_wing_safety[WHITE];
		hash_entry.pawns_info.queen_wing_safety[BLACK] = pawns_info.queen_wing_safety[BLACK];
		hash_entry.pawns_info.score = pawns_info.score;
	}

	/*
	 * Get the position score from the pawn hash table
	 * and load the pawn structure info.
	 */
	bool probe_hash_pawns(Key key, Pawns_info &pawns_info) {
		Pawn_hash_entry hash_entry = pawns_hash_table[key % pawns_hash_table.capacity()];
		if (hash_entry.zobrist_key == key) {
			pawns_info = hash_entry.pawns_info;
			return true;
		}
		return false;
	}
}
