/* Minesweeper, version 1 (first working draft, was minesweeper_old.c). Design: minesweeper_plan_1.
 * The board is a grid of nodes, each linked to its 8 neighbours. The first dig is handled by
 * firstdig(): it keeps the dug tile and the tiles around it free of mines (excluderand()), then
 * playgame() runs the rest of the game. countbomb() and digaround() still check the 8 directions
 * with 8 repeated if-blocks, and there is no validate() yet; version 2 moves these into
 * subcountbomb(), subdigaround() and validate(). Hard level has 21 mines here (22 later).
 * Compile: gcc v1_minesweeper_old.c -o minesweeper_v1    Run: ./minesweeper_v1
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
    struct ground *up;
    struct ground *down;
    struct ground *left;
    struct ground *right;
    struct ground *upright;
    struct ground *upleft;
    struct ground *downright;
    struct ground *downleft;
    char inground;   /* what is under the tile: '*' (42) mine, ' ' clear or '1' to '8' clue */
    char onground;   /* what covers the tile: '#' or the flag (16) */
    char *groundptr; /* points to onground while covered, to inground once dug */
} *g[12][12];

int arrayr[30], arrayc[30], dig = 0, flag = 0;

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

/* skips the rest of the input line; ends the program when the input has ended */
void clear_line(void)
{
    int ch;

    while ((ch = getchar()) != '\n')
    {
        if (ch == EOF)
        {
            printf("\nNo more input.\n");
            exit(0);
        }
    }
}

/* places totbomb mines on random different tiles, but never on the 9 excluded tiles
 * (the first dug tile and its neighbours). Called random() in the plan. */
int randombomb(int limit, int totbomb, int *arrayexr, int *arrayexc)
{
    time_t t;

    srand((unsigned)time(&t));

    for (int i = 0; i < totbomb; ++i)
    {
        arrayr[i] = rand() % limit;
        arrayc[i] = rand() % limit;
    }

check:
    for (int i = 0; i < totbomb; ++i)
    {
        for (int j = i + 1; j < totbomb; ++j)
        {
            if (arrayr[i] == arrayr[j] && arrayc[i] == arrayc[j]) /* two mines on one tile */
            {
                arrayr[j] = rand() % limit;
                arrayc[j] = rand() % limit;
                goto check;
            }
        }
    }
    for (int i = 0; i < totbomb; ++i)
    {
        for (int j = 0; j < 9; ++j)
        {
            if (arrayr[i] == arrayexr[j] && arrayc[i] == arrayexc[j]) /* mine on an excluded tile */
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

    return totbomb;
}

/* allocates limit x limit tiles and links every tile to its neighbours */
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
    for (int i = 0; i < limit; i++)
    {
        for (int j = 0; j < limit; j++)
        {
            /* a neighbour outside the board is NULL */
            if (i == 0)
            {
                g[i][j]->up = NULL;
            }
            else
            {
                g[i][j]->up = g[i - 1][j];
            }

            if (i == limit - 1)
            {
                g[i][j]->down = NULL;
            }
            else
            {
                g[i][j]->down = g[i + 1][j];
            }

            if (j == limit - 1)
            {
                g[i][j]->right = NULL;
            }
            else
            {
                g[i][j]->right = g[i][j + 1];
            }

            if (j == 0)
            {
                g[i][j]->left = NULL;
            }
            else
            {
                g[i][j]->left = g[i][j - 1];
            }

            if (i == 0 || j == limit - 1)
            {
                g[i][j]->upright = NULL;
            }
            else
            {
                g[i][j]->upright = g[i - 1][j + 1];
            }

            if (i == 0 || j == 0)
            {
                g[i][j]->upleft = NULL;
            }
            else
            {
                g[i][j]->upleft = g[i - 1][j - 1];
            }

            if (i == limit - 1 || j == limit - 1)
            {
                g[i][j]->downright = NULL;
            }
            else
            {
                g[i][j]->downright = g[i + 1][j + 1];
            }

            if (i == limit - 1 || j == 0)
            {
                g[i][j]->downleft = NULL;
            }
            else
            {
                g[i][j]->downleft = g[i + 1][j - 1];
            }
        }
    }
}

/* displays the flag count and the game board */
void display(int limit)
{
    clear_screen();
    set_color(1); /* blue */
    printf("\n-------");
    for (int i = 0; i < limit; ++i)
    {
        printf("----");
    }
    set_color(12); /* light red */
    printf("\nFlags Left: %d\n", flag);
    set_color(8); /* grey */
    printf("\n   ");
    for (int i = 0; i < limit; ++i)
    {
        if (i > 9)
        {
            printf(" %d ", i);
        }
        else
        {
            printf("  %d ", i);
        }
    }
    printf(" \n   ");
    for (int i = 0; i < limit; ++i)
    {
        printf("+---");
    }
    printf("+\n");
    for (int i = 0; i < limit; i++)
    {
        if (i > 9)
        {
            printf("%d |", i);
        }
        else
        {
            printf(" %d |", i);
        }
        for (int j = 0; j < limit; j++)
        {
            switch (*g[i][j]->groundptr)
            {
                case '1':
                    set_color(1); /* blue */
                    break;
                case '2':
                    set_color(2); /* green */
                    break;
                case '3':
                    set_color(4); /* red */
                    break;
                case '4':
                    set_color(5); /* purple */
                    break;
                case '5':
                    set_color(13); /* magenta */
                    break;
                case '6':
                    set_color(6); /* dark yellow */
                    break;
                case '7':
                    set_color(12); /* light red */
                    break;
                case '8':
                    set_color(11); /* cyan blue */
                    break;
                case 42:
                    set_color(4); /* red */
                    break;
                case 16:
                    set_color(4); /* red */
                    break;
                default:
                    set_color(7); /* white */
                    break;
            }
            printf(" %c ", *g[i][j]->groundptr == 16 ? FLAG_SHOW : *g[i][j]->groundptr);
            set_color(8); /* grey */
            printf("|");
        }
        printf("\n   ");
        for (int k = 0; k < limit; ++k)
        {
            printf("+---");
        }
        printf("+\n");
    }
    set_color(1); /* blue */
    printf("\n-------");
    for (int i = 0; i < limit; ++i)
    {
        printf("----");
    }
    printf("\n");
    set_color(7); /* white */
}

/* returns the clue for a tile: '1' to '8' mines around it, or ' ' when there are none */
char countbomb(struct ground *g)
{
    int bombcount = 48; /* 48 = '0' */

    if (g->up != NULL)
    {
        if (g->up->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->down != NULL)
    {
        if (g->down->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->right != NULL)
    {
        if (g->right->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->left != NULL)
    {
        if (g->left->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->upright != NULL)
    {
        if (g->upright->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->upleft != NULL)
    {
        if (g->upleft->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->downright != NULL)
    {
        if (g->downright->inground == 42)
        {
            bombcount++;
        }
    }
    if (g->downleft != NULL)
    {
        if (g->downleft->inground == 42)
        {
            bombcount++;
        }
    }

    if (bombcount == 48)
    {
        bombcount = 32; /* 32 = ' ' */
    }
    return (char)bombcount;
}

/* digs every covered, mine-free neighbour of a tile that has no mine around it;
 * recursive, so a whole empty area opens at once */
void digaround(struct ground *g)
{
    if (g->up != NULL)
    {
        if (g->up->groundptr != &g->up->inground && g->up->inground != 42)
        {
            if (*g->up->groundptr == 16)
            {
                flag++; /* a flag on a dug tile is given back */
            }
            g->up->inground = countbomb(g->up);
            g->up->groundptr = &g->up->inground;
            dig++;
            if (g->up->inground == ' ')
            {
                digaround(g->up);
            }
        }
    }

    if (g->down != NULL)
    {
        if (g->down->groundptr != &g->down->inground && g->down->inground != 42)
        {
            if (*g->down->groundptr == 16)
            {
                flag++;
            }
            g->down->inground = countbomb(g->down);
            g->down->groundptr = &g->down->inground;
            dig++;
            if (g->down->inground == ' ')
            {
                digaround(g->down);
            }
        }
    }

    if (g->right != NULL)
    {
        if (g->right->groundptr != &g->right->inground && g->right->inground != 42)
        {
            if (*g->right->groundptr == 16)
            {
                flag++;
            }
            g->right->inground = countbomb(g->right);
            g->right->groundptr = &g->right->inground;
            dig++;
            if (g->right->inground == ' ')
            {
                digaround(g->right);
            }
        }
    }

    if (g->left != NULL)
    {
        if (g->left->groundptr != &g->left->inground && g->left->inground != 42)
        {
            if (*g->left->groundptr == 16)
            {
                flag++;
            }
            g->left->inground = countbomb(g->left);
            g->left->groundptr = &g->left->inground;
            dig++;
            if (g->left->inground == ' ')
            {
                digaround(g->left);
            }
        }
    }

    if (g->upright != NULL)
    {
        if (g->upright->groundptr != &g->upright->inground && g->upright->inground != 42)
        {
            if (*g->upright->groundptr == 16)
            {
                flag++;
            }
            g->upright->inground = countbomb(g->upright);
            g->upright->groundptr = &g->upright->inground;
            dig++;
            if (g->upright->inground == ' ')
            {
                digaround(g->upright);
            }
        }
    }

    if (g->upleft != NULL)
    {
        if (g->upleft->groundptr != &g->upleft->inground && g->upleft->inground != 42)
        {
            if (*g->upleft->groundptr == 16)
            {
                flag++;
            }
            g->upleft->inground = countbomb(g->upleft);
            g->upleft->groundptr = &g->upleft->inground;
            dig++;
            if (g->upleft->inground == ' ')
            {
                digaround(g->upleft);
            }
        }
    }

    if (g->downright != NULL)
    {
        if (g->downright->groundptr != &g->downright->inground && g->downright->inground != 42)
        {
            if (*g->downright->groundptr == 16)
            {
                flag++;
            }
            g->downright->inground = countbomb(g->downright);
            g->downright->groundptr = &g->downright->inground;
            dig++;
            if (g->downright->inground == ' ')
            {
                digaround(g->downright);
            }
        }
    }

    if (g->downleft != NULL)
    {
        if (g->downleft->groundptr != &g->downleft->inground && g->downleft->inground != 42)
        {
            if (*g->downleft->groundptr == 16)
            {
                flag++;
            }
            g->downleft->inground = countbomb(g->downleft);
            g->downleft->groundptr = &g->downleft->inground;
            dig++;
            if (g->downleft->inground == ' ')
            {
                digaround(g->downleft);
            }
        }
    }
}

/* lists the first dug tile and its neighbours (9 tiles) so randombomb() keeps them free of mines;
 * a missing neighbour keeps the dug tile's own position */
int excluderand(int limit, int totbomb, struct ground *g, int row, int col)
{
    int arrayexr[] = {row, row, row, row, row, row, row, row, row};
    int arrayexc[] = {col, col, col, col, col, col, col, col, col};

    if (g->up != NULL)
    {
        arrayexr[1] = row - 1;
        arrayexc[1] = col;
    }

    if (g->down != NULL)
    {
        arrayexr[2] = row + 1;
        arrayexc[2] = col;
    }

    if (g->right != NULL)
    {
        arrayexr[3] = row;
        arrayexc[3] = col + 1;
    }

    if (g->left != NULL)
    {
        arrayexr[4] = row;
        arrayexc[4] = col - 1;
    }

    if (g->upright != NULL)
    {
        arrayexr[5] = row - 1;
        arrayexc[5] = col + 1;
    }

    if (g->upleft != NULL)
    {
        arrayexr[6] = row - 1;
        arrayexc[6] = col - 1;
    }

    if (g->downright != NULL)
    {
        arrayexr[7] = row + 1;
        arrayexc[7] = col + 1;
    }

    if (g->downleft != NULL)
    {
        arrayexr[8] = row + 1;
        arrayexc[8] = col - 1;
    }

    return randombomb(limit, totbomb, arrayexr, arrayexc);
}

/* first move of a game: flags are allowed, and the first dig places the mines around it */
int firstdig(int limit)
{
    int totbomb;

    switch (limit)
    {
        case 7:
            totbomb = 7;
            flag = 7;
            break;
        case 9:
            totbomb = 13;
            flag = 13;
            break;
        default: /* 11 */
            totbomb = 21;
            flag = 21;
            break;
    }

    display(limit);

    int row, col;

dugged:
    printf("Enter row and column to dig or flag\n");

    printf("Row: ");
    while (scanf("%d", &row) != 1)
    {
        clear_line();
        printf("Invalid Input.\nPlease enter integer value for Row: ");
    }
    clear_line();

    printf("Col: ");
    while (scanf("%d", &col) != 1)
    {
        clear_line();
        printf("Invalid Input.\nPlease enter integer value for Col: ");
    }
    clear_line();

    if (row < 0 || col < 0 || row >= limit || col >= limit)
    {
        printf("Row and Column must be between 0 and %d\n", limit - 1);
        goto dugged;
    }

    char choice = '\n';
chooseflagdig:
    printf("Enter f to flag tile or d to dig tile: ");
    if (scanf("%c", &choice) != 1)
    {
        clear_line(); /* end of input: clear_line() ends the program */
    }
    if (choice != '\n')
    {
        clear_line();
    }

    if (choice == 'd')
    {
        if (g[row][col]->onground == 16)
        {
            flag++; /* the flag on the dug tile is given back */
        }
        g[row][col]->groundptr = &g[row][col]->inground;

        excluderand(limit, totbomb, g[row][col], row, col);

        digaround(g[row][col]);

        dig++;
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
        goto dugged;
    }
    else
    {
        printf("Invalid input. Try again\n");
        goto chooseflagdig;
    }

    display(limit);

    return totbomb;
}

void playgame(int limit, int totbomb)
{
    int row, col;

    while (dig != ((limit * limit) - totbomb))
    {
    dugged:
        printf("Enter row and column to dig or flag\n");

        printf("Row: ");
        while (scanf("%d", &row) != 1)
        {
            clear_line();
            printf("Invalid Input.\nPlease enter integer value for Row: ");
        }
        clear_line();

        printf("Col: ");
        while (scanf("%d", &col) != 1)
        {
            clear_line();
            printf("Invalid Input.\nPlease enter integer value for Col: ");
        }
        clear_line();

        if (row < 0 || col < 0 || row >= limit || col >= limit)
        {
            printf("Row and Column must be between 0 and %d\n", limit - 1);
            goto dugged;
        }

        char choice = '\n';
    chooseflagdig:
        printf("Enter f to flag tile or d to dig tile: ");
        if (scanf("%c", &choice) != 1)
        {
            clear_line(); /* end of input: clear_line() ends the program */
        }
        if (choice != '\n')
        {
            clear_line();
        }

        if (choice == 'd')
        {
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
                g[row][col]->inground = countbomb(g[row][col]);
                g[row][col]->groundptr = &g[row][col]->inground;
                if (g[row][col]->inground == ' ')
                {
                    digaround(g[row][col]);
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

/* frees the board after every game and resets the counters for the next game */
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
    clear_screen();
    int choice, limit;
    set_color(1); /* blue */
    printf("       ******* MINESWEEPER *******\n");
    set_color(12); /* light red */
    printf("\nCONTROLS:\n\n\"The Game is purely using keyboard as a controller\"\n\nSELECT: Enter Row and Col to select a tile.\nDIG: Select and enter 'd' to dig the selected tile.\nFLAG: Select and enter 'f' to flag the selected tile.\nUNFLAG: Select a flagged tile and flag it again to unflag it.\n\nHOW TO PLAY:\n\nStart with digging any tile.\nWhen you DIG you get CLUE as a number in the tile.\nThe number shows number of mines hidden under adjacent tiles.\nIf there is no number in dug tile all tiles around it are clear.\nMark any suspected mine tile as FLAG.\nIf you think your suspected mine tile is clear, UNFLAG it.\n\nYou cannot dig or flag any tile which is already dug.\n\nIf you dig all the clear tiles without digging any mine tiles, you win.\nBut be careful not to dig any mine tile,\nit will trigger all the mines and you will lose the game.\n\nBest Of Luck!\n\n");
    set_color(7); /* white */
choose:
    printf("\n1. Easy(7x7)\t2. Medium(9x9)\t 3. Hard(11x11)\nYour choice: ");
    while (scanf("%d", &choice) != 1)
    {
        clear_line();
        printf("Invalid Input.\nPlease enter integer value: ");
    }
    clear_line();

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

    int totbomb = firstdig(limit);

    playgame(limit, totbomb);

    emptymemory(limit);

    printf("Do you want to play again?\n");
choose2:
    printf("1. Play Again\n2. Exit\nYour choice: ");
    while (scanf("%d", &choice) != 1)
    {
        clear_line();
        printf("Invalid Input.\nPlease enter integer value for Choice: ");
    }
    clear_line();

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
