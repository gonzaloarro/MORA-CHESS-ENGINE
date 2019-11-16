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

#ifndef SRC_TRANSPOSITIONTABLE_H_
#define SRC_TRANSPOSITIONTABLE_H_

#include "types.h"
#include "move.h"
#include "position.h"

namespace Search {

	// Constants
	constexpr int PV_MAX_LENGTH = 32;
	/*
	 * Node types for transposition table.
	 */
	int constexpr HASH_EXACT = 0;
	int constexpr HASH_BETA = 1;
	int constexpr HASH_ALPHA = 2;

	/*
	 * Hash size limits.
	 */
	constexpr int MIN_HASH_SIZE = 1;
	constexpr int MAX_HASH_SIZE = 1024;

	/*
	 * Hash entry struct.
	 */
	struct Hash_entry {
		Key zobrist_key;
		Move best_move;
		int score;
		int depth;
		int node_type;
	};

	// Default Hash table size
	int constexpr default_hash_table_size = 0x100000 * 128; // 128 MB
	int constexpr default_hash_table_entries = default_hash_table_size / sizeof(Hash_entry);


	// Principal variation struct
	struct PV {
		Move moves[PV_MAX_LENGTH];
		int pv_length;
	};

	/*
	 * Principal variation to be displayed in the search.
	 */
	extern PV principal_variation;

	/*
	 * Initializes the transposition table.
	 */
	void init();

	/*
	 * Sets the transposition table size in megabytes.
	 */
	void set_transposition_table_size(int mb);

	/*
	 * Stores a hash entry into the hash table.
	 */
	void store_hash(Key key, Move best_move, int score, int depth, int node_type);

	/*
	 * Returns the score assigned to the position corresponding to the key,
	 * if the hash entry for the position exists.
	 */
	int probe_hash(Key key, int depth, int alpha, int beta, Move &pv_move);

	/*
	 * Loads the principal variation line
	 * into an array available for the search.
	 */
	void load_pv_line(int depth, Position &pos);
}

#endif /* SRC_TRANSPOSITIONTABLE_H_ */
