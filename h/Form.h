#ifndef _FORM_
#define _FORM_
#include "Window.h"

typedef int (*FormAct)(Form &, int);

struct Field
{
    char *data; // data for the field (its value)
    Point pos;  // position of the field
    int len;    // field length
};

class Form : public Window
{
    Field *fields;  // form fields
    int nFields;    // number of fields
    int totLen;     // total length of all fields
    FormAct action; // form action routine
    void HiliteField(int nFld, Bool hilite);
    void Drain(int nFld);

public:
    int Fields() { return nFields; }
    int TotalLen() { return totLen; }
    int FieldLen(int nFld) { return fields[nFld - 1].len; }
    void SetAct(FormAct act) { action = act; }
    void GetAct(FormAct *act) { *act = action; }

    Form(const char *title, int top, int left,
         FormAct act, const char *name...);
    ~Form();

    void Read(int nFld = 0, FormAct escFun = 0);
    void Update(int nFld = 0);
    void Blank(int nFld = 0);
    void SetValue(int nFld, const char *val, Bool update = true);
    void SetValue(int nFld, long val, Bool update = true);
    void SetValue(int nFld, double val, Bool update = true);
    void SetAll(const char *vals, Bool update = true);
    void GetValue(int nFld, char *val, Bool truncate = false);
    void GetValue(int nFld, long *val, Bool truncate = false);
    void GetValue(int nFld, double *val, Bool truncate = false);
    void GetAll(char *vals, Bool truncate = false);
};

#endif _FORM_