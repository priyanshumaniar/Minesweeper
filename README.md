# Minesweeper in C

A console Minesweeper game I wrote in C for my B.Tech Data Structures (DSA) course project at GLS University, together with every version of the code from the first draft to the final game, and a second implementation on a plain 2D array.

## What Minesweeper is

Minesweeper is a one-player puzzle. The board is a grid of covered tiles, and some of them hide mines. You uncover ("dig") tiles one at a time. A dug tile that is not a mine shows a number: how many of the 8 tiles touching it hide a mine. Using these numbers you work out where the mines are. You win when every tile without a mine has been dug. You lose if you dig a mine.

## How to play

1. Start the program (see "Build and run" below). The controls and rules are shown first.
2. Choose a level: type `1` for Easy (7 x 7 board, 7 mines), `2` for Medium (9 x 9, 13 mines) or `3` for Hard (11 x 11, 22 mines), then press Enter.
3. Every move has two steps. First pick a tile: type its row number and press Enter, then its column number and press Enter. Rows and columns are counted from 0, as numbered along the edges of the board.
4. Then type `d` to dig that tile or `f` to put a flag on it (or take the flag off again), and press Enter.

On the board:

| Symbol | Meaning |
| --- | --- |
| `#` | a covered tile |
| `1` to `8` | a dug tile: this many mines are in the 8 tiles around it |
| blank | a dug tile with no mine around it |
| `F` | a flag you placed (an arrow symbol in the Windows console) |
| `*` | a mine (shown when you lose) |

Things worth knowing:

- Your first dig is always safe. The mines are placed only after it, and never on that tile.
- When you dig a tile with no mine around it, the game digs all the tiles around it for you, and keeps going through every empty tile it finds, so a whole clear area opens at once.
- A flag is only a reminder. It does not stop you from digging that tile, and you do not need to flag the mines to win.
- "Flags Left" above the board shows the number of mines minus the flags you have placed (the very first screen, before the game starts, shows 0).
- When the game ends you can play again (`1`) or exit (`2`).

The same rules are in [`Howtoplay`](Howtoplay) and, with a description of the code, in `minesweeper_description.pdf`.

## Files

| File | What it is |
| --- | --- |
| `minesweeper_code.c` | **The final game. Start here.** The board is a grid of linked tiles. |
| `minesweeper_array.c` | The same game on a plain 2D array (alternative implementation). |
| `versions/v1_minesweeper_old.c` | First working draft. |
| `versions/v2_minesweeper_1.c` | Second draft: repeated code moved into small functions. |
| `versions/validation_prototype.c` | A small test program for the input check used by the game. |
| `Howtoplay` | Controls and rules as plain text. |
| `minesweeper_plan_1`, `minesweeper_plan_2` | Design notes: plan 1 goes with the two drafts, plan 2 with the final game. |
| `minesweeper_description.docx` / `.pdf` | Project description: rules, levels and every function explained. |

## Build and run

Any C compiler works. With gcc:

```
gcc -Wall -Wextra -std=c11 minesweeper_code.c -o minesweeper
./minesweeper
```

The other versions build the same way:

```
gcc -Wall -Wextra -std=c11 minesweeper_array.c -o minesweeper_array
gcc -Wall -Wextra -std=c11 versions/v1_minesweeper_old.c -o minesweeper_v1
gcc -Wall -Wextra -std=c11 versions/v2_minesweeper_1.c -o minesweeper_v2
gcc -Wall -Wextra -std=c11 versions/validation_prototype.c -o validation
```

On Windows the game uses the Windows console for colours and clearing the screen. On Linux and macOS it uses standard terminal (ANSI) codes instead, and the flag is shown as `F`.

## Two ways to build the board

The game logic is the same in both implementations. What differs is how a tile finds the 8 tiles around it, which the game needs all the time: to count the mines around a tile, and to open the tiles around an empty one.

### Linked-neighbour board (`minesweeper_code.c`)

Every tile is its own small record (a node) that holds what is under it, what covers it, and 8 links (pointers) to its neighbours: up, down, left, right and the four diagonals. The links are set once, when the board is created. A tile at the edge simply has no link on that side.

```
  upleft    up    upright
        \   |   /
  left  - [tile] -  right
        /   |   \
downleft   down  downright
```

After that, the game never works out positions again. To count the mines around a tile it follows its 8 links; to open an empty area it follows links from tile to tile. The board behaves like a small network (a graph) where every tile knows its own neighbours, and "being at the edge" is just a missing link, so the counting and opening code needs no boundary checks.

In this design the rules only talk to the links, so a board of a different shape (hexagonal tiles, or edges that wrap around) would only need a different linking step when the board is built.

### 2D array board (`minesweeper_array.c`)

The board is a plain grid of tiles `g[row][col]`, like a table. A neighbour is found by arithmetic on the position: the tile above `g[row][col]` is `g[row - 1][col]`, the one to the lower right is `g[row + 1][col + 1]`, and so on. Before using a neighbour, the code checks that the position is still on the board.

This version is lighter and faster: there is no linking step when the board is built, and each tile does not store 8 pointers (a tile takes 16 bytes instead of 80 on a 64-bit machine). Array indexing is also the usual way grid games are written.

Comparing the two files side by side (`diff minesweeper_code.c minesweeper_array.c`) shows only the board handling; everything else is the same.

## How the code developed

The drafts are kept to show how the design changed. Each file starts with a comment that explains its differences.

1. **Version 1** (`versions/v1_minesweeper_old.c`, design in `minesweeper_plan_1`). The linked board is already there. The first move is handled by a separate `firstdig()` function: it keeps the dug tile and all its neighbours free of mines (`excluderand()`), so the first dig always opens an area. Counting mines and opening tiles are written out as 8 repeated blocks, one per direction.
2. **Version 2** (`versions/v2_minesweeper_1.c`). The 8 repeated blocks become one helper called 8 times (`subcountbomb()` and `subdigaround()`), and reading a number with an input check becomes one function, `validate()`. The Hard level goes from 21 to 22 mines.
3. **Final** (`minesweeper_code.c`, design in `minesweeper_plan_2`). `firstdig()` and `excluderand()` are dropped. The mines are placed inside the normal game loop on the first dig, and only the dug tile itself is kept clear. One loop now handles every move, and the dig and flag counters are reset between games.
4. **Array version** (`minesweeper_array.c`). The final game rewritten function for function on a 2D array, to compare the two designs.

`versions/validation_prototype.c` is a small test program for the input check: it reads a number, asks again if something else is typed, and prints the number as a character. The game stores its symbols the same way, as character codes (42 is `*`, and 16 is the arrow used as the flag in the Windows console).

## Fixes made when the code was prepared for publishing

All versions now build without warnings with gcc and clang (`-Wall -Wextra -std=c11`) and were tested by playing scripted games. Bugs that were fixed: a row or column outside the board crashed the game (in the drafts, only a negative one); in the final and array versions the last mine could land on the first dug tile; in the drafts the tiles kept free around the first dig were worked out with the wrong row/column offsets, so at the edge of the board a mine could sit next to the first tile while that tile showed no number; the drafts did not reset the dig counter between games, so a second game could end in an early win; some flags were not given back when their tile was dug; pressing Enter alone at the dig/flag prompt skipped the next input; and the input loops never ended if the input stopped.
