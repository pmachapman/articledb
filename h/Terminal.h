#ifndef _TERM_
#define _TERM_
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <sys/ioctl.h>
#endif
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif
#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN  0x0008
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT  0x0200
#endif
#else
#include <sgtty.h>
#include <signal.h>
#endif
#include "Common.h"

class Terminal
{
#ifdef _WIN32
    static DWORD oldInMode, oldOutMode;
#else
    static sgttyb ttym;
#endif
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
    void SetErr(ErrFun ef) { errFun = ef; }
    void GetErr(ErrFun* ef) { *ef = errFun; }
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
#if _WIN32
    friend BOOL WINAPI Interrupt(DWORD fdwCtrlType);
#else
    friend void Interrupt();
#endif
    friend class Window;
    friend class Menu;
    friend class Form;
};
#endif