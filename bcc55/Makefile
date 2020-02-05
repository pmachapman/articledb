CC = bcc32
CFLAGS = -P -O2 -w -I$(INC) -eArticleDB.exe

SRC = ..\src
INC = ..\h

all:
		$(CC) $(CFLAGS) \
			$(SRC)\Globals.cpp \
			$(SRC)\Terminal.cpp \
			$(SRC)\Window.cpp \
			$(SRC)\Menu.cpp \
			$(SRC)\Form.cpp \
			$(SRC)\ArticleDB.cpp

clean:
		del *.obj
		del articledb.exe
		del articledb.tds
