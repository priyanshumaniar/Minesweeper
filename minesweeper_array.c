/* Minesweeper, array version (was minesweeper_warray.c). The same game as minesweeper_code.c,
 * function for function, but the board is a plain 2D array of tiles found by index: the
 * neighbours of g[row][col] are g[row - 1][col], g[row + 1][col + 1] and so on, with a bounds
 * check instead of NULL links. No neighbour pointers are stored, so each tile is smaller and a
 * neighbour is reached by arithmetic on the indexes instead of by following a link.
 * Compile: gcc minesweeper_array.c -o minesweeper_array    Run: ./minesweeper_array
 * (Colours use the Windows console API on Windows and ANSI escape codes elsewhere.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define FLAG_SHOW 16 /* arrow symbol in the Windows console */
#else
#define FLAG_SHOW 'F'
#endif

struct ground
{
    char inground;   /* what is under the tile: '*' (42) mine, ' ' clear or '1' to '8' clue */
    char onground;   /* what covers the tile: '#' or the flag (16) */
    char *groundptr; /* points to onground while covered, to inground once dug */
} *g[11][11];

int arrayr[22], arrayc[22], dig = 0, flag = 0;

/* colour numbers are the Windows console colours listed in the plan */
void set_color(int color)
{
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
    switch (color)
    {
        case 1:  printf("\033[34m"); break; /* blue */
        case 2:  printf("\033[32m"); break; /* green */
        case 4:  printf("\033[31m"); break; /* red */
        case 5:  printf("\033[35m"); break; /* purple */
        case 6:  printf("\033[33m"); break; /* dark yellow */
        case 8:  printf("\033[90m"); break; /* grey */
        case 11: printf("\033[96m"); break; /* cyan blue */
        case 12: printf("\033[91m"); break; /* light red */
        case 13: printf("\033[95m"); break; /* light purple */
        default: printf("\033[0m"); break;  /* 7 white */
    }
#endif
}

void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

/* skips the rest of the input line (also stops at the end of input) */
void clear_line(void)
{
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        continue;
    }
}

/* allocates limit x limit tiles */
void gameload(int limit)
{
    for (int i = 0; i < limit; i++)
    {
        for (int j = 0; j < limit; j++)
        {
            g[i][j] = (struct ground *)malloc(sizeof(struct ground));
            if (g[i][j] == NULL)
            {
                printf("ALLOCATION ERROR!\n");
                exit(1);
            }
            g[i][j]->onground = '#';
            g[i][j]->inground = ' ';
            g[i][j]->groundptr = &g[i][j]->onground;
        }
    }
}

/* prints a line of dashes as wide as the board */
void print_border(int limit)
{
    set_color(1); /* blue */
    printf("\n-------");
    for (int k = 0; k < limit; ++k)
    {
        printf("----");
    }
}

/* prints the +---+---+ line between board rows */
void print_row_line(int limit)
{
    for (int k = 0; k < limit; ++k)
    {
        printf("+---");
    }
    printf("+\n");
}

/* colour of a tile depends on what is shown on it */
int tile_color(char shown)
{
    switch (shown)
    {
        case '1': return 1;  /* blue */
        case '2': return 2;  /* green */
        case '3': return 4;  /* red */
        case '4': return 5;  /* purple */
        case '5': return 13; /* magenta */
        case '6': return 6;  /* dark yellow */
        case '7': return 12; /* light red */
        case '8': return 11; /* cyan blue */
        case 42:  return 4;  /* mine: red */
        case 16:  return 4;  /* flag: red */
        default:  return 7;  /* white */
    }
}

/* displays the flag count and the game board */
void display(int limit)
{
    clear_screen();
    print_border(limit);
    set_color(12); /* light red */
    printf("\nFlags Left: %d\n", flag);
    set_color(8); /* grey */
    printf("\n   ");
    for (int k = 0; k < limit; ++k)
    {
        printf(k > 9 ? " %d " : "  %d ", k);
    }
    printf(" \n   ");
    print_row_line(limit);
    for (int i = 0; i < limit; i++)
    {
        printf(i > 9 ? "%d |" : " %d |", i);
        for (int j = 0; j < limit; j++)
        {
            char shown = *g[i][j]->groundptr;

            set_color(tile_color(shown));
            printf(" %c ", shown == 16 ? FLAG_SHOW : shown);
            set_color(8); /* grey */
            printf("|");
        }
        printf("\n   ");
        print_row_line(limit);
    }
    print_border(limit);
    printf("\n");
    set_color(7); /* white */
}

/* 1 if (row, col) is on the board and has a mine */
int subcountbomb(int row, int col, int limit)
{
    if (row != -1 && col != -1 && row < limit && col < limit)
    {
        return (g[row][col]->inground == 42) ? 1 : 0;
    }
    return 0;
}

/* returns the clue for a tile: '1' to '8' mines around it, or ' ' when there are none */
char countbomb(int row, int col, int limit)
{
    int bombcount = 48; /* 48 = '0' */

    bombcount += subcountbomb(row + 1, col, limit);
    bombcount += subcountbomb(row - 1, col, limit);
    bombcount += subcountbomb(row, col + 1, limit);
    bombcount += subcountbomb(row, col - 1, limit);
    bombcount += subcountbomb(row + 1, col + 1, limit);
    bombcount += subcountbomb(row + 1, col - 1, limit);
    bombcount += subcountbomb(row - 1, col + 1, limit);
    bombcount += subcountbomb(row - 1, col - 1, limit);

    if (bombcount == 48)
    {
        bombcount = 32; /* 32 = ' ' */
    }
    return (char)bombcount;
}

void digaround(int row, int col, int limit);

/* digs the tile at (row, col) if it is on the board, still covered and not a mine */
void subdigaround(int row, int col, int limit)
{
    if (row != -1 && col != -1 && row < limit && col < limit)
    {
        if (g[row][col]->groundptr == &g[row][col]->onground && g[row][col]->inground != 42)
        {
            if (*g[row][col]->groundptr == 16)
            {
                flag++; /* a flag on a dug tile is given back */
            }
            g[row][col]->inground = countbomb(row, col, limit);
            g[row][col]->groundptr = &g[row][col]->inground;
            dig++;
            if (g[row][col]->inground == ' ')
            {
                digaround(row, col, limit); /* recursive: keep opening clear tiles */
            }
        }
    }
}

/* digs all 8 neighbours of a tile that has no mine around it */
void digaround(int row, int col, int limit)
{
    subdigaround(row + 1, col, limit);
    subdigaround(row - 1, col, limit);
    subdigaround(row, col + 1, limit);
    subdigaround(row, col - 1, limit);
    subdigaround(row + 1, col + 1, limit);
    subdigaround(row - 1, col + 1, limit);
    subdigaround(row + 1, col - 1, limit);
    subdigaround(row - 1, col - 1, limit);
}

/* places totbomb mines on random different tiles, never on the first dug tile (row, col).
 * Called random() in the plan and description documents. */
void randombomb(int limit, int totbomb, int row, int col)
{
    srand((unsigned)time(NULL));

    for (int i = 0; i < totbomb; ++i)
    {
        arrayr[i] = rand() % limit;
        arrayc[i] = rand() % limit;
    }

check:
    for (int i = 0; i < totbomb; ++i)
    {
        if (arrayr[i] == row && arrayc[i] == col) /* the first dug tile must stay clear */
        {
            arrayr[i] = rand() % limit;
            arrayc[i] = rand() % limit;
            goto check;
        }
        for (int j = i + 1; j < totbomb; ++j)
        {
            if (arrayr[i] == arrayr[j] && arrayc[i] == arrayc[j]) /* two mines on one tile */
            {
                arrayr[i] = rand() % limit;
                arrayc[i] = rand() % limit;
                goto check;
            }
        }
    }

    for (int i = 0; i < totbomb; ++i)
    {
        g[arrayr[i]][arrayc[i]]->inground = 42;
    }
}

/* reads an integer and asks again when something else is typed */
void validate(int *data)
{
    int result;

    while ((result = scanf("%d", data)) != 1)
    {
        if (result == EOF)
        {
            printf("\nNo more input.\n");
            exit(0);
        }
        clear_line();
        printf("Invalid Input.\nPlease enter integer value only: ");
    }
    clear_line();
}

void playgame(int limit)
{
    int totbomb;

    switch (limit)
    {
        case 7:
            totbomb = 7;
            break;
        case 9:
            totbomb = 13;
            break;
        default: /* 11 */
            totbomb = 22;
            break;
    }
    flag = totbomb;

    int row, col, randomflag = 0;

    while (dig != ((limit * limit) - totbomb))
    {
    dugged:
        printf("Enter row and column to dig or flag\n");

        printf("Row: ");
        validate(&row);

        printf("Col: ");
        validate(&col);

        if (row < 0 || col < 0 || row >= limit || col >= limit)
        {
            printf("Row and Column must be between 0 and %d\n", limit - 1);
            goto dugged;
        }

        char choice;
    chooseflagdig:
        printf("Enter f to flag tile or d to dig tile: ");
        if (scanf("%c", &choice) != 1)
        {
            printf("\nNo more input.\n");
            exit(0);
        }
        if (choice != '\n')
        {
            clear_line();
        }

        if (choice == 'd')
        {
            if (randomflag == 0) /* mines are placed only after the first dig */
            {
                randombomb(limit, totbomb, row, col);
                randomflag++;
            }

            if (g[row][col]->groundptr == &g[row][col]->inground)
            {
                printf("You have already dug the tile.\nEnter other tile! ");
                goto dugged;
            }
            else if (g[row][col]->inground == 42) /* 42 = '*' mine: show all mines and stop */
            {
                for (int i = 0; i < totbomb; ++i)
                {
                    g[arrayr[i]][arrayc[i]]->groundptr = &g[arrayr[i]][arrayc[i]]->inground;
                }
                display(limit);
                break;
            }
            else
            {
                if (*g[row][col]->groundptr == 16)
                {
                    flag++;
                }
                g[row][col]->inground = countbomb(row, col, limit);
                g[row][col]->groundptr = &g[row][col]->inground;
                if (g[row][col]->inground == ' ')
                {
                    digaround(row, col, limit);
                }
                dig++;
            }
        }
        else if (choice == 'f')
        {
            if (g[row][col]->groundptr == &g[row][col]->inground)
            {
                printf("You have already dug the tile.\nEnter other tile! ");
                goto dugged;
            }
            if (g[row][col]->onground != 16) /* 16 is the arrow symbol used for a flag */
            {
                g[row][col]->onground = 16;
                flag--;
            }
            else
            {
                g[row][col]->onground = '#';
                flag++;
            }
        }
        else
        {
            printf("Invalid input. Try again\n");
            goto chooseflagdig;
        }
        display(limit);
    }
    if (dig != ((limit * limit) - totbomb))
    {
        printf("\nYou Lose. Try Next Time!\n");
    }
    else
    {
        printf("\nYou Win!!!\n");
    }
}

/* frees the board after every game and resets the counters */
void emptymemory(int limit)
{
    for (int i = 0; i < limit; i++)
    {
        for (int j = 0; j < limit; j++)
        {
            free(g[i][j]);
            g[i][j] = NULL;
        }
    }
    dig = 0;
    flag = 0;
}

int main(void)
{
    int choice, limit;

    clear_screen();
    set_color(1); /* blue */
    printf("       ******* MINESWEEPER *******\n");
    set_color(12); /* light red */
    printf("\nCONTROLS:\n\n\"The Game is purely using keyboard as a controller\"\n\nSELECT: Enter Row and Col to select a tile.\nDIG: Select and enter 'd' to dig the selected tile.\nFLAG: Select and enter 'f' to flag the selected tile.\nUNFLAG: Select a flagged tile and flag it again to unflag it.\n\nHOW TO PLAY:\n\nStart with digging any tile.\nWhen you DIG you get CLUE as a number in the tile.\nThe number shows number of mines hidden under adjacent tiles.\nIf there is no number in dug tile all tiles around it are clear.\nMark any suspected mine tile as FLAG.\nIf you think your suspected mine tile is clear, UNFLAG it.\n\nYou cannot dig or flag any tile which is already dug.\n\nIf you dig all the clear tiles without digging any mine tiles, you win.\nBut be careful not to dig any mine tile,\nit will trigger all the mines and you will lose the game.\n\nBest Of Luck!\n\n");
    set_color(7); /* white */

choose:
    printf("\n1. Easy(7x7)\t2. Medium(9x9)\t 3. Hard(11x11)\nYour choice: ");
    validate(&choice);

    switch (choice)
    {
        case 1:
            limit = 7;
            break;
        case 2:
            limit = 9;
            break;
        case 3:
            limit = 11;
            break;
        default:
            printf("Invalid choice. Enter again: ");
            goto choose;
    }

    gameload(limit);

    display(limit);

    playgame(limit);

    emptymemory(limit);

    printf("Do you want to play again?\n");
choose2:
    printf("1. Play Again\n2. Exit\nYour choice: ");
    validate(&choice);

    if (choice == 1)
    {
        goto choose;
    }
    else if (choice == 2)
    {
        return 0;
    }
    else
    {
        printf("Invalid choice. Enter again\n");
        goto choose2;
    }
}
