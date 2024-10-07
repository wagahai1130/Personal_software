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
float RotationSpeed = 15.0f;
Objdata obj_s;
bool weapon = false;
bool skill = false;
bool special = false;
bool play = false;
bool back = false;

/* 関数 */

void updateWalkingAnimation(float deltaTime) {
    float armRotationSpeed = 100.0f;
    float legRotationSpeed = 100.0f;

    // 右腕の回転
    if (gGame.player->rightArmIncreasing) {
        gGame.player->rightArmRotation += armRotationSpeed * deltaTime;
        if (gGame.player->rightArmRotation > 30.0f) {
            gGame.player->rightArmRotation = 30.0f;  // 上限を30度に制限
            gGame.player->rightArmIncreasing = false;
        }
    } else {
        gGame.player->rightArmRotation -= armRotationSpeed * deltaTime;
        if (gGame.player->rightArmRotation < -30.0f) {
            gGame.player->rightArmRotation = -30.0f;  // 下限を-30度に制限
            gGame.player->rightArmIncreasing = true;
        }
    }

    // 左腕の回転も同様に
    if (gGame.player->leftArmIncreasing) {
        gGame.player->leftArmRotation -= armRotationSpeed * deltaTime;
        if (gGame.player->leftArmRotation < -30.0f) {
            gGame.player->leftArmRotation = -30.0f;  // 下限を-30度に制限
            gGame.player->leftArmIncreasing = false;
        }
    } else {
        gGame.player->leftArmRotation += armRotationSpeed * deltaTime;
        if (gGame.player->leftArmRotation > 30.0f) {
            gGame.player->leftArmRotation = 30.0f;  // 上限を30度に制限
            gGame.player->leftArmIncreasing = true;
        }
    }

    // 右足の回転も同様
    if (gGame.player->rightLegIncreasing) {
        gGame.player->rightLegRotation -= legRotationSpeed * deltaTime;
        if (gGame.player->rightLegRotation < -30.0f) {
            gGame.player->rightLegRotation = -30.0f;  // 下限を-30度に制限
            gGame.player->rightLegIncreasing = false;
        }
    } else {
        gGame.player->rightLegRotation += legRotationSpeed * deltaTime;
        if (gGame.player->rightLegRotation > 30.0f) {
            gGame.player->rightLegRotation = 30.0f;  // 上限を30度に制限
            gGame.player->rightLegIncreasing = true;
        }
    }

    // 左足の回転も同様
    if (gGame.player->leftLegIncreasing) {
        gGame.player->leftLegRotation += legRotationSpeed * deltaTime;
        if (gGame.player->leftLegRotation > 30.0f) {
            gGame.player->leftLegRotation = 30.0f;  // 上限を30度に制限
            gGame.player->leftLegIncreasing = false;
        }
    } else {
        gGame.player->leftLegRotation -= legRotationSpeed * deltaTime;
        if (gGame.player->leftLegRotation < -30.0f) {
            gGame.player->leftLegRotation = -30.0f;  // 下限を-30度に制限
            gGame.player->leftLegIncreasing = true;
        }
    }
}

// 背景の描画
void draw_selectWindow(GLuint texture) {
    obj_s = loadObjData("../select/select.txt");
    makeObj(obj_s, 0, 0, 0, texture); 
}

void draw_selectErement(GLuint texture) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // 例：オブジェクトの範囲内にマウスがある場合に実行する
    if (mouseX >= 1343 && mouseX <= 1659 && mouseY >= 215 && mouseY <= 280) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight1.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1211 && mouseX <= 1394 && mouseY >= 485 && mouseY <= 555) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight2.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1605 && mouseX <= 1881 && mouseY >= 485 && mouseY <= 555) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight3.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1431 && mouseX <= 1631 && mouseY >= 798 && mouseY <= 877) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight4.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }

    obj_s = loadObjData("../select/total.txt");
    glPushMatrix();
    glTranslatef(0.1f, 0.0f, 0.1f);
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 
    glPopMatrix();

    obj_s = loadObjData("../select/play.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/weapon.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/skill.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/special.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 
}

void draw_weaponErement(GLuint texture) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    // 例：オブジェクトの範囲内にマウスがある場合に実行する
    if (mouseX >= 1157 && mouseX <= 1416 && mouseY >= 292 && mouseY <= 374) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight_w1.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1594 && mouseX <= 1835 && mouseY >= 293 && mouseY <= 375) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight_w2.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1152 && mouseX <= 1431 && mouseY >= 589 && mouseY <= 684) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight_w3.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1600 && mouseX <= 1859 && mouseY >= 589 && mouseY <= 677) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight_w4.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    else if (mouseX >= 1388 && mouseX <= 1627 && mouseY >= 858 && mouseY <= 940) {
        // マウスが範囲内にある場合の処理
        obj_s = loadObjData("../select/highlight_w5.txt");
        makeObj(obj_s, 1.0f, 1.0f, 1.0f, elementTexture); 
    }
    obj_s = loadObjData("../select/weapon_sub.txt");
    glPushMatrix();
    glTranslatef(0.1f, 0.0f, 0.1f);
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 
    glPopMatrix();

    obj_s = loadObjData("../select/AK47_sub.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/AWP_sub.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/Sword_sub.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/Knife_sub.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 

    obj_s = loadObjData("../select/back_weapon.txt");
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture); 
}

// 3Dキャラクターの描画
void draw_3d_character() {
    glPushMatrix();
    glTranslatef(-59.1f, -0.7f, -0.66f);
    
    // キャラクターの回転
    glRotatef(RotationAngle, 0.0f, 0.0f, 1.0f);  // z軸周りに RotationAngle で回転
    
    // 頭部の描画
    obj_s = loadObjData("../select/head_s.txt");
    glPushMatrix();
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, charaTexture1);  // 右腕
    glPopMatrix();

    // 胴部の描画
    obj_s = loadObjData("../select/body_s.txt");
    glPushMatrix();
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, bodyTexture1);  // 右腕
    glPopMatrix();
    
    // 右腕の描画
    GLuint texture;
    if(gGame.player->weapon.AK_47 == SDL_TRUE){
        obj_s = loadObjData("../select/AK-47_rightArm_sub.txt");  
        texture = gun_arm_subTexture;  
    }
    else if(gGame.player->weapon.AWP == SDL_TRUE){
        obj_s = loadObjData("../select/AWP_rightArm_sub.txt");  
        texture = gun_arm_subTexture;  
    }
    else if(gGame.player->weapon.Sword == SDL_TRUE){
        obj_s = loadObjData("../select/Sword_rightArm_sub.txt");  
        texture = swordTexture;  
    }
    else if(gGame.player->weapon.Knife == SDL_TRUE){
        obj_s = loadObjData("../select/Knife_rightArm_sub.txt");  
        texture = swordTexture;  
    }
    else if(gGame.player->weapon.AK_47 == SDL_FALSE && gGame.player->weapon.AWP == SDL_FALSE && gGame.player->weapon.Sword == SDL_FALSE && gGame.player->weapon.Knife == SDL_FALSE){
        obj_s = loadObjData("../select/rightArm_s.txt");
        texture = bodyTexture1;
    }
    glPushMatrix();
    glTranslatef(-0.185f, 0.0f, 0.65f);  // 肩の位置
    glRotatef(gGame.player->rightArmRotation, 1.0f, 0.0f, 0.0f);  // 腕の回転
    glTranslatef(0.185f, 0.0f, -0.65f); 
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, texture);  // 右腕
    glPopMatrix();

    // 左腕の描画
    obj_s = loadObjData("../select/leftArm_s.txt");
    glPushMatrix();
    glTranslatef(0.185f, 0.0f, 0.65f);  // 肩の位置
    glRotatef(gGame.player->leftArmRotation, 1.0f, 0.0f, 0.0f);  // 腕の回転
    glTranslatef(-0.185f, 0.0f, -0.65f); 
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, bodyTexture1);  // 左腕
    glPopMatrix();
    
    // 右足の描画
    obj_s = loadObjData("../select/rightLeg_s.txt");
    glPushMatrix();
    glTranslatef(-0.075f, 0.0f, 0.3f);  // 股関節の位置
    glRotatef(gGame.player->rightLegRotation, 1.0f, 0.0f, 0.0f);  // 足の回転
    glTranslatef(0.075f, 0.0f, -0.3f);
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, bodyTexture1);  // 右足
    glPopMatrix();
    
    // 左足の描画
    obj_s = loadObjData("../select/leftLeg_s.txt");
    glPushMatrix();
    glTranslatef(0.075f, 0.0f, 0.3f);  // 股関節の位置
    glRotatef(gGame.player->leftLegRotation, 1.0f, 0.0f, 0.0f);  // 足の回転
    glTranslatef(-0.075f, 0.0f, -0.3f);
    makeObj(obj_s, 1.0f, 1.0f, 1.0f, bodyTexture1);  // 左足
    glPopMatrix();
    
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
    if(weapon == false && skill == false && special == false)
        draw_selectErement(0);
    else if(weapon == true)
        draw_weaponErement(0);

    // キャラクターの歩行モーションを更新
    updateWalkingAnimation(deltaTime);
    
    // 3Dキャラクターの描画
    RotationAngle += RotationSpeed * deltaTime;   // 回転速度を調整
    if (RotationAngle >= 360.0f) {
        RotationAngle -= 360.0f;
    }
    draw_3d_character();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    SDL_GL_SwapWindow(window);
}


void selectWeapon() {
    bool running = true;  // ループを制御するためのフラグ
    // 最初のフレーム時間を設定
    lastFrameTime = SDL_GetTicks();
    // イベントループの開始
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // イベントがキューにある間はそれを処理
            if (event.type == SDL_QUIT) {
                SDL_Quit();
            }
            if (event.type == SDL_KEYDOWN) {
                // キーが押されたときの処理
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;  // ループを終了
                    SDL_Quit();
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                // マウスクリックの処理
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= 1431 && x <= 1631 && y >= 798 && y <= 877) {
                    play = true;
                    running = false;
                }
                else if (x >= 1343 && x <= 1659 && y >= 215 && y <= 280) {
                    // マウスが範囲内にある場合の処理
                    weapon = true;
                }
                else if (x >= 1388 && x <= 1627 && y >= 858 && y <= 940 && weapon == true) {
                    // マウスが範囲内にある場合の処理
                    weapon = false;
                }
                /* 武器の種類によって分岐 */
                else if (x >= 1157 && x <= 1416 && y >= 292 && y <= 374) {
                    // マウスが範囲内にある場合の処理
                    gGame.player->weapon.AK_47 = SDL_TRUE;
                    gGame.player->weapon.AWP = SDL_FALSE;
                    gGame.player->weapon.Sword = SDL_FALSE;
                    gGame.player->weapon.Knife = SDL_FALSE;
                }
                else if (x >= 1594 && x <= 1835 && y >= 293 && y <= 375) {
                    // マウスが範囲内にある場合の処理
                    gGame.player->weapon.AWP = SDL_TRUE;
                    gGame.player->weapon.AK_47 = SDL_FALSE;
                    gGame.player->weapon.Sword = SDL_FALSE;
                    gGame.player->weapon.Knife = SDL_FALSE;
                }
                else if (x >= 1152 && x <= 1431 && y >= 589 && y <= 684) {
                    // マウスが範囲内にある場合の処理
                    gGame.player->weapon.AWP = SDL_FALSE;
                    gGame.player->weapon.AK_47 = SDL_FALSE;
                    gGame.player->weapon.Sword = SDL_TRUE;
                    gGame.player->weapon.Knife = SDL_FALSE;
                }
                else if (x >= 1600 && x <= 1859 && y >= 589 && y <= 677) {
                    // マウスが範囲内にある場合の処理
                    gGame.player->weapon.AWP = SDL_FALSE;
                    gGame.player->weapon.AK_47 = SDL_FALSE;
                    gGame.player->weapon.Sword = SDL_FALSE;
                    gGame.player->weapon.Knife = SDL_TRUE;
                }
                // マウスクリックされた位置の座標をターミナルに表示
                printf("クリック位置: X = %d, Y = %d\n", x, y);
            }
        }
        calculateDeltaTime();
        // 描画処理
        display_select();
        displayFPS();       

        // 画面更新を遅らせるために少し待機
        //SDL_Delay(16);  // 16ms = 約60FPS
    }
    gGame.stts = GS_Playing;
}


