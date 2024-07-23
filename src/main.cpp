#include <M5Cardputer.h>
#include <M5Unified.h>

#include "utility/Keyboard.h"

// 4x4 slide puzzle game controlled using the keyboard

// array storing the current state of the puzzle
int puzzle[4][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 0}};

// array storing a solved puzzle for win detection
int solved[4][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 0}};

bool won = false; // whether the puzzle has been solved
char c;           // keyboard input

// array for controlscheme that directly moves the hole in the puzzle to a position
char controls[4][4] = {
    {'3', '4', '5', '6'},
    {'e', 'r', 't', 'y'},
    {'s', 'd', 'f', 'g'},
    {'z', 'x', 'c', 'v'}};

int zero[2] = {3, 3}; // position of the hole in the puzzle

int moveCount = 0; // number of moves made

void positionCursor(int x, int y)
{
    M5.Lcd.setCursor(x * 40, y * 40);
}

void redrawPuzzle()
{
    M5.Lcd.setTextSize(2);

    // draw the puzzle without drawing 0, and with plenty of space between numbers for readability, in black on white for contrast with a space padding single digits
    if (won)
    {
        M5.Lcd.setTextColor(TFT_BLACK, TFT_GREEN);
    }

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (puzzle[y][x] == 0)
            {
                continue;
            }
            int textColour = TFT_BLACK;
            if (won)
            {
                textColour = TFT_GOLD; //gold text when won
            }
            // create white-red checkerboard pattern by colouring 1, 3, 6, 8, 9, 11, 14 white and the rest red
            if (((puzzle[y][x] - 1) / 4 + (puzzle[y][x] - 1) % 4) % 2 == 0)
            {
                M5.Lcd.setTextColor(textColour, TFT_WHITE);
            }
            else
            {
                M5.Lcd.setTextColor(textColour, TFT_RED);
            }
            positionCursor(x, y);
            M5.Lcd.printf("%2d", puzzle[y][x]);
        }
    }

    // move cursor to zero and print a space in white on black to represent the hole
    positionCursor(zero[0], zero[1]);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.printf("  ");

    // draw the move count in the corner of the screen
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(160, 10);
    M5.Lcd.printf("Moves: %d", moveCount);
}

void swapPieces(int x, int y)
{
    // swap pieces according to direction given by x and y. x is positive for right, y is positive for down

    // first find the "gap", the 0 piece
    int gapX = zero[0];
    int gapY = zero[1];

    // add x and y to gap position to get the position of the piece to swap into the gap
    int swapX = gapX + x;
    int swapY = gapY + y;

    // if this is out of bounds, return early
    if (swapX < 0 || swapX >= 4 || swapY < 0 || swapY >= 4)
    {
        return;
    }

    // swap the pieces
    int temp = puzzle[gapY][gapX];
    puzzle[gapY][gapX] = puzzle[swapY][swapX];
    puzzle[swapY][swapX] = temp;

    // update zero
    zero[0] = swapX;
    zero[1] = swapY;

    moveCount++;

    // check if the puzzle is solved
    won = true;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (puzzle[y][x] != solved[y][x])
            {
                won = false;
                return;
            }
        }
    }
}

void swapToTarget(int x, int y)
{
    // swap until the zero piece is at the target position
    // first swap up and down, then swap left and right
    while (zero[0] != x)
    {
        if (zero[0] < x)
        {
            swapPieces(1, 0);
        }
        else
        {
            swapPieces(-1, 0);
        }
    }
    while (zero[1] != y)
    {
        if (zero[1] < y)
        {
            swapPieces(0, 1);
        }
        else
        {
            swapPieces(0, -1);
        }
    }
}

void scramble()
{
    // scramble the puzzle by making 400 random moves using swapToTarget
    for (int i = 0; i < 400; i++)
    {
        int x = random(4);
        int y = random(4);
        swapToTarget(x, y);
        redrawPuzzle();
    }
    won = false;
    moveCount = 0;
    M5.Lcd.fillScreen(TFT_BLACK);
    redrawPuzzle();
}

bool getKeyboardInput()
{
    if (M5Cardputer.Keyboard.isChange())
    {
        if (M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            for (auto i : status.word)
            {
                c = i;
            }
            switch (c)
            {
            case ';':
                swapPieces(0, -1);
                break;
            case ',':
                swapPieces(-1, 0);
                break;
            case '.':
                swapPieces(0, 1);
                break;
            case '/':
                swapPieces(1, 0);
                break;
            case '0':
                scramble();
                break;
            default:
                break;
            }
            // otherwise swap to target according to control scheme
            for (int y = 0; y < 4; y++)
            {
                for (int x = 0; x < 4; x++)
                {
                    if (controls[y][x] == c)
                    {
                        swapToTarget(x, y);
                    }
                }
            }
            return true;
        }
    }
    return false;
}

void setup()
{
    M5Cardputer.begin(true);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(2);
    redrawPuzzle();
}

void loop()
{
    M5Cardputer.update();
    if (getKeyboardInput())
    {
        // if a move was made, redraw the puzzle
        redrawPuzzle();
    }
}