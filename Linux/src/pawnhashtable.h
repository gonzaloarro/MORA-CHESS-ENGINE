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

#ifndef SRC_PAWNHASHTABLE_H_
#define SRC_PAWNHASHTABLE_H_

#include "types.h"
#include "bitboards.h"

namespace Evaluation {

struct Pawns_info {
	Bitboard passed_pawns[PLAYERS]; // Passed pawns location
	Bitboard pawn_targets[PLAYERS]; // Pawn attacks
	int number_of_pawns[PLAYERS]; 	// Number of pawns
	int king_wing_safety[PLAYERS]; 	// King safety score for king wing
	int queen_wing_safety[PLAYERS]; // King safety score for queen wing
	int score;						// Pawn structure score
};

  struct Pawn_hash_entry {
		Key zobrist_key;
		Pawns_info pawns_info;
  };

  /*
   * Initializes the pawns hash table.
   */
  void init();

  /*
   * Stores a hash entry into the pawns hash table.
   */
  void store_hash_pawns(Key key, Pawns_info pawns_info);

  /*
   * Returns the pawn score assigned to the pawn structure corresponding to the key,
   * if the hash entry for the structure exists.
   * Also loads aditional info about the pawn structure.
   */
  bool probe_hash_pawns(Key key, Pawns_info &pawns_info);
}

#endif /* SRC_PAWNHASHTABLE_H_ */
