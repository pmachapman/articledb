#include "Terminal.h"

// terminal control codes:

const char *posCode = "\033[%d;%dH";  // position cursor
#ifdef _WIN32
const char *alfaCode = "\033(B";         // normal chars
const char *graphCode = "\033(0";        // graphic chars
const char* defCode = "\033(B\033[0m";   // default: normal+plain
const char* initCode = "\033(B\033[0m\033[?1h";         // initialize
#else
const char* alfaCode = "\017";         // normal chars
const char* graphCode = "\016";        // graphic chars
const char* defCode = "\017\033(0m";   // default: normal+plain
const char* initCode = "\033(B\033)0"; // initialize
#endif
const char *plainCode = "\033[0m";     // plain video
const char *revsCode = "\033[7m";      // reverse video
const char *clearCode = "\033[2J";     // clear screen
const char *bellCode = "\07";          // margin bell

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

void Interrupt()
{
    Terminal::term->DefaultPen();
#ifndef _WIN32
    ioctl(0, TIOCSETP, (char*)&(Terminal::ttym));
#endif
    exit(1);
} /* Interrupt */