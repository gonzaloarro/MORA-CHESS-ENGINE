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

#include "position.h"
#include "bitboards.h"
#include "attacks.h"
#include "uci.h"
#include "transpositiontable.h"
#include "pawnhashtable.h"

using namespace std;

int main() {

	// Initialization
	Position::init();
	Bitboards::init();
	Attacks::init();
	Search::init();
	Evaluation::init();

	// LICENSE
	cout << "**************************************************************" << endl;
	cout << "* MORA CHESS ENGINE (MCE) - Copyright (C) 2019 Gonzalo Arró. *" << endl;
	cout << "**************************************************************" << endl;
	cout << "This program comes with ABSOLUTELY NO WARRANTY." << endl;;
	cout << "This is free software, and you are welcome to redistribute it under certain conditions." << endl;
	cout << "See COPYING file for details." << endl << endl;

	// UCI Protocol
	UCI::loop();

	return 0;
}
