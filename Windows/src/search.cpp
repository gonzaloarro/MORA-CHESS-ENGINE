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

#include <iostream>
#include <climits>

#include "search.h"
#include "movegenerator.h"
#include "evaluation.h"
#include "timemanagement.h"
#include "transpositiontable.h"

namespace Search {

	// Move ordering
	/*
	 * Killer heuristic.
	 */
	const int killer_score = 1024;
	unsigned int killer_moves[2][MAX_DEPTH];
	/*
	 * History heuristic.
	 */
	int search_history[SQUARES][SQUARES];

	/*
	 * PV Move.
	 */
	const int pv_score = 5000;

	// Delta pruning
	const int safety_margin = 200;
	const int max_delta = 900;

	// Lazy evaluation
	const int positional_margin = 100;

	// Null move pruning
	const int R = 2;

	// Helpers
	void clear_search(Position &pos);
	void send_search_iteration_info(int score, int current_depth, Search_info &search_info);
	int alpha_beta(Position &pos, int alpha, int beta, int depth, Search_info &search_info, bool null_move_pruning);
	void set_next_move(MoveGen::Move_list &move_list, int move_num);
	int quiescence_search(Position &pos, int alpha, int beta, Search_info &search_info);

	/*
	 * Seach the position to a certain depth
	 * depending on the options specified in the
	 * search info.
	 * Prints the best move found.
	 */
	void search(Position &pos, Search_info &search_info) {
		Move best_move;
		clear_search(pos);

		if (search_info.depth == 0)
			search_info.depth = MAX_DEPTH;

		int alpha = -MATE_SCORE - 100;
		int beta = MATE_SCORE + 100;

		// Iterative deepening
		for (int current_depth = 1; current_depth <= search_info.depth; current_depth++) {
			// Iteration score
		    int score = alpha_beta(pos, alpha, beta, current_depth, search_info, true);

		    // Check for timeout
			if (Time::time_out(search_info.start_time, search_info.time_to_search) || search_info.stop)
		    	break; // the iteration didn't finish, ignore the values

			// Load the principal variation line and the best move
		    load_pv_line(current_depth, pos);
		    best_move = principal_variation.moves[0];

		    // Print info for UCI Protocol
		    send_search_iteration_info(score, current_depth, search_info);

		    // Check time before starting a new iteration
		    if (!Time::time_for_next_iteration(search_info.start_time, search_info.time_to_search))
		    	break;
		}
		// Send best move found
		std::cout << "bestmove " << best_move.long_algebraic_notation() << std::endl;
	}

	/*
	 * Clear the arrays for killer moves and
	 * history heuristic; also resets the search ply.
	 */
	void clear_search(Position &pos) {
		for (int i = 0; i < SQUARES; i++) {
			for (int j = 0; j < SQUARES; j++) {
				search_history[i][j] = 0;
			}
		}
		for (int i = 0; i < MAX_DEPTH; i++) {
			killer_moves[0][i] = 0;
			killer_moves[1][i] = 0;
		}
		pos.reset_search_ply();
	}

	/*
	 * Sends information about a search iteration
	 * using UCI Protocol.
	 */
    void send_search_iteration_info(int score, int depth, Search_info &search_info) {
    	// UCI command
    	std::cout << "info ";

    	// Score
    	if (abs(score) < MATE_SCORE - MAX_DEPTH)
    		// Normal score
    		std::cout << "score cp " << score;
    	else {
    		// Mate score
    		std::cout << "score mate ";
    		if (score > MATE_SCORE - MAX_DEPTH)
        		std::cout << (MATE_SCORE - score)/2 + 1;
    		else
    			std::cout << -((MATE_SCORE - score)/2 + 1);

    	}
    	// General info
    	std::cout << " depth " << depth;
    	std::cout << " nodes " << search_info.nodes;
	    long long searched_time = Time::get_current_time_in_milliseconds() - search_info.start_time;
    	std::cout << " time " << searched_time;

    	// Print principal variation
	    std::cout << " pv ";
	    for (int i = 0; i < principal_variation.pv_length; i++) {
	    	std::cout << principal_variation.moves[i].long_algebraic_notation() << " ";
	    }

	    // Finish
	    std::cout << std::endl;
    }

    /*
     * Search the position with the alpha beta algorithm.
     */
	int alpha_beta(Position &pos, int alpha, int beta, int depth, Search_info &search_info, bool null_move_pruning) {
		// Draw detection
		if (pos.get_search_ply() > 0 && (pos.get_fifty_count() >= 100 || pos.is_repetition()))
			return Evaluation::draw_score;

		// Probe the hash table for a score and a pv move
		Move pv_move;
		int hash_score = probe_hash(pos.get_position_key(), depth, alpha, beta, pv_move);
		if (hash_score != -1) {
			return hash_score;
		}

		// Leaf node
		if (depth == 0)
			return quiescence_search(pos, alpha, beta, search_info);

		bool in_check = pos.in_check();
		if(in_check) {
			// Search extension because of check
			depth++;
		}
		else {
			// Null move pruning
			if(null_move_pruning && pos.get_search_ply() > 0 && depth > R && !pos.endgame()) {
				pos.make_null_move();
				int score = -alpha_beta(pos, -beta, -beta+1, depth-R, search_info, false);
				pos.undo_null_move();
				if (score >= beta && abs(score) < MATE_SCORE - MAX_DEPTH) // @suppress("Invalid arguments")
					return beta;
			}
		}

		// Update search info
		search_info.nodes++;

	    // Generate moves
	    MoveGen::Move_list move_list;
    	generate_moves(pos, move_list);

    	// Set different scores using the pv move, killer move and history heuristic
    	for (int i = 0; i < move_list.size; i++) {
    		if (pv_move == move_list.moves[i]) // pv move
    			move_list.moves[i].set_score(pv_score);
    		else if (move_list.moves[i].get_move() == killer_moves[0][pos.get_search_ply()]) // killer
    			move_list.moves[i].set_score(killer_score);
    		else if (move_list.moves[i].get_move() == killer_moves[1][pos.get_search_ply()]) // killer
    			move_list.moves[i].set_score(killer_score);
    		else // history
    			move_list.moves[i].set_score(search_history[move_list.moves[i].get_from()][move_list.moves[i].get_to()]);
    	}


		// Variables for the search
		Move best_move;
		int node_type = HASH_ALPHA;
	    int max = INT_MIN;
	    int legal_moves = 0;
    	int searched_moves = 0;
    	int score;
	    // Search each move
	    for (int i = 0; i < move_list.size; i++) {
	    	set_next_move(move_list, i);
	    	if (pos.make_move(move_list.moves[i])) {
	    		legal_moves++;
	    		// PVS Search
	    		if (searched_moves == 0) {
	    			score = -alpha_beta(pos, -beta, -alpha, depth - 1, search_info, true);
	    		}
	    		else {
	    			// Late move reductions
		    		if ((searched_moves >= 4) & (!in_check) & (!move_list.moves[i].is_capture()) & (depth > 2))
			    		score = -alpha_beta(pos, -alpha-1, -alpha, depth - 2, search_info, true);
		    		else
		    			score = -alpha_beta(pos, -alpha-1, -alpha, depth - 1, search_info, true);

		    		if (score > alpha)
		    			score = -alpha_beta(pos, -beta, -alpha, depth - 1, search_info, true);
	    		}
	    		searched_moves++;
	        	pos.undo_move();
	    		// Return if timeout
	    		if ((search_info.nodes & 2047) == 0) {
	    			if (Time::time_out(search_info.start_time, search_info.time_to_search) || search_info.stop)
	    				return -1;
	    		}
	    		if (score > max) {
	    			best_move = move_list.moves[i];
	    			max = score;
		    		if (score > alpha) { // Alpha cutoff
			    		if (score >= beta) { // Beta cutoff
			    			store_hash(pos.get_position_key(), best_move, beta, depth, HASH_BETA);
			    			if (!move_list.moves[i].is_capture()) {
				    			killer_moves[1][pos.get_search_ply()] = killer_moves[0][pos.get_search_ply()];
				    			killer_moves[0][pos.get_search_ply()] = move_list.moves[i].get_move();
			    			}
			    			return beta;
			    		}
		    			alpha = score;
		    			node_type = HASH_EXACT;
		    			if (!move_list.moves[i].is_capture())
			    			search_history[move_list.moves[i].get_from()][move_list.moves[i].get_to()] += depth;
		    		}
	    		}
	    	}
	    }

	    // checkmate or stalemate
	    if (legal_moves == 0) {
	    	if (in_check)
	    		return (-MATE_SCORE + pos.get_search_ply());
	    	else
				return Evaluation::draw_score;
	    }

	    // store entry in hash table
		store_hash(pos.get_position_key(), best_move, alpha, depth, node_type);
	    return alpha;
	}

	/*
	 * Set the move with the best score to be chosen next.
	 */
	void set_next_move(MoveGen::Move_list &move_list, int move_num) {
		unsigned best_score = 0;
		int best_move_index = move_num;
		for (int i = move_num; i < move_list.size; i++) {
			if (move_list.moves[i].get_score() > best_score) {
				best_score = move_list.moves[i].get_score();
				best_move_index = i;
			}
		}
		Move temp = move_list.moves[move_num];
		move_list.moves[move_num] = move_list.moves[best_move_index];
		move_list.moves[best_move_index] = temp;
	}

	/*
	 * Expand the search until a quiet position is reached.
	 */
	int quiescence_search(Position &pos, int alpha, int beta, Search_info &search_info) {
		// Output info
		search_info.nodes++;

		// Draw detection
		if (Evaluation::insufficient_material(pos))
			return Evaluation::draw_score;

		// Lazy evaluation
		// Evaluate only material first and try to cutoff
		int stand_pat = Evaluation::evaluate_material(pos);

		if (stand_pat >= beta + positional_margin)
			return beta;

		// Full evaluation
		stand_pat += Evaluation::evaluate_positional_factors(pos);

		if (stand_pat >= beta)
			return beta;

		// Delta pruning all moves
		if (stand_pat < alpha - max_delta ) {
		   return alpha;
		}

		if (alpha < stand_pat)
			alpha = stand_pat;

		// Generate captures and promotions
	    MoveGen::Move_list move_list;
		if (pos.in_check())
			generate_moves(pos, move_list); // todo: it would be better to have a special move generator for check evasions
		else {
		    generate_captures(pos, move_list);
		    generate_promotions(pos, move_list);
		}

	    // Search each capture
	    for (int i = 0; i < move_list.size; i++) {
	    	set_next_move(move_list, i);
	    	// Delta pruning specific move
	    	if (move_list.moves[i].is_capture()) {
		    	int captured_piece = pos.get_piece(move_list.moves[i].get_to());
	    		if (Evaluation::get_piece_value(captured_piece) + safety_margin + stand_pat < alpha)
	    			continue;
	    	}
	    	if (pos.make_move(move_list.moves[i])) {
	    		int score = -quiescence_search(pos, -beta, -alpha, search_info);
	    		pos.undo_move();
	    		// Return if timeout
	    		if ((search_info.nodes & 2047) == 0) {
	    			if (Time::time_out(search_info.start_time, search_info.time_to_search) || search_info.stop)
	    				return -1;
	    		}
	    		if (score > alpha) {
		    		if (score >= beta) {
		    			return beta;
		    		}
	    			alpha = score;
	    		}
	    	}
	    }
	    return alpha;
	}
}
