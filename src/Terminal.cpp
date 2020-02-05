#include "Terminal.h"
#include "Window.h"

ErrFun Terminal::errFun;
int Terminal::rows;
int Terminal::cols;
int *Terminal::screen;
Window **Terminal::region;
Terminal *Terminal::term;
Window *Terminal::botWind;
Window *Terminal::topWind;
Window *Terminal::curWind;
char Terminal::termBuf[bufSize];
#ifdef _WIN32
DWORD Terminal::oldInMode;
DWORD Terminal::oldOutMode;
#else
sgttyb Terminal::ttym;
#endif

Terminal::Terminal(int rows, int cols)
{
    typedef void *VoidPtr;
    if (term != 0)
        Error(termErr, "in Terminal"); // make sure only called once
    term = this; // self-reference
    Terminal::rows = rows = (rows <= 0 || rows > maxRows ? maxRows : rows);
    Terminal::cols = cols = (cols <= 0 || cols > maxCols ? maxCols : cols);
    if ((screen = new int[rows * cols]) == 0)
        Error(memErr, "for Screen");
    if ((region = (Window **)new VoidPtr[rows * cols]) == 0)
        Error(memErr, "for Region");

    // region should be initially blank
    for (register int row = 0; row < rows; ++row)
    {
        Window **rgn = region + cols * row;
        for (register int col = 0; col < cols; ++col)
            *(rgn + col) = 0;
    }

#ifdef _WIN32
    // Set up console output
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        Error(sysErr, "for GetStdHandle(Output)");
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        Error(sysErr, "for GetConsoleMode(Output)");
    Terminal::oldOutMode = dwMode;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    dwMode |= DISABLE_NEWLINE_AUTO_RETURN;
    if (!SetConsoleMode(hOut, dwMode))
        Error(sysErr, "for SetConsoleMode(Output)");

    // Set up console input
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
        Error(sysErr, "for GetStdHandle(Input)");
    if (!GetConsoleMode(hIn, &dwMode))
        Error(sysErr, "for GetConsoleMode(Input)");
    Terminal::oldInMode = dwMode;
    dwMode = ENABLE_VIRTUAL_TERMINAL_INPUT;
    if (!SetConsoleMode(hIn, dwMode))
        Error(sysErr, "for SetConsoleMode(Input)");

    // Register the Ctrl+C handler
    // This only works if ENABLE_PROCESSED_INPUT is enabled, which is not
    // Instead, the handler is called by our own code managing Ctrl+C
    if (!SetConsoleCtrlHandler(Interrupt, TRUE))
        Error(sysErr, "for SetConsoleCtrlHandler");

    // Fix the "two enters required" bug, but break Unicode:
    if (!setmode(_fileno(stdin), _O_BINARY))
        Error(sysErr, "for setmode");
#else
    if (ioctl(0, TIOCSETP, (char*)&ttym) == -1) // restore original mode
        Error(sysErr, "for ioctl");
    sgttyb tmp = ttym; // make a copy
    tmp.sg_flags |= CBREAK; // use cbreak mode
    tmp.sg_flags &= ~ECHO; // use no echo mode
#ifndef __APPLE__
    if (signal(SIGINT, &Interrupt) == -1)
        Error(sysErr, "for signal");
#endif
#endif
    Clear();
    InitChars();
}

Terminal::~Terminal()
{
    delete screen;
    delete region;
#ifndef _WIN32
    if (ioctl(0, TIOCGETP, (char *)&ttym) == -1) // get original mode
        Error(sysErr, "for ioctl");
#endif
    DefaultPen();
}

void Terminal::Refresh(Rect &rect, Window *from)
{
    register int row, col = 0;
    int width;
    if (rect.Empty())
        return;
    if (from == 0)
        from = botWind;
    width = rect.right - rect.left + 1;

    // make rect and its region blank
    for (row = rect.top; row <= rect.bot; ++row)
    {
        int *scr = screen + row * col + rect.left;
        Window **rgn = region + row * col + rect.left;
        for (col = 0; col < width; ++col)
        {
            *scr++ = ' ';
            *rgn++ = 0;
        }
    }

    // update the part of the screen and region denoted by rect
    // for each visible window in the list denoted by from:
    for (Window *wind = from; wind != 0; wind = wind->next)
        if (!wind->Hidden())
        {
            Rect box = rect * wind->bounds;
            if (!box.Empty())
            {
                int boxWidth = box.right - box.left + 1;
                int windWidth = wind->bounds.right - wind->bounds.left + 1;
                int leftDiff = box.left - wind->bounds.left;
                for (row = box.top; row <= box.bot; ++row)
                {
                    int *scr = wind->area + (row - wind->bounds.top) * (windWidth + leftDiff);
                    int *dest = screen + row * cols + box.left;
                    Window **rgn = region + row * cols + box.left;
                    for (col = 0; col < boxWidth; ++col)
                    {
                        *dest++ = *scr++;
                        *rgn++ = wind;
                    }
                }
            }
        }

    DefaultPen();
    Mode mode, oldMode = defPen;

    // use screen to refresh the area denoted by rect:
    for (row = rect.top; row <= rect.bot; ++row)
    {
        char *buf = termBuf;
        int *scr = screen + row * cols + rect.left;
        PosCode(buf, row + 1, rect.left + 1);
        buf += strlen(buf);
        for (col = 0; col < width; ++col)
        {
            mode = (Mode)(*scr & modeMask);
            if (mode != oldMode)
            {
                if (oldMode & graphPen)
                {
                    if (!(mode & graphPen))
                        buf = CopyCode(buf, alfaCode);
                }
                else if (mode & graphPen)
                    buf = CopyCode(buf, graphCode);
                if (oldMode & revsPen)
                {
                    if (!(mode & revsPen))
                        buf = CopyCode(buf, plainCode);
                }
                else if (mode & revsPen)
                    buf = CopyCode(buf, revsCode);
                oldMode = mode;
            }
            *buf++ = (char)(*scr++ & charMask);
        }
        write(1, termBuf, buf - termBuf);
    }
    DefaultPen();
}

void Terminal::PenPos(int row, int col)
{
    row = (row < 0 ? 0 : (row >= rows ? rows - 1 : row));
    col = (col < 0 ? 0 : (col >= cols ? cols - 1 : col));
    PosCode(termBuf, row + 1, col + 1);
    write(1, termBuf, strlen(termBuf));
}

int Terminal::GetKey()
{
    char ch;
    for (;;)
    {
        read(0, &ch, 1);
        if (ch == escape)
        {
            read(0, &ch, 1);
            switch (ch)
            {
            case escape:
                return escCmd;
            case '[':
                read(0, &ch, 1);
                switch (ch)
                {
                case 'A':
                    return upCmd;
                case 'B':
                    return downCmd;
                case 'C':
                    return rightCmd;
                case 'D':
                    return leftCmd;
                }
            }
            Bell();
            continue;
        }
        return (int)ch;
    }
}

void Terminal::Error(ErrKind err, const char *msg)
{
    if (errFun != 0)
        (*errFun)(err, msg);
    else
    {
        sprintf(termBuf, "Error:%d %s \n", err, msg);
        write(1, termBuf, strlen(termBuf));
        Interrupt();
    }
}

void Terminal::Message(const char *msg)
{
    Point pt;
    int lastRow = rows;
    curWind->GetPos(&pt);
    PenPos(lastRow, 0);
    RevsPen();
    strncpy(termBuf, msg, cols);
    register int len = strlen(msg);
    if (len > cols)
    {
        termBuf[len = cols] = '\0';
        while (len >= cols - 3)
            termBuf[--len] = '.';
    }
    else
        while (len < cols)
            termBuf[len++] = ' ';
    write(1, termBuf, len);
    curWind->Resume();
}
