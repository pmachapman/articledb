#include "Terminal.h"
#include "Window.h"
#include "Menu.h"
#include <stdarg.h>

Menu::Menu(const char *title, int top, int left, MenuAct act, const char *optn...)
    : Window(title, top, left, top, left)
{
    const char *optns[maxRows];
    register int rows = 0, cols = 0, n;
    action = act;
    SetKind(menuWind);
    va_list arg;
    va_start(arg, optn);
    while (rows < Terminal::rows - 2)
    {
        optns[rows++] = optn;
        if ((n = strlen(optn)) > cols)
            cols = n;
        if ((optn = va_arg(arg, char *)) == 0)
            break;
    }
    va_end(arg);
    nOptions = rows;
    if (++cols + 2 > Terminal::cols)
        cols = Terminal::cols - 2;
    Resize(top, left, top + rows + 1, left + cols + 1);
    for (n = 1; n <= nOptions; ++n)
    { // write the options
        PenPos(n, 2);
        WriteStr(optns[n - 1], cols - 1);
    }
    PenPos(1, 1);
    HiliteOption(curOptn = 1, true);
} /* Menu */

void Menu::HiliteOption(int nOptn, Bool hilite)
{
    char optn[maxCols];
    Rect bounds;
    GetBounds(&bounds);
    GetLine(optn, nOptn);
    PenPos(nOptn, 1);
    PenMode(hilite ? revsPen : defPen);
    WriteStr(optn, bounds.right - bounds.left - 1);
    PenPos(nOptn, 1);
} /* HiliteOption */

int Menu::Select(int start, MenuAct escFun)
{
    if (start != 0 && start != curOptn)
    {
        HiliteOption(curOptn, false);
        HiliteOption(curOptn = start, true);
    }
    Activate();
    for (;;)
    {
        int optn = 0;
        int n;
        switch (Terminal::term->GetKey())
        {
        case upCmd:
            optn = (curOptn == 1 ? 0 : curOptn - 1);
            break;
        case downCmd:
            optn = (curOptn == nOptions ? 0 : curOptn + 1);
            break;
        case '\n':
        case '\r':
            n = curOptn;
            if (action == 0 || (n = (*action)(*this, n)) != 0)
                return n;
            continue;
        case escCmd:
            n = curOptn;
            if (escFun != 0 && (n = (*escFun)(*this, n)) != 0)
                return n;
            continue;
        } /* switch */
        if (optn == 0)
            Terminal::term->Bell();
        else
        {
            HiliteOption(curOptn, false);
            HiliteOption(curOptn = optn, true);
        }
    } /* for */
} /* Select */