/*
 * ファイル名 : window.cpp
 * 機能       : ユーザーインターフェース処理
 */
#include "system.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

/* 変数 */
int selected_weapon = -1;     // 選択された武器のインデックス
float RotationAngle = 0.0f;  // 3Dキャラクターの回転角度
float RotationSpeed = 2.5f;

/* 関数 */

// 背景の描画
void draw_selectWindow(GLuint texture) {
    obj = loadObjData("../select/select.txt");
    //glPushMatrix();
    makeObj(obj, 0, 0, 0, texture); 
    //glRotatef(gCameraYaw - 90.0f, 0.0f, 0.0f, 1.0f); 
    //glPopMatrix();
}

void draw_selectErement(GLuint texture) {
    obj = loadObjData("../select/select_element.txt");
    makeObj(obj, 0, 0, 0, texture); 
}

void draw_weaponWindow(GLuint texture) {
    obj = loadObjData("../select/selectWeapon.txt");
    makeObj(obj, 0, 0, 0, texture); 
}

void draw_weaponErement(GLuint texture) {
    obj = loadObjData("../select/weapon_element.txt");
    makeObj(obj, 0, 0, 0, texture); 
}

// 3Dキャラクターの描画
void draw_3d_character() {
    obj = loadObjData("../select/chara.txt");
    glPushMatrix();
    glTranslatef(-59.1f, -0.7f, -0.66f);
    makeObj(obj, 1.0f, 1.0f, 1.0f, bodyTexture1); 
    //glRotatef(RotationAngle, 0.0f, 0.0f, 1.0f);  // z軸周りに回転
    glPopMatrix();
}

void setSelectcamera(){
    // モデルビュー行列の設定
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-58.4634, -2.5637, -0.05,    // カメラの位置
              -58.4634, -2.136, -0.05,        // 注視点
              0.0, 0.0, 1.0);                // 上方向
}

// 描画処理
void display_select() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    setSelectcamera();
    glColor3f(0.0f, 0.0f, 0.0f);
    // 背景の描画
    draw_selectWindow(skyTexture);
    draw_selectErement(elementTexture);
    // 3Dキャラクターの描画
    RotationAngle += RotationSpeed * deltaTime;   // 回転速度を調整
    if (RotationAngle >= 360.0f) {
        RotationAngle -= 360.0f;
    }
    draw_3d_character();
    glEnable(GL_TEXTURE_2D);

    SDL_GL_SwapWindow(window);
}

void selectWeapon() {
    bool running = true;  // ループを制御するためのフラグ

    // イベントループの開始
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // イベントがキューにある間はそれを処理
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                // キーが押されたときの処理
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;  // ループを終了
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                // マウスクリックの処理
                int x, y;
                SDL_GetMouseState(&x, &y);
            }
        }

        // 描画処理
        display_select();

        // 画面更新を遅らせるために少し待機
        //SDL_Delay(16);  // 16ms = 約60FPS
    }

    // SDLのクリーンアップ
    SDL_Quit();
}


