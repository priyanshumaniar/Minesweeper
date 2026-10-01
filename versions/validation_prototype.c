/* Prototype of the integer input check (was validation.c), not part of the game.
 * It holds the same loop as validate() in version 2 and the final game: read an int, and on a
 * letter or other text skip the line and ask again. It then prints the number as a character
 * (the game stores its symbols as character codes too: 42 is '*', the mine, and 16 is the arrow
 * used as the flag, which shows only in the Windows console).
 * Compile: gcc validation_prototype.c -o validation    Run: ./validation
 */

#include <stdio.h>

int main(void)
{
    int input, ch;

    printf("Please enter number:");

    while (scanf("%d", &input) != 1)
    {
        while ((ch = getchar()) != '\n')
        {
            if (ch == EOF)
            {
                printf("\nNo more input.\n");
                return 0;
            }
        }
        printf("Invalid Input... please enter number:");
    }
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        continue;
    }
    printf("Your number is %c\n", (char)input);
    return 0;
}
