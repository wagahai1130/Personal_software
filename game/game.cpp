/*
 * ファイル名： game.cpp
 * 機能　　　： メインルーチン
 */
#include "system.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_video.h> // SDLでウィンドウ関連の機能を扱うために必要なヘッダファイル
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <filesystem>

// SDL_Eventによるイベントの検知
SDL_Event event;

/* エラーメッセージ表示 */
int PrintError(const char* str)
{
    fprintf(stderr, "%s\n", str);
    return -1;
}

/** タイマー処理 **/
Uint32 AniTimer(Uint32 interval, void* param)
{
    if (*(int*)param > 0) {
        if (gGame.stts == GS_Ready && gGame.timeStep > 0.0)
            gGame.stts = GS_Playing;
        gGame.timeStep = 0.1 / *(int*)param;
        printf("FPS: %d\r", *(int*)param * 10);
        *(int*)param = 1;
    }
    return interval;
}

/* マウス、キーボード操作読み取り */
SDL_bool InputEvent(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return SDL_FALSE;
        
        case SDL_KEYDOWN:
            printf("Key Down: %s\n", SDL_GetKeyName(event.key.keysym.sym));
            if (event.key.repeat)
                break;
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                gGame.key.key_esc = SDL_TRUE;
                return SDL_FALSE;
            case SDLK_w:
                gGame.key.key_w = SDL_TRUE;
                printf("w");
                break;
            case SDLK_a:
                gGame.key.key_a = SDL_TRUE;
                break;
            case SDLK_s:
                gGame.key.key_s = SDL_TRUE;
                break;
            case SDLK_d:
                gGame.key.key_d = SDL_TRUE;
                break;
            case SDLK_SPACE:
                gGame.key.key_space = SDL_TRUE;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                gGame.key.key_shift = SDL_TRUE;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                gGame.key.key_ctrl = SDL_TRUE;
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                gGame.key.key_alt = SDL_TRUE;
                break;
            default:
                break;
            }
            break;

        case SDL_KEYUP:
            printf("Key Up: %s\n", SDL_GetKeyName(event.key.keysym.sym));
            if (event.key.repeat)
                break;
            switch (event.key.keysym.sym) {
            case SDLK_w:
                gGame.key.key_w = SDL_FALSE;
                break;
            case SDLK_a:
                gGame.key.key_a = SDL_FALSE;
                break;
            case SDLK_s:
                gGame.key.key_s = SDL_FALSE;
                break;
            case SDLK_d:
                gGame.key.key_d = SDL_FALSE;
                break;
            case SDLK_SPACE:
                gGame.key.key_space = SDL_FALSE;
                break;
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                gGame.key.key_shift = SDL_FALSE;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                gGame.key.key_ctrl = SDL_FALSE;
                break;
            case SDLK_LALT:
            case SDLK_RALT:
                gGame.key.key_alt = SDL_FALSE;
                break;
            case SDLK_ESCAPE:
                gGame.key.key_esc = SDL_FALSE;
                break;
            default:
                break;
            }
            break;

        case SDL_MOUSEMOTION:
            gGame.mouse.x = event.motion.x;
            gGame.mouse.y = event.motion.y;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
                gGame.mouse.left_button = SDL_TRUE;
            else if (event.button.button == SDL_BUTTON_RIGHT)
                gGame.mouse.right_button = SDL_TRUE;
            else if (event.button.button == SDL_BUTTON_MIDDLE)
                gGame.mouse.middle_button = SDL_TRUE;
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
                gGame.mouse.left_button = SDL_FALSE;
            else if (event.button.button == SDL_BUTTON_RIGHT)
                gGame.mouse.right_button = SDL_FALSE;
            else if (event.button.button == SDL_BUTTON_MIDDLE)
                gGame.mouse.middle_button = SDL_FALSE;
            break;

        default:
            break;
        }
    }
    return SDL_TRUE;
}

/* メイン関数 */
int main(int argc, char** argv) { 
    // GLUTの初期化
    glutInit(&argc, argv);

    /** 初期化処理 **/
    /* SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        return PrintError(SDL_GetError());
    }
     
    /* 簡易フレームカウンタ */
    int framecnt = 0;
    /** タイマー起動 **/
    SDL_TimerID atimer = SDL_AddTimer(100, AniTimer, &framecnt);
    if (atimer == 0) {
        PrintError(SDL_GetError());
        exit(0);
    }

    InitWindow();
    InitSystem();

    // objDataArrayの最初の要素を表示
    if (!objDataArray.empty()) {
        const Objdata& firstObj = objDataArray.front();
        
        // ファイル名
        std::cout << "Filename: " << firstObj.filename << std::endl;

        // バウンディングボックスの最小値
        std::cout << "Bounding Box Min: ";
        for (size_t i = 0; i < firstObj.box_min.size(); i += 3) {
           std::cout << "(" << firstObj.box_min[i] << ", " << firstObj.box_min[i+1] << ", " << firstObj.box_min[i+2] << ") ";
        }
        std::cout << std::endl;

        // バウンディングボックスの最大値
        std::cout << "Bounding Box Max: ";
        for (size_t i = 0; i < firstObj.box_max.size(); i += 3) {
            std::cout << "(" << firstObj.box_max[i] << ", " << firstObj.box_max[i+1] << ", " << firstObj.box_max[i+2] << ") ";
        }
        std::cout << std::endl;

        // 頂点
        std::cout << "Vertices: ";
        for (size_t i = 0; i < firstObj.vertex.size(); i += 3) {
            std::cout << "(" << firstObj.vertex[i] << ", " << firstObj.vertex[i+1] << ", " << firstObj.vertex[i+2] << ") ";
        }
        std::cout << std::endl;

        // 法線ベクトル
        std::cout << "Normals: ";
        for (size_t i = 0; i < firstObj.flat.size(); i += 3) {
            std::cout << "(" << firstObj.flat[i] << ", " << firstObj.flat[i+1] << ", " << firstObj.flat[i+2] << ") ";
        }
        std::cout << std::endl;

        // ソートされた頂点
        std::cout << "Sorted Vertices: ";
        for (size_t i = 0; i < firstObj.sortedVertex.size(); i += 3) {
            std::cout << "(" << firstObj.sortedVertex[i] << ", " << firstObj.sortedVertex[i+1] << ", " << firstObj.sortedVertex[i+2] << ") ";
        }
        std::cout << std::endl;

        // ソートされた法線ベクトル
        std::cout << "Sorted Normals: ";
        for (size_t i = 0; i < firstObj.sortedFlat.size(); i += 3) {
            std::cout << "(" << firstObj.sortedFlat[i] << ", " << firstObj.sortedFlat[i+1] << ", " << firstObj.sortedFlat[i+2] << ") ";
        }
        std::cout << std::endl;

        // UV座標
        std::cout << "UV Coordinates: ";
        for (size_t i = 0; i < firstObj.UV.size(); i += 3) {
            std::cout << "(" << firstObj.UV[i] << ", " << firstObj.UV[i+1] << "," << firstObj.UV[i+2] << ") ";
        }
        std::cout << std::endl;

        // インデックス
        std::cout << "Indices: ";
        for (size_t i = 0; i < firstObj.index.size(); i += 3) {
            std::cout << "(" << firstObj.index[i] << ", " << firstObj.index[i+1] << ", " << firstObj.index[i+2] << ") ";
        }
        std::cout << std::endl;

    } else {
        std::cout << "No objects loaded into objDataArray." << std::endl;
    }

    /** メインループ **/
    SDL_bool loopflg = SDL_TRUE;
    while (loopflg) {
        loopflg = InputEvent();
        // 描画処理
        drawStatic();
    }
    SDL_Quit();
    return 0;
}
