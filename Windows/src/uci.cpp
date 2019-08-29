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
#include <string>
#include <vector>
#include <sstream>

#include "uci.h"
#include "position.h"
#include "search.h"
#include "timemanagement.h"

using namespace std;

namespace UCI {

	/*
	 * Engine info.
	 */
	const struct {
		string name;
		string author;
	} engine_info = {"MORA", "Gonzalo Arro" }; // Gonzalo Arró

	// UCI Commands
	void position(vector<string> tokens, Position &pos);
	Search::Search_info go(vector<string> tokens, Position &pos);

	// Helpers
	Move parse_move(string s, Position &pos);

	/*
	 * UCI Loop
	 */
	void loop() {

		Position pos; // Position object to work with during the game.

		Search::Search_info search_info;

		// Read commands
		string line;
		while(getline(cin, line)) {
			stringstream ss(line);
			vector<string> tokens;
			string token;
			while(getline(ss, token, ' ')) {
				tokens.push_back(token);
			}

			string command = tokens[0];

			if (command == "uci") {
				cout << "id name " << engine_info.name << endl;
				cout << "id author " << engine_info.author << endl;
				cout << "uciok" << endl;
			}
			else if (command == "isready") {
				cout << "readyok" << endl;
			}
			else if (command == "ucinewgame") {
				// not neccesary right now...
			}
			else if (command == "position") {
				position(tokens, pos);
			}
			else if (command == "go") {
				search_info = go(tokens, pos);
				Search::search(pos, search_info);
			}
			else if (command == "stop") {
				// Not implemented
			}
			else if (command == "quit") {
				break;
			}
		}
	}

	/*
	 * Implements the UCI position command.
	 * Loads the starting position or a FEN and
	 * then makes moves over that position.
	 */
	void position(vector<string> tokens, Position &pos) {
		vector<string>::iterator it = tokens.begin();
		vector<string>::iterator end = tokens.end();
		it++;
		// Position
		if (it != end) {
			if (*it == "startpos") {
				pos.load_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
				it++;
			}
			else if (*it == "fen") {
				it++;
				string fen = "";
				while (it != end && *it != "moves") {
					fen += *it;
					it++;
					if (it != end && *it != "moves")
						fen += " ";
				}
				pos.load_FEN(fen);
			}
		}
		// Moves
		if (it != end && *it == "moves") {
			it++;
			while (it != end) {
				string move_string = *it;
				Move move = parse_move(move_string, pos);
				pos.make_move(move);
				it++;
			}
		}
	}

	/*
	 * Converts a move in string format to an
	 * internal Move object.
	 */
	Move parse_move(string s, Position &pos) {
		int from = (s[1] - '1') * 8 + (s[0] - 'a');
		int to = (s[3] - '1') * 8 + (s[2] - 'a');

		int info = 0;

		int moved_piece = pos.get_piece(from);

		// Double pawn push
		if (moved_piece == PAWN && abs(to - from) == 16)
			info |= DoublePawnPush;

		// Castling
		if (moved_piece == KING && abs(to - from) == 2)
			info |= Castling;

		// Promotion
		if (s.size() == 5) {
			info |= Promotion;
			switch(s[4]) {
			case('q'): info |= PromotedQueen; break;
			case('r'): info |= PromotedRook; break;
			case('b'): info |= PromotedBishop; break;
			}
		}

		// Capture
		int captured_piece = pos.get_piece(to);
		if (captured_piece != EMPTY) {
			info |= Capture;
		}
		// Enpassant Capture
		if (moved_piece == PAWN && to == pos.get_enpassant_square())
			info |= Enpassant;

		return Move(info, from, to);
	}

	/*
	 * Returns the necessary info for the search.
	 */
	Search::Search_info go(vector<string> tokens, Position &pos) {

		Time::Time_options options;
		options.infinite = false;
		options.moves_to_go = -1;

		vector<string>::iterator it = tokens.begin();
		vector<string>::iterator end = tokens.end();
		it++;
		int depth = 0;
		int movetime = 0;
		while(it != end) {
			if (*it == "wtime") {
				it++;
				if (pos.get_side_to_move() == WHITE)
					options.time_left = std::stoi(*it);
			}
			if (*it == "btime") {
				it++;
				if (pos.get_side_to_move() == BLACK)
					options.time_left = std::stoi(*it);
			}
			if (*it == "movestogo") {
				it++;
				options.moves_to_go = std::stoi(*it);
			}
			if (*it == "depth") {
				it++;
				depth = std::stoi(*it);
				options.infinite = true;
			}
			if (*it == "movetime") {
				it++;
				movetime = std::stoi(*it);
				options.infinite = true;
			}
			if (*it == "infinite") {
				options.infinite = true;
				depth = 16;
			}
			it++;
		}

		int time_to_search = movetime > 0 ? movetime : Time::get_time_to_search(options, pos.get_history_ply());

		Search::Search_info search_info;
		search_info.depth = depth;
		search_info.time_to_search = time_to_search;
		search_info.nodes = 0;
		search_info.start_time = Time::get_current_time_in_milliseconds();
		search_info.stop = false;

		return search_info;
	}
}
