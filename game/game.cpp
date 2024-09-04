/*
 * ファイル名： game.cpp
 * 機能　　　： メインルーチン
 */
#include "system.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <filesystem>
#include <GL/glut.h>

// GLUTのキーボードコールバック関数
void keyboardFunc(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // エスケープキーのASCIIコードは27
            exit(0);
            break;
        case 'w':
            gGame.key.key_w = SDL_TRUE;
            break;
        case 'a':
            gGame.key.key_a = SDL_TRUE;
            break;
        case 's':
            gGame.key.key_s = SDL_TRUE;
            break;
        case 'd':
            gGame.key.key_d = SDL_TRUE;
            break;
        case ' ':
            gGame.key.key_space = SDL_TRUE;
            break;
        case 16: // Shiftキーのコードは16 (ただし通常は特殊キー扱い)
            gGame.key.key_shift = SDL_TRUE;
            break;
        case 17: // Ctrlキーのコードは17 (ただし通常は特殊キー扱い)
            gGame.key.key_ctrl = SDL_TRUE;
            break;
        case 18: // Altキーのコードは18 (ただし通常は特殊キー扱い)
            gGame.key.key_alt = SDL_TRUE;
            break;
        default:
            break;
    }
}

void keyboardUpFunc(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            gGame.key.key_w = SDL_FALSE;
            break;
        case 'a':
            gGame.key.key_a = SDL_FALSE;
            break;
        case 's':
            gGame.key.key_s = SDL_FALSE;
            break;
        case 'd':
            gGame.key.key_d = SDL_FALSE;
            break;
        case ' ':
            gGame.key.key_space = SDL_FALSE;
            break;
        case 16: // Shiftキーのコードは16
            gGame.key.key_shift = SDL_FALSE;
            break;
        case 17: // Ctrlキーのコードは17
            gGame.key.key_ctrl = SDL_FALSE;
            break;
        case 18: // Altキーのコードは18
            gGame.key.key_alt = SDL_FALSE;
            break;
        default:
            break;
    }
}

// GLUTのマウスボタンコールバック関数
void mouseFunc(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            gGame.mouse.left_button = SDL_TRUE;
        } else if (button == GLUT_RIGHT_BUTTON) {
            gGame.mouse.right_button = SDL_TRUE;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            gGame.mouse.middle_button = SDL_TRUE;
        }
    } else if (state == GLUT_UP) {
        if (button == GLUT_LEFT_BUTTON) {
            gGame.mouse.left_button = SDL_FALSE;
        } else if (button == GLUT_RIGHT_BUTTON) {
            gGame.mouse.right_button = SDL_FALSE;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            gGame.mouse.middle_button = SDL_FALSE;
        }
    }
}

// GLUTのマウスモーションコールバック関数
void motionFunc(int x, int y) {
    gGame.mouse.x = x;
    gGame.mouse.y = y;
}

void displayFunc() {
    drawStatic(); // 描画処理
    glutSwapBuffers(); // ダブルバッファリングのスワップ
}

void idleFunc() {
    // アイドル時の処理（ここでは特に何もしない）
    glutPostRedisplay(); // 再描画をリクエスト
}

/* メイン関数 */
int main(int argc, char** argv) { 
    // GLUTの初期化
    glutInit(&argc, argv);

    /** 初期化処理 **/
    InitWindow();
    InitSystem();

    // コールバック関数の登録
    glutDisplayFunc(displayFunc);
    glutIdleFunc(idleFunc);
    glutKeyboardFunc(keyboardFunc);
    glutKeyboardUpFunc(keyboardUpFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);

    // GLUTのメインループ開始
    glutMainLoop();

    return 0;
}
