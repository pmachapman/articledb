#include "Terminal.h"

// terminal control codes:
const char *posCode = "\033[%d;%dH";            // position cursor
#ifdef _WIN32
const char *alfaCode = "\033(B";                // normal chars
const char *graphCode = "\033(0";               // graphic chars
const char* defCode = "\033[!p\033[0m";         // default: normal+plain
const char* initCode = "\033(B\033[0m\033[?1l"; // initialize
const char* clearCode = "\033[1;1H\033[2J";     // clear screen
#else
const char* alfaCode = "\017";         // normal chars
const char* graphCode = "\016";        // graphic chars
const char* defCode = "\017\033(0m";   // default: normal+plain
const char* initCode = "\033(B\033)0"; // initialize
const char* clearCode = "\033[2J";     // clear screen
#endif
const char *plainCode = "\033[0m";     // plain video
const char *revsCode = "\033[7m";      // reverse video
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

#if _WIN32
BOOL WINAPI Interrupt(DWORD fdwCtrlType)
{
    // Get output handle
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        exit(sysErr);

    // Get input handle
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
        exit(sysErr);

    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT: // Handle the CTRL-C signal.
    case CTRL_CLOSE_EVENT: // CTRL-CLOSE: confirm that the user wants to exit.

        // Reset via ANSI
        Terminal::term->DefaultPen();
        Terminal::term->Clear();

        // Reset console output to how we received it
        if (!SetConsoleMode(hOut, Terminal::oldOutMode))
            exit(sysErr);

        // Reset console input to how we received it
        if (!SetConsoleMode(hIn, Terminal::oldInMode))
            exit(sysErr);

        // Exit
        exit(ctrlC);
        return TRUE;

        // Pass other signals to the next handler. 
    default:
        return FALSE;
    }
}
#else
void Interrupt()
{
    Terminal::term->DefaultPen();
    ioctl(0, TIOCSETP, (char*)&(Terminal::ttym));
    exit(1);
} /* Interrupt */
#endif