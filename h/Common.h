#ifndef _COMMON_
#define _COMMON_
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#define PosCode(buf, r, c) sprintf(buf, posCode, r, c)
#define WriteCode(code) write(1, code, strlen(code))
#define maxRows 23    // max no. of rows on the screen
#define maxCols 80    // max no. of columns on the screen
#define maxFlds 64    // max no. of fields in a form
#define maxLits 64    // max no. of literals in a form
#define bufSize 512   // buffer size
#define escape '\033' // escape character
extern const char *posCode, *alfaCode, *graphCode, *plainCode, *revsCode,
    *defCode, *initCode, *clearCode, *bellCode;
extern char botRight, topRight, topLeft, botLeft, horizontal, vertical;
#define Bool bool
enum Command
{                 // keyboard commands
    escCmd = -1,  // escape command
    upCmd = -2,   // up arrow
    downCmd = -3, // down arrow
    leftCmd = -4, // left arrow
    rightCmd = -5 // right arrow
};

enum Mode
{                      // pen mode
    defPen = 0x0000,   // default pen
    graphPen = 0x0100, // graphic pen
    revsPen = 0x0200,  // reverse video pen
    charMask = 0x00FF, // character mask
    modeMask = 0xFF00
};
enum WindFlags
{                       // window flags
    plainWind = 0x0000, // plain window
    menuWind = 0x0001,  // menu window
    formWind = 0x0002,  // form window
    userl = 0x0004,     // user-defined window
    user2 = 0x0008,
    user3 = 0x0010,
    user4 = 0x0020,
    hidden = 0x0100,  // invisible window
    kindMask = 0x00FF // window kind mask
};
enum ErrKind
{ // error kind
    memErr,
    termErr,
    sysErr // memory, terminal, system error
};
struct Point
{
    int row, col;
    Point() { row = col = 0; }
    Point(int r, int c)
    {
        row = r;
        col = c;
    }
    void Offset(int r, int c)
    {
        row += r;
        col += c;
    }
};
struct Rect
{ // rectangle
    int top, left, bot, right;
    Rect() { top = left = bot = right = 0; }
    Rect(int top, int left, int bot, int right);
    void Offset(int rows, int cols);
    Bool Empty() { return left > right || top > bot; }
    Rect operator*(Rect &rect);
    Rect operator+(Rect &rect);
};
typedef void (*ErrFun)(int, const char *);
typedef void (*InterruptFun)(void);
class Terminal;
class Window;
class Menu;
class Form;
#endif _COMMON_