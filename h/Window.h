#ifndef _WIND_
#define _WIND_
#include "Common.h"
class Window
{
    WindFlags flags;              // window flags
    char *title;                  // window title
    Rect bounds;                  // window bounds rectangle
    Point pos;                    // pen position
    Mode mode;                    // pen mode
    int *area;                    // area covered by the window
    Window *prev;                 // previous window pointer
    Window *next;                 // next window pointer
    static char windBuf[bufSize]; // window buffer
    void SetArea(int top, int left, int bot, int right);
    void Resume();
    void WriteBehind(const char *str, int row, int col, int len);

public:
    void GetBounds(Rect *rect) { *rect = bounds; }
    void GetPos(Point *pt) { *pt = pos; }
    void GetMode(Mode *pm) { *pm = mode; }
    void GetKind(WindFlags *kd) { *kd = (WindFlags)(flags & kindMask); }
    void SetKind(WindFlags kd) { flags = (WindFlags)((flags & ~kindMask) | kd); }
    Bool Hidden() { return flags & hidden; }
    Window *Previous() { return prev; }
    Window *Next() { return next; }

    Window(const char *title, int top, int left,
           int bot, int right);
    Window(const char *title, Rect &bounds);
    ~Window();
    void PenMode(Mode pm);
    void PenPos(int row, int col);
    void Move(int row, int col);
    void Resize(int top, int left, int bot, int right);
    void Activate();
    void Hide();
    void Show();
    void Clear(int from = 1, int to = maxRows);
    void GetLine(char *str, int row);
    void WriteStr(const char *str, int len);
    void ReadStr(char *str, int len);
    Window &operator<<(const char *);
    Window &operator<<(char);
    Window &operator<<(long);
    Window &operator<<(double);
    Window &operator>>(char *);
    Window &operator>>(char &);
    Window &operator>>(long &);
    Window &operator>>(double &);
    friend class Terminal;
};
#endif _WIND_