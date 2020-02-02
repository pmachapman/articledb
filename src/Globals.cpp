#include "Common.h"

// terminal control codes:

const char *posCode = "\033([%d;%dH";  // position cursor
const char *alfaCode = "\017";         // normal chars
const char *graphCode = "\016";        // graphic chars
const char *plainCode = "\033[0m";     // plain video
const char *revsCode = "\033[7m";      // reverse video
const char *defCode = "\017\033(0m";   // default: normal+plain
const char *initCode = "\033(B\033)0"; // initialize
const char *clearCode = "\033(2J";     // clear screen
const char *bellCode = "\07";          // wargin bell

// graphic characters:
char botRight = '\152';
char topRight = '\153';
char topLeft = '\154';
char botLeft = '\155';
char horizontal = '\161';
char vertical = '\170';

Rect::Rect(int top, int left, int bot, int right)
{
    Rect::top = top;
    Rect::left = left,
    Rect::bot = bot;
    Rect::right = right;
} /* Rect */

void Rect::Offset(int rows, int cols)
{
    top += rows;
    left += cols;
    bot += rows;
    right += cols;
} /* Offset */

Rect Rect::operator*(Rect &rect)
{
    return Rect(top > rect.top ? top : rect.top,
                left > rect.left ? left : rect.left,
                bot < rect.bot ? bot : rect.bot,
                right < rect.right ? right : rect.right);
} /* operator * */

Rect Rect::operator+(Rect &rect)
{
    return Rect(top < rect.top ? top : rect.top,
                left < rect.left ? left : rect.left,
                bot > rect.bot ? bot : rect.bot,
                right > rect.right ? right : rect.right);
} /* operator + */