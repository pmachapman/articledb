#include "Terminal.h"
#include "Form.h"
#include <stdarg.h>

Form::Form(const char *title, int top, int left, FormAct act, const char *line...)
    : Window(title, top, left, top, left)
{
    Field flds[maxFlds];
    Field lits[maxLits];
    int totLen = 0;
    int rows = 0, cols = 0;
    register int n = 0, m = 0, i;
    action = act;
    SetKind(formWind);

    va_list arg;
    va_start(arg, line);
    while (n < maxFlds && m < maxLits)
    {
        register const char *str = line;
        while (*str)
        {
            if (*str == '_' && n < maxFlds)
            {
                flds[n].pos.row = rows + 1;
                flds[n].pos.col = str - line + 1;
                flds[n].len = 0;
                for (flds[n].len = 1; *++str == '_'; ++flds[n].len)
                    ;
                totLen += flds[n++].len + 1;
            }
            else if (m < maxLits)
            {
                lits[m].data = (char *)str;
                while (*++str != '\0' && *str != '_')
                    ;
                lits[m].pos.row = rows + 1;
                lits[m].pos.col = lits[m].data - line + 1;
                lits[m].len = str - lits[m].data;
                ++m;
            }
            else
                ++str;
        }
        ++rows;
        cols = (str - line > cols ? str - line : cols);
        if ((line = va_arg(arg, char *)) == 0)
            break;
    }
    va_end(arg);
    Resize(top, left, top + rows + 1, left + cols + 1);
    for (i = 0; i < m; ++i)
    {
        PenPos(lits[i].pos.row, lits[i].pos.col);
        WriteStr(lits[i].data, lits[i].len);
    }
    char *data;
    if ((data = new char[totLen + 1]) == 0)
        Terminal::term->Error(memErr, "in Form");
    for (i = 0; i < totLen; ++i)
        data[i] = ' ';
    data[totLen] = '\0';
    if ((fields = new Field[nFields = n]) == 0)
        Terminal::term->Error(memErr, "in Form");
    for (i = 0; i < n; ++i)
    {
        flds[i].data = data;
        data += flds[i].len + 1;
        *(data - 1) = '\t';
        fields[i] = flds[i];
    }
    PenPos(fields[0].pos.row, fields[0].pos.col);
}

/* Form */
Form::~Form()
{
    delete fields[0].data;
    delete fields;
} /* Form */

void Form::HiliteField(int nFld, Bool hilite)
{
    if (nFld < 1 || nFld > nFields)
        return;
    Field &fld = fields[nFld - 1];
    PenPos(fld.pos.row, fld.pos.col);
    PenMode(hilite ? revsPen : defPen);
    WriteStr(fld.data, fld.len);
    PenPos(fld.pos.row, fld.pos.col);
} /* HiliteField */

void Form::Drain(int nFld)
{
    if (nFld < 1 || nFld > nFields)
        return;
    char *data = fields[nFld - 1].data;
    int len = fields[nFld - 1].len;
    for (int i = 0; i < len; ++i)
        *data++ = ' ';
} /* Drain */

void Form::Read(int nFld, FormAct escFun)
{
    Point pos;
    Mode mode;
    GetPos(&pos);
    GetMode(&mode);
    Activate();
    Bool readAll = nFld == 0;
    nFld = (nFld <= 0 ? 1 : (nFld > nFields ? nFields : nFld));
    Field *fld = &fields[nFld - 1];
    PenPos(fld->pos.row, fld->pos.col);
    int key, next, idx = -1;
    char ch;
    for (;;)
    {
        switch (key = Terminal::term->GetKey())
        {
        case upCmd:
            next = (readAll && nFld > 1 ? nFld - 1 : 0);
            break;
        case '\n':
        case '\r':
            if (idx >= 0)
                HiliteField(nFld, false);
            idx = -1;
            if (action != 0 && (*action)(*this, nFld) != 0 || !readAll)
                goto out;
        case '\t':
        case downCmd:
            next = (readAll && nFld < nFields ? nFld + 1 : 0);
            break;
        case leftCmd:
            if (idx <= 0)
                Terminal::term->Bell();
            else
                PenPos(fld->pos.row, fld->pos.col + --idx);
            continue;
        case rightCmd:
            if (idx < 0)
            {
                HiliteField(nFld, true);
                idx = 0;
            }
            if (idx >= fld->len)
                Terminal::term->Bell();
            else
                PenPos(fld->pos.row, fld->pos.col + ++idx);
            continue;
        case '\b':
            if (idx < 0)
            {
                for (int i = 0; i < fld->len; ++i)
                    fld->data[i] = ' ';
                HiliteField(nFld, false);
            }
            else if (idx == 0)
            {
                Terminal::term->Bell();
            }
            else
            {
                for (int i = idx; i < fld->len; ++i)
                    fld->data[i - 1] = fld->data[i];
                fld->data[fld->len - 1] = ' ';
                --idx;
                write(1, "\b", 1);
                write(1, fld->data + idx, fld->len - idx);
                PenPos(fld->pos.row, fld->pos.col + idx);
            }
            continue;
        case escCmd:
            if (idx >= 0)
                HiliteField(nFld, false);
            if (escFun == 0 || (*escFun)(*this, nFld) != 0)
                goto out;
            continue;
        default:
            if (!isprint(key) || idx >= fld->len)
            {
                Terminal::term->Bell();
                continue;
            }
            if (++idx == 0)
            {
                if (action != 0 && (*action)(*this, -nFld) != 0)
                    continue;
                HiliteField(nFld, true);
                ++idx;
            }
            write(1, &(ch = (char)key), 1);
            *(fld->data + idx - 1) = ch;
            continue;
        } /* switch */
        if (next == 0)
            Terminal::term->Bell();
        else
        {
            if (idx >= 0)
                HiliteField(nFld, false);
            fld = &fields[(nFld = next) - 1];
            PenPos(fld->pos.row, fld->pos.col);
            idx = -1;
        }
    } /* for */
out:
    PenPos(pos.row, pos.col);
    PenMode(mode);
} /* Read */

void Form::Update(register int nFld)
{
    Point pos;
    Mode mode;
    GetPos(&pos);
    GetMode(&mode);
    if (nFld <= 0)
        for (nFld = 1; nFld <= nFields; ++nFld)
            HiliteField(nFld, false);
    else if (nFld <= nFields)
        HiliteField(nFld, false);
    PenPos(pos.row, pos.col);
    PenMode(mode);
} /* Update */

void Form::Blank(register int nFld)
{
    Point pos;
    Mode mode;
    GetPos(&pos);
    GetMode(&mode);
    if (nFld <= 0)
    {
        for (nFld = 1; nFld <= nFields; ++nFld)
        {
            Drain(nFld);
            HiliteField(nFld, false);
        }
    }
    else if (nFld <= nFields)
    {
        Drain(nFld);
        HiliteField(nFld, false);
    }
    PenPos(pos.row, pos.col);
    PenMode(mode);
} /* Blank */

void Form::SetValue(int nFld, register const char *val, Bool update)
{
    if (nFld >= 1 && nFld <= nFields)
    {
        register char *data = fields[nFld - 1].data;
        while (*val && *val != '\t' && *data != '\t')
            *data++ = *val++;
        while (*data != '\t')
            *data++ = ' ';
        if (update)
            HiliteField(nFld, false);
    }
} /* SetValue */

void Form::SetValue(int nFld, long val, Bool update)
{
    char data[maxCols];
    sprintf(data, "%d", val);
    SetValue(nFld, data, update);
} /* SetValue */

void Form::SetValue(int nFld, double val, Bool update)
{
    char data[maxCols];
    sprintf(data, "%f", val);
    SetValue(nFld, data, update);
} /* SetValue */

void Form::SetAll(register const char *vals, Bool update)
{
    for (register int nFld = 0; nFld < nFields; ++nFld)
    {
        register char *data = fields[nFld].data;
        while (*vals && *vals != '\t' && *data != '\t')
            *data++ = *vals++;
        while (*vals && *vals != '\t')
            ++vals;
        if (*vals == '\t')
            ++vals;
        while (*data != '\t')
            *data = ' ';
    }
    if (update)
        Update();
} /* SetAll */

void Form::GetValue(int nFld, register char *val, Bool truncate)
{
    if (nFld < 1 || nFld > nFields)
    {
        *val = '\0';
        return;
    }
    register const char *data = fields[nFld - 1].data;
    while (*data != '\t')
        *val++ = *data++;
    if (truncate)
    {
        while (*--val == ' ')
            ;
        ++val;
    }
    *val = '\0';
} /* GetValue */

void Form::GetValue(int nFld, long *val, Bool truncate)
{
    char data[maxCols];
    GetValue(nFld, data, truncate);
    *val = atol(data);
} /* GetValue */

void Form::GetValue(int nFld, double *val, Bool truncate)
{
    char data[maxCols];
    GetValue(nFld, data, truncate);
    *val = atof(data);
} /* GetValue */

void Form::GetAll(register char *vals, Bool truncate)
{
    for (register int nFld = 0; nFld < nFields; ++nFld)
    {
        register const char *data = fields[nFld].data;
        while (*data != '\t')
            *vals++ = *data++;
        if (truncate)
        {
            while (*--vals == ' ')
                ;
            ++vals;
        }
        *vals++ = '\t';
    }
    *vals = '\0';
} /* GetAll */