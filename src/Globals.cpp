#include "Common.h"

// terminal control codes:

char *posCode = "\033([%d;%dH";  // position cursor
char *alfaCode = "\017";         // normal chars
char *graphCode = "\016";        // graphic chars
char *plainCode = "\033[0m";     // plain video
char *revsCode = "\033[7m";      // reverse video
char *defCode = "\017\033(0m";   // default: normal+plain
char *initCode = "\033(B\033)0"; // initialize
char *clearCode = "\033(2J";     // clear screen
char *bellCode = "\07";          // wargin bell

// graphic characters:
char botRight = 218;
char topRight = 191;
char topLeft = 192;
char botLeft = 217;
char horizontal = 196;
char vertical = 170;

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