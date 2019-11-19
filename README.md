# MORA CHESS ENGINE

Being a software engineering student, I found myself struggling to come with an idea for my final project. I love to play chess and I love to code. So one day while I was reading about [AlphaZero](https://en.wikipedia.org/wiki/AlphaZero) I started to wonder how difficult could it be to write my own chess engine. After a while I realized that, being challenging enough, writing a chess engine was a feasible project within 6 months. So I presented the idea to my project director and started to work.

The main goal was for my engine to reach 2000 ELO in the [Computer Chess Rating Lists](https://ccrl.chessdom.com/ccrl/). After a lot of hard work, I think (based on some experiments) MORA CHESS ENGINE is in the 2100-2200 range.

***UPDATE:** MORA CHESS ENGINE 1.0.0 is rated **2182 ELO** under [40 moves in 4 minutes time control](https://www.computerchess.org.uk/ccrl/404/) and **2203** under [40 moves in 40 minutes](https://www.computerchess.org.uk/ccrl/4040/). The new version 1.1.0 is almost the same but I would say that is probably 20 ELO points stronger.*

## Build
I've provided binaries for both Windows and Linux. Still, if you want to compile the sources yourself you can use g++ compiler with the following commands:

### Linux

```
g++ -std=c++11 -O3 -pthread attacks.cpp bitboards.cpp evaluation.cpp main.cpp move.cpp movegenerator.cpp pawnhashtable.cpp position.cpp search.cpp timemanagement.cpp transpositiontable.cpp uci.cpp -o MORA
```

### Windows

```
g++ -std=c++11 -O3 attacks.cpp bitboards.cpp evaluation.cpp main.cpp move.cpp movegenerator.cpp pawnhashtable.cpp position.cpp search.cpp timemanagement.cpp transpositiontable.cpp uci.cpp -o MORA
```

Notice the `-O3` flag to turn on all the optimizations of the compiler. 
## GUI

MORA CHESS ENGINE supports the UCI Protocol, so you can use any GUI that implements UCI to try the engine. 
I use [Arena Chess GUI](http://www.playwitharena.de/), which you can download for free.

In Arena you can install a new engine like this:

1. Go to **Engines/Install New Engine...**
2. Select the binary corresponding to your platform.
3. Go to **Engines/Manage...**
4. Select **UCI** where it says *type* (if it was not recognized from the beginning)
5. You should also go to **Books** in the previous menu and select the <*Use Arena mainbooks with this engine*> option (MCE doesn't implement its own opening book).

## Platforms

MORA CHESS ENGINE is available for both **Windows** and **Linux**. You can download the binaries from [here](https://github.com/gonzaloarro/MORA-CHESS-ENGINE/releases/latest).

**Note:** The sources are pretty much the same, but for the moment Windows's version doesn't implement *stop/quit* UCI commands to interrupt a search. In the future maybe I will add a cross platform thread library (like [Boost](https://www.boost.org/doc/libs/1_64_0/doc/html/thread.html)) to unify both versions.

## Technical Details

Here is a summary of the features implemented in MORA CHESS ENGINE.

### Board Representation

- Bitboards
- Redundant 8x8 Mailbox

### Move Generation

- Plain Magic Bitboards

### Search

- Iterative Deepening
- Alpha Beta
- Transposition Tables
- Check Extension
- PVS Search
- Null Move Pruning
- Late Move Reductions
- Move Ordering
  - PV Move
  - Promotions
  - Captures ordered by MVVLVA
  - Killer Moves
  - History Heuristic
- Quiescence Search
  - Delta Pruning
  - Captures and Promotions Move Generator
  - Check Evasion (without special generator)

### Evaluation

- Material
- Piece-Square Tables
- Mobility
- Piece Evaluation
- Pawn Structure
  - Isolated Pawns
  - Doubled Pawns
  - Backward Pawns
  - Passed Pawns
- King Safety
  - Pawn Shelter
  - Pawn Storms
  - Open Files next to the King
  - Enemy Queen Attacks
- Tempo bonus

### Communication Protocol

- UCI (not every command)

## Author

**Gonzalo Arró** 🇦🇷

- **Github:** [gonzaloarro](https://github.com/gonzaloarro)
- **Linkedin:** [gonzaloarro](https://www.linkedin.com/in/gonzaloarro/)
- **Email:** gonzalo.arro@gmail.com

## License

MORA CHESS ENGINE is free software: you can redistribute it and/or modify it under the terms of the **GNU General Public License** as published by the *Free Software Foundation*, either version 3 of the License, or (at your option) any later version. 

MORA CHESS ENGINE is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the [GNU General Public License](COPYING) for more details.

## Acknowledgments

- [Chess Programming Wiki](https://www.chessprogramming.org/)
- [Bluefever Software - Video Tutorial Series: Programming a Chess Engine in C](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)
- [Computer Chess Club](http://talkchess.com/forum3/index.php)
- [Computer Chess Rating Lists](https://ccrl.chessdom.com/ccrl/)
- Robert Hyatt
- Bruce Moreland
- Graham Banks

And special thanks to my project director: [Luciano H. Tamargo](http://cs.uns.edu.ar/~lt/site/), who from the very first moment embrace my idea of writing a chess engine and kept me always motivated.

