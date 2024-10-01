/*
 * ファイル名 : window.cpp
 * 機能       : ユーザーインターフェース処理
 */
#include "system.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

/* 変数 */
int selected_weapon = -1;     // 選択された武器のインデックス
float rotation_angle = 0.0f;  // 3Dキャラクターの回転角度

/* 関数 */
void draw_background(GLuint texture);

// 背景の描画
void draw_background(GLuint texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WD_Width, 0, WD_Height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(WD_Width, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(WD_Width, WD_Height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, WD_Height);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// 3Dキャラクターの描画
void draw_3d_character() {
    glPushMatrix();
    glTranslatef(-gGame.player->point.x, -gGame.player->point.y, -gGame.player->point.z);
    renderPlayer(0,0,0);
    glRotatef(rotation_angle, 0.0f, 0.0f, 1.0f);  // z軸周りに回転
    glPopMatrix();
}

// 武器選択UIの描画
void draw_2d_ui() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WD_Width, 0, WD_Height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const int button_width = 100;
    const int button_height = 50;
    const int button_x = WD_Width - 150;
    for (int i = 0; i < 4; i++) {
        int button_y = WD_Height - (i + 1) * 60;  // ボタンのY座標

        if (i == selected_weapon) {
            glColor3f(1.0f, 1.0f, 0.0f);  // 選択された武器は黄色
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);  // 他の武器は白
        }

        glBegin(GL_QUADS);
        glVertex2f(button_x, button_y);
        glVertex2f(button_x + button_width, button_y);
        glVertex2f(button_x + button_width, button_y + button_height);
        glVertex2f(button_x, button_y + button_height);
        glEnd();
    }
}

// 描画処理
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 背景の描画
    draw_background();
    gluLookAt(0.0, -1.0, 0.0,    // カメラの位置
              0.0, 0.0, 0.0,                 // 注視点
              0.0, 0.0, 1.0);                // 上方向
    // 3Dキャラクターの描画
    rotation_angle += 0.5f;
    draw_3d_character();

    // 2D UIの描画
    draw_2d_ui();

    glutSwapBuffers();
}

// マウスクリックによる武器選択処理
void handle_mouse_click(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        const int button_width = 100;
        const int button_height = 50;
        const int button_x = WD_Width - 150;

        for (int i = 0; i < 4; i++) {
            int button_y = WD_Height - (i + 1) * 60;

            // マウスクリックがボタンの範囲内か判定
            if (x >= button_x && x <= button_x + button_width &&
                (WD_Height - y) >= button_y && (WD_Height - y) <= button_y + button_height) {
                selected_weapon = i;  // 選択された武器のインデックスを更新
                printf("Weapon %d selected\n", selected_weapon);
            }
        }
    }
}

void selectWeapon() {
    glutDisplayFunc(display);        // 描画関数
    glutMouseFunc(handle_mouse_click);  // マウスクリック処理
    glutMainLoop();  // メインループ
}

