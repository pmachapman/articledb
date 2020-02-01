#include <stdio.h>
#include "Terminal.h"
#include "Window.h"
#include "Menu.h"
#include "Form.h"

struct Article
{
    Article *next; // pointer to the next article in the list
    char image[1]; // article image (allocated dynamically)
};

class ArticleDB
{
    static const int maxSelect = 20; // max no. of selected articles
    char *fileName;                  // DB file name
    Article *articles;               // linked-list of articles
    Article *select[maxSelect];      // selected articles
    int nSelect;                     // no. of selected articles
    int curSel;                      // currently selected article
    static char artBuf[bufSize];     // article buffer
    Bool update;                     // update flag
    Bool Match(const char *pattern, const char *image);

public:
    int Selected() { return nSelect; }
    int CurSelect() { return curSel; }
    void ZeroSelect() { nSelect = curSel = 0; }
    ArticleDB(const char *fName);
    ~ArticleDB();
    Bool Insert(const char *image);
    Bool Delete();
    int Search(const char *pattern);
    char *PrevImage();
    char *CurImage();
    char *NextImage();
    void SetImage(const char *image);
};

char ArticleDB::artBuf[bufSize];

ArticleDB::ArticleDB(const char *fName)
{
    fileName = new char[strlen(fName) + 1];
    strcpy(fileName, fName);
    articles = 0;
    nSelect = curSel = 0;
    FILE *file = fopen(fName, "r");
    if (file != 0)
    {
        while (fgets(artBuf, bufSize, file) != 0)
        {
            int len = strlen(artBuf);
            artBuf[--len] = '\0'; // replace \n by \0O
            Article *art = (Article *)new char[sizeof(Article) + len];
            art->next = articles;
            strcpy(art->image, artBuf);
            articles = art;
        }
        fclose(file);
    }
} /* ArticleDB */

ArticleDB::~ArticleDB()
{
    if (update)
    {
        FILE *file = fopen(fileName, "w");
        if (file != 0)
        {
            for (Article *art = articles; art != 0; art = art->next)
                if (fputs(art->image, file) <= 0 || fputs("\n", file) < 0)
                    break;
            fclose(file);
        }
    }
    delete fileName;
    while (articles != 0)
    {
        Article *temp = articles;
        articles = articles->next;
        delete temp;
    }
} /* ArticleDB */

Bool ArticleDB::Insert(const char *image)
{
    Article *art;
    for (art = articles; art != 0; art = art->next)
        if (strcmp(image, art->image) == 0)
            return true;
    art = (Article *)new char[sizeof(Article) + strlen(image)];
    strcpy(art->image, image);
    art->next = articles;
    articles = art;
    return false;
} /* Insert */

Bool ArticleDB::Delete()
{
    if (curSel == 0)
        return true;
    Article *article = select[curSel - 1];
    if (article == articles)
    {
        articles = articles->next;
        delete article;
    }
    else
    {
        for (Article *art = articles; art->next != 0; art = art->next)
            if (article == art->next)
            {
                art->next = art->next->next;
                delete article;
                goto out;
            }
        return true;
    }
out:
    for (int n = 0; n < nSelect; ++n)
        if (select[n] == article)
        {
            --nSelect;
            for (; n < nSelect; ++n)
                select[n] = select[n + 1];
            break;
        }
    return false;
} /* Delete */

int ArticleDB::Search(const char *pattern)
{
    nSelect = 0;
    for (Article *art = articles; art != 0 && nSelect < maxSelect; art = art->next)
        if (Match(pattern, art->image))
            select[nSelect++] = art;
    curSel = (nSelect > 0 ? 1 : 0);
    return nSelect;
} /* Search */

Bool ArticleDB::Match(const char *pattern, const char *image)
{
    while (*pattern)
    {
        Bool match = false;
        int len = 0;
        for (const char *str = pattern; *str != '\t'; ++str)
            ++len;
        while (*image != '\t')
            if (strncmp(pattern, image++, len) == 0)
            {
                match = true;
                break;
            }
        if (!match)
            return false;
        while (*image++ != '\t')
            ;
        while (*pattern++ != '\t')
            ;
    }
    return true;
} /* Match */

char *ArticleDB::PrevImage()
{
    return (curSel <= 1 || nSelect == 0 ? 0 : select[--curSel - 1]->image);
}

char *ArticleDB::CurImage()
{
    return (nSelect == 0 ? 0 : select[curSel - 1]->image);
}

char *ArticleDB::NextImage()
{
    return (curSel < nSelect ? select[curSel++]->image : 0);
}

void ArticleDB::SetImage(const char *image)
{
    if (curSel > 0)
        strcpy(select[curSel - 1]->image, image);
}

void Display(Terminal &term, Form &article, const char *image, int idx, int total)
{
    if (image != 0)
    {
        article.SetAll(image);
        char titleBuf[50];
        sprintf(titleBuf, "%d of %d", idx, total);
        term.Message(titleBuf);
    }
    else
        term.Bell();
} /* Display */

enum MenuCmds
{
    none,
    newTemplateCmd,
    editTemplateCmd,
    searchCmd,
    previousCmd,
    nextCmd,
    saveArticleCmd,
    deleteArticleCmd,
    quitCmd
};

void main()
{
    Terminal term;
    ArticleDB db("articleFile");
    char buffer[bufSize];
    Menu commands("", 0, 0, 0,
                  "New Template", "Edit Template", "Search", "Previous Image",
                  "Next Image", "Save Article", "Delete Article", "Quit", 0);
    Form article(
        "Article", 5, 20, 0,
        "AUTHOR:     ___________________ ENTERED: __________",
        "TITLE:      _______________________________________",
        "PERIODICAL: _______________________________________",
        "VOLUME:     ___   NUMBER: ___   YEAR:  ____",
        "PUBLISHER:  ___________________ PLACE: ____________",
        0);
    article.Show();
    Bool quit = false;
    while (!quit)
    {
        switch (commands.Select())
        {
        case newTemplateCmd:
            db.ZeroSelect();
            article.Blank();
            term.Message("");
            article.Read();
            break;
        case editTemplateCmd:
            article.Read();
            break;
        case searchCmd:

            if (db.Selected() == 0)
            {
                article.GetAll(buffer, true);
                db.Search(buffer);

                Display(term, article, db.CurImage(),
                        db.CurSelect(), db.Selected());
            }
            break;
        case previousCmd:
            Display(term, article, db.PrevImage(),
                    db.CurSelect(), db.Selected());
            break;
        case nextCmd:
            Display(term, article, db.NextImage(),
                    db.CurSelect(), db.Selected());
            break;
        case saveArticleCmd:

            article.GetAll(buffer, false);

            if (db.CurSelect() != 0)
            {
                db.SetImage(buffer);
                term.Message("Substituted");
            }
            else if (db.Insert(buffer))
            {
                term.Message("Duplicate!");
                term.Bell();
            }
            else
                term.Message("Saved");

            break;

        case deleteArticleCmd:

            if (db.Delete())
            {
                term.Message("No selection!");
                term.Bell();
            }
            else
            {
                char *image;
                if ((image = db.CurImage()) != 0 ||

                    (image = db.PrevImage()) != 0)
                    Display(term, article, image,
                            db.CurSelect(), db.Selected());
                else

                    article.Blank();
            }

            break;

        case quitCmd:
            quit = true;
        }
    } /* while */
} /* main */