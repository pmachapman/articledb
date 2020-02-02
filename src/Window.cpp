#include "Terminal.h"
#include "Window.h"

char Window::windBuf[bufSize];

Window::Window(const char *title, int top, int left, int bot, int right)
{
    if (Terminal::term == 0)
        Terminal::term->Error(termErr, "in Window");
    if (*title)
    {
        Window::title = new char[strlen(title) + 1];
        if (Window::title == 0)
            Terminal::term->Error(memErr, "for title");
        strcpy(Window::title, title);
    }
    else
        Window::title = 0;
    // insert in window list:
    prev = next = 0;
    if (Terminal::topWind == 0)
    {
        Terminal::botWind = this;
        Terminal::topWind = this;
    }
    else
    {
        Terminal::topWind->next = this;
        this->prev = Terminal::topWind;
        Terminal::topWind = this;
    }
    flags = hidden;
    SetArea(top, left, bot, right);
    PenPos(1, 1);
    PenMode(defPen);
} /* Window */

inline Window::Window(const char *title, Rect &bounds)
{
    Window(title, bounds.top, bounds.left, bounds.bot, bounds.right);
} /* Window */

void Window::SetArea(int top, int left, int bot, int right)
{
    top = (top < 0 ? 0 : (top >= Terminal::rows ? Terminal::rows - 1 : top));
    left = (left < 0 ? 0 : (left >= Terminal::cols ? Terminal::cols - 1 : left));
    bot = (bot < top ? top : (bot >= Terminal::rows ? Terminal::rows - 1 : bot));
    right = (right < left ? left : (right >= Terminal::cols ? Terminal::cols - 1 : right));
    bounds = Rect(top, left, bot, right);
    int height = bot - top + 1;
    int width = right - left + 1;
    register int row, col;
    if ((area = new int[height * width]) == 0)
        Terminal::term->Error(memErr, "for area");
    register int *line = area;
    if (height >= 2 && width >= 2)
    {
        *line++ = topLeft | graphPen; // draw upper border of window
        for (col = 1; col < width - 1; ++col)
            *line++ = horizontal | graphPen;
        *line++ = topRight | graphPen;
        for (row = 1; row < height - 1; ++row)
        { // draw middle of window
            *line++ = vertical | graphPen;
            for (col = 1; col < width - 1; ++col)
                *line++ = ' ';
            *line++ = vertical | graphPen;
        }
        *line++ = botLeft | graphPen; // draw lower border of window
        for (col = 1; col < width - 1; ++col)
            *line++ = horizontal | graphPen;
        *line++ = botRight | graphPen;
    }
    if (title != 0)
    { // draw the title
        const char *name = title;
        line = area + 1;
        for (col = 1; col < width - 1l && *name; ++col)
            *line++ = *name++ | revsPen;
    }
} /* SetArea */

Window::~Window()
{
    if (!Hidden())
        Hide();
    if (this == Terminal::botWind)
        Terminal::botWind = this->next;
    if (this == Terminal::topWind)
        Terminal::topWind = this->prev;
    if (this->next != 0)
        this->next->prev = this->prev;
    if (this->prev != 0)
        this->prev->next = this->next;
    delete area;
    delete title;
} /* ~Window */

void Window::PenPos(int row, int col)
{
    pos.row = (row <= 0 ? 1 : (row >= bounds.bot - bounds.top ? bounds.bot - bounds.top - 1 : row));
    pos.col = (col <= 0 ? 1 : (col >= bounds.right - bounds.left ? bounds.right - bounds.left - 1 : col));
    if (!Hidden())
    {
        PosCode(Terminal::termBuf,
                bounds.top + pos.row + 1, bounds.left + pos.col + 1);
        write(1, Terminal::termBuf, strlen(Terminal::termBuf));
    }
} /* PenPos */

void Window::PenMode(Mode pm)
{
    if (!Hidden())
    {
        if (pm & graphPen)
            Terminal::term->GraphPen();
        else
            Terminal::term->AlfaPen();
        if (pm & revsPen)
            Terminal::term->RevsPen();
        else
            Terminal::term->PlainPen();
    }
    mode = pm;
} /* PenMode */

void Window::Move(int row, int col)
{
    int height = bounds.bot - bounds.top + 1;
    int width = bounds.right - bounds.left + 1;
    Rect oldBounds = bounds;
    row = (row < 0 ? 0 : (row + height > Terminal::rows ? Terminal::rows - height : row));
    col = (col < 0 ? 0 : (col + width > Terminal::cols ? Terminal::cols - width : col));
    if (row == bounds.top && col == bounds.left)
        return;
    row -= bounds.top;
    col -= bounds.left;
    bounds.Offset(row, col);
    if (!Hidden())
    {
        Terminal::term->Refresh(oldBounds);
        Terminal::term->Refresh(bounds, this);
        Terminal::curWind->Resume();
    }
} /* Move */

void Window::Resize(int top, int left, int bot, int right)
{
    Bool visible = !Hidden();
    if (visible)
        Hide();
    delete area;
    SetArea(top, left, bot, right);
    PenPos(1, 1);
    PenMode(defPen);
    if (visible)
        Show();
} /* Resize */

void Window::Activate()
{
    if (this == Terminal::curWind && !Hidden())
        return;
    Rect refBox; // refresh box
    if (Hidden())
    {
        refBox = bounds;
        flags = (WindFlags)(flags & ~hidden);
    }
    else
    {
        Rect rect = bounds, box;
        // refBox is initially empty:
        refBox = Rect(rect.bot, rect.right, rect.top, rect.left);
        // work out the overlapped area:
        for (Window *wind = this->next; wind != 0; wind = wind->next)
            if (!wind->Hidden() && !(box = rect * wind->bounds).Empty())
                refBox = box + refBox;
    }
    if (this != Terminal::topWind)
    {
        if (this->prev != 0)
            this->prev->next = this->next;
        else
            Terminal::botWind = this->next;
        this->next->prev = this->prev;
        this->next = 0;
        this->prev = Terminal::topWind;
        Terminal::topWind->next = this;
        Terminal::topWind = this;
    }
    Terminal::curWind = this;
    Terminal::term->Refresh(refBox, this);
    this->Resume();
} /* Activate */

void Window::Hide()
{
    if (Hidden())
        return;
    flags = (WindFlags)(flags | hidden);
    Terminal::term->Refresh(bounds);
    if (this == Terminal::curWind)
        this->prev->Resume();
} /* Hide */

void Window::Show()
{
    if (!Hidden())
        return;
    flags = (WindFlags)(flags & ~hidden);
    Terminal::term->Refresh(bounds, this);
    if (Terminal::curWind == 0)
        Resume();
    else // resume this if above curWind:
        for (Window *wind = Terminal::curWind; wind != 0; wind = wind->next)
            if (wind == this)
                Resume();
} /* Show */

void Window::Resume()
{
    Window *wind = this;
    while (wind != 0 && wind->Hidden())
        wind = wind->prev;
    if (wind == 0)
    {
        int lastRow = Terminal::rows;
        Terminal::term->PenPos(lastRow, 1);
    }
    else
    {
        PenPos(pos.row, pos.col);
        PenMode(mode);
    }
    Terminal::curWind = wind;
} /* Resume */

void Window::Clear(int from, int to)
{
    int height = bounds.bot - bounds.top - 1;
    int width = bounds.right - bounds.left - 1;
    char *buf = Terminal::termBuf;
    if (from > to || from > height || to < 1)
        return;
    from = (from < 1 ? 1 : from);
    to = (to > height ? height : to);
    int col;
    for (col = 0; col < width; ++col)
        buf[col] = ' ';
    buf[col] = '\0';
    for (int row = from; row <= to; ++row)
    {
        PenPos(bounds.top + row, 1);
        WriteStr(buf, width);
    }
} /* Clear */

void Window::GetLine(register char *str, int row)
{
    register int width = bounds.right - bounds.left + 1;
    register int *line = area + row * width + 1;
    width -= 2;
    while (width-- > 0)
        *str++ = (char)*line++;
    *str = '\0';
} /* GetLine */

void Window::WriteStr(const char *string, int len)
{
    Bool visible = !Hidden();
    Bool current = this == Terminal::curWind;
    int width = bounds.right - bounds.left + 1;
    int room = width - pos.col - 1;
    int col = pos.col;
    register const char *str = string;
    if (!current && visible)
    {
        PenPos(pos.row, pos.col);
        PenMode(mode);
    }
    for (register int i = 0; i < len && *str; ++i)
    {
        if (*str == '\n' || i >= room)
        {
            if (current)
                write(1, string, str - string);
            else if (visible)
                WriteBehind(string, pos.row, col, str - string);
            if (*str == '\n')
                ++str;
            else
                --i;
            string = str;
            if (pos.row >= bounds.bot - bounds.top - 1)
                PenPos(1, col = 1);
            else
                PenPos(pos.row + 1, col = 1);
            room += width - 2;
        }
        else
            *(area + pos.row * width + pos.col++) = *str++ | mode;
    }
    if (current)
        write(1, string, str - string);
    else if (visible)
        WriteBehind(string, pos.row, col, str - string);
    if (!current && visible)
        Terminal::curWind->Resume();
} /* WriteStr */

void Window::WriteBehind(register const char *str, int row, int col, int len)
{
    register Window **rgn = Terminal::region +
                            (bounds.top + row) * Terminal::cols + bounds.left + col;
    register char *buf = Terminal::termBuf;
    int behind = 0;
    for (register int i = 0; i < len; ++i)
    {
        if (*rgn++ == this)
        {
            if (behind > 0)
            {
                PosCode(buf, bounds.top + row + 1, bounds.left + col + 1 + i);
                buf += strlen(buf);
                behind = 0;
            }
            *buf++ = *str++;
        }
        else
            ++behind;
    }
    write(1, Terminal::termBuf, buf - Terminal::termBuf);
} /* WriteBehind */

void Window::ReadStr(char *string, int len)
{
    int width = bounds.right - bounds.left + 1;
    int room = width - pos.col - 1;
    int n = 0;
    Activate();
    int *line = area + pos.row * width + pos.col;
    room = (room < len ? room : len);
    for (;;)
    {
        int key = Terminal::term->GetKey();
        char ch;
        if (n < room && isprint(key))
        {
            *line++ = (string[n++] = key) | mode;
            write(0, &(ch = key), 1);
            ++pos.col;
        }
        else if (key == '\r' || key == '\n')
        {
            break;
        }
        else if (key == '\b' && n > 0)
        {
            write(1, "\b", 1);
            write(1, &(ch = ' '), 1);
            *--line = (string[--n] = ' ') | mode;
            PenPos(pos.row, pos.col);
        }
        else
            Terminal::term->Bell();
    }
    string[n] = '\0';
} /* ReadStr */

Window &Window::operator<<(const char *str)
{
    WriteStr(str, strlen(str));
    return *this;
} /* operator << */

Window &Window::operator<<(char ch)
{
    windBuf[0] = ch;
    windBuf[1] = '\0';
    WriteStr(windBuf, 1);
    return *this;
} /* Operator << */

Window &Window::operator<<(long num)

{
    sprintf(windBuf, "%d", num);
    WriteStr(windBuf, strlen(windBuf));
    return *this;

} /* operator << */

Window &Window::operator<<(double num)

{
    sprintf(windBuf, "%f", num);
    WriteStr(windBuf, strlen(windBuf));
    return *this;

} /* operator << */

Window &Window::operator>>(char *str)

{
    ReadStr(str, bounds.right - pos.col - 1);
    return *this;

} /* operator >> */

Window &Window::operator>>(char &ch)

{
    ReadStr(windBuf, bounds.right - pos.col - 1);
    ch = windBuf[0];
    return *this;

} /* operator >> */

Window &Window::operator>>(long &num)

{
    ReadStr(windBuf, bounds.right - pos.col - 1);
    num = atol(windBuf);
    return *this;

} /* operator >> */

Window &Window::operator>>(double &num)

{
    ReadStr(windBuf, bounds.right - pos.col - 1);
    num = atof(windBuf);
    return *this;

} /* operator >> */