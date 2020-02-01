#ifndef _TERM_
#define _TERM_
#include "Common.h"
#include <io.h>
#include <string.h>
#include <signal.h>

class Terminal
{
    static ErrFun errFun;
    static int rows, cols;
    static int *screen;
    static Window **region;
    static Terminal *term;
    static Window *botWind;
    static Window *topWind;
    static Window *curWind;
    static char termBuf[bufSize];
    char *CopyCode(char *buf, const char *code)
    {
        strcpy(buf, code);
        return buf + strlen(code);
    }

public:
    void AlfaPen() { WriteCode(alfaCode); }
    void GraphPen() { WriteCode(graphCode); }
    void PlainPen() { WriteCode(plainCode); }
    void RevsPen() { WriteCode(revsCode); }
    void DefaultPen() { WriteCode(defCode); }
    void InitChars() { WriteCode(initCode); }
    void Clear() { WriteCode(clearCode); }
    void Bell() { WriteCode(bellCode); }
    Window *BotWind() { return botWind; }
    Window *TopWind() { return topWind; }
    Window *CurWind() { return curWind; }
    Terminal(int rows = maxRows, int col = maxCols);
    ~Terminal();
    void Refresh(Rect &rect, Window *from = 0);
    void PenPos(int row, int col);
    int GetKey();
    void Error(ErrKind err, const char *msg);
    void Message(const char *msg);
    friend class Window;
    friend class Menu;
    friend class Form;
};
#endif _TERM_