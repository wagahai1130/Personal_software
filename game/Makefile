CC = g++
TEX = platex
CFLAGS = -Wall -std=c++17 $(COPTIONS)
LDFLAGS = 
LOADLIBES = 
CPPFLAGS = -I/home/vmware/software_g/c612301099/game/glm -I/usr/include/libqhull_r
LDLIBS = -L/usr/lib/x86_64-linux-gnu -lqhull_r -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lGL -lGLU -lSDL2 -lSDL2_gfx -lglut -lpthread

 
# 実行時にライブラリが無い，と言われる場合は以下のオプションを追加
# LDLIBS = -lm -lSDL2 -lSDL2_image -lSDL2_ttf -Wl,-rpath,/usr/local/lib
# Joy-Conライブラリを利用するときは以下のライブラリを追加
#LDLIBS = -lm -lSDL2 -ljoyconlib -lhidapi-hidraw 

TARGET = game
OBJS = $(TARGET).o system.o window.o 
HEADS = system.h

# 課題に取り組むにはこのターゲットで
#   -g3 はデバッグ情報を埋め込むオプション
#
# ターゲット指定しないと，最初のターゲットであるこれが選ばれる
#
debug: CPPFLAGS += -std=c++17 -g -O0
debug: COPTIONS = -g -O0
debug: $(TARGET)

# デバッグ情報を埋め込まないターゲット
#   -O3 は最適化オプション
#
# ゲームの個人開発などで，完成品をmakeする際などに使うとよい
#
release: CPPFLAGS += -DNDEBUG -std=c++17 
release: COPTIONS = -O3 
release: $(TARGET)

# 暗黙のルールでの動作は以下の通り
# cソースからオブジェクト
#   $(CC) $(CFLAGS) $(CPPFLAGS) -c n.c -o n.o
# オブジェクトから実行ファイル
#   $(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS) -o n
#
$(TARGET): $(OBJS)
$(OBJS): $(HEADS)

# レポート作成用ターゲット
# 例えば repo.tex があるとき，
#  make repo.pdf でpdfまで生成する
#
# texからdviへは暗黙のルール $(TEX) n.tex が使われる
#
%.pdf:%.dvi
	dvipdfmx $<

# コンパイルをやり直したい時などに，
# 中間ファイルなどを削除するターゲット
#   使用例： make clean
#
.PHONY: clean
clean:
	@rm -f $(TARGET) $(OBJS) core *~
