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

#include "transpositiontable.h"

namespace Search {

	Hash_table hash_table;
	PV principal_variation;

	// Helpers
	Move probe_pv_move(Position &pos);

	/*
	 * Stores a hash entry using "always replace" as the replacement strategy.
	 */
	void store_hash(Key key, Move best_move, int score, int depth, int node_type) {
		// Get the corresponding entry
		Hash_entry &hash_entry = hash_table.hash_entries[key % hash_table_entries];
		// Set the info
		hash_entry.zobrist_key = key;
		hash_entry.best_move = best_move;
		hash_entry.score = score;
		hash_entry.depth = depth;
		hash_entry.node_type = node_type;

	}

	/*
	 * Get the position score from the tranposition table
	 * and load the best move.
	 */
	int probe_hash(Key key, int depth, int alpha, int beta, Move &pv_move) {
		Hash_entry hash_entry = hash_table.hash_entries[key % hash_table_entries];
		if (hash_entry.zobrist_key == key) {
			pv_move = hash_entry.best_move;
			if (hash_entry.depth >= depth) { // Only use a value obtained with a deeper or equal search
				if (hash_entry.node_type == HASH_EXACT)
					return hash_entry.score;
				if (hash_entry.node_type == HASH_ALPHA && hash_entry.score <= alpha)
					return alpha;
				if (hash_entry.node_type == HASH_BETA && hash_entry.score >= beta)
					return beta;
			}
		}
		return -1;
	}

	/*
	 * Load the principal variation line in the
	 * transposition table.
	 */
	void load_pv_line(int depth, Position &pos) {
		Move move = probe_pv_move(pos);
		principal_variation.pv_length = 0;
		int i = 0;
		while (!move.is_null() && i < depth) {
			pos.make_move(move);
			principal_variation.moves[i] = move;
			move = probe_pv_move(pos);
			principal_variation.pv_length++;
			i++;
		}
		while (i > 0) {
			pos.undo_move();
			i--;
		}
	}

	/*
	 * Returns the hash move for this position, if any.
	 * Otherwise returns a null move.
	 */
	const Move null_move;

	Move probe_pv_move(Position &pos) {
		Hash_entry hash_entry = hash_table.hash_entries[pos.get_position_key() % hash_table_entries];
		if (hash_entry.zobrist_key == pos.get_position_key()) {
			return hash_entry.best_move;
		}
		return null_move;
	}
}
