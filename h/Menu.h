#ifndef _MENU_
#define _MENU_
#include "Common.h"
#include "Window.h"

typedef int (*MenuAct)(Menu &, int);

class Menu : Window
{
    int nOptions;   // number of options
    int curOptn;    // current option
    MenuAct action; // menu action routine
    void HiliteOption(int nOptn, Bool hilite);

public:
    Window::GetBounds;
    Window::Hidden;
    Window::Show;
    Window::Hide;
    Window::Activate;
    Window::Move;
    int Options() { return nOptions; }
    int CurOptn() { return curOptn; }
    void SetAct(MenuAct act) { action = act; }
    void GetAct(MenuAct *act) { *act = action; }

    Menu(const char *title, int row, int col,
         MenuAct act, const char *optn...);

    int Select(int start = 0, MenuAct escFun = 0);
};

#endif _MENU_