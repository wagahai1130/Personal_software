/*
 * ファイル名： game.cpp
 * 機能　　　： メインルーチン
 */
#include "system.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <chrono>

// フレームレート計測用の変数
std::chrono::time_point<std::chrono::steady_clock> startTime;
int frameCount = 0;
float gCameraYaw = 0.0f;
float cameraPitch = 0.0f;
bool firstMouse = true;
float lastX = 0.0f, lastY = 0.0f;
int CenterX, CenterY;

SDL_Window* window;
SDL_GLContext context;

void handleKeyboardEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                exit(0);
            case SDLK_w:
                gGame.key.key_w = SDL_TRUE;
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
                gGame.key.key_shift = SDL_TRUE;
                break;
            case SDLK_LCTRL:
                gGame.key.key_ctrl = SDL_TRUE;
                break;
            case SDLK_LALT:
                gGame.key.key_alt = SDL_TRUE;
                break;
        }
    } else if (event.type == SDL_KEYUP) {
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
                gGame.key.key_shift = SDL_FALSE;
                break;
            case SDLK_LCTRL:
                gGame.key.key_ctrl = SDL_FALSE;
                break;
            case SDLK_LALT:
                gGame.key.key_alt = SDL_FALSE;
                break;
        }
    }
}

void handleMouseMotionEvent() {
    int xOffset, yOffset;

    // 相対モードでのマウスの動きを取得
    SDL_GetRelativeMouseState(&xOffset, &yOffset);

    // 感度の調整
    float yawSensitivity = 0.1f;    // 横方向の感度（強め）
    float pitchSensitivity = 0.01f;  // 縦方向の感度（弱め）
    
    float yawOffset = -xOffset * yawSensitivity;
    float pitchOffset = yOffset * pitchSensitivity;

    // カメラの回転角度を更新
    gCameraYaw += yawOffset;
    cameraPitch -= pitchOffset; // ピッチは上下反転するので符号を反転

    // ピッチ角を制限（上下方向の視界を制限）
    cameraPitch = std::clamp(cameraPitch, -89.0f, 89.0f);

    // カメラの注視点を更新
    float radius = 10.0f; // 注視点が回転する半径
    cnt[0] = eye[0] + radius * cos(glm::radians(gCameraYaw)) * cos(glm::radians(cameraPitch));
    cnt[1] = eye[1] + radius * sin(glm::radians(gCameraYaw)) * cos(glm::radians(cameraPitch));
    cnt[2] = eye[2] + radius * sin(glm::radians(cameraPitch));
}


void displayFunc() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setCamera();
    
    glCallList(staticObjectList);
    
    drawTexturedSphere(200.0f, skyTexture);
    drawCrosshair();
    SDL_GL_SwapWindow(window);
}

void updatePlayerPosition() {
    float moveSpeed = 0.1f;
    float yawRadians = glm::radians(gCameraYaw);

    float forwardX = cos(yawRadians);
    float forwardY = sin(yawRadians);

    float rightX = cos(yawRadians + glm::radians(90.0f));
    float rightY = sin(yawRadians + glm::radians(90.0f));

    if (gGame.key.key_w) {
        gGame.player->point.x += forwardX * moveSpeed;
        gGame.player->point.y += forwardY * moveSpeed;
    }
    if (gGame.key.key_s) {
        gGame.player->point.x -= forwardX * moveSpeed;
        gGame.player->point.y -= forwardY * moveSpeed;
    }
    if (gGame.key.key_a) {
        gGame.player->point.x += rightX * moveSpeed;
        gGame.player->point.y += rightY * moveSpeed;
    }
    if (gGame.key.key_d) {
        gGame.player->point.x -= rightX * moveSpeed;
        gGame.player->point.y -= rightY * moveSpeed;
    }

    eye[0] = gGame.player->point.x;
    eye[1] = gGame.player->point.y;
    eye[2] = gGame.player->point.z + 0.5;

    cnt[0] = eye[0] + forwardX;
    cnt[1] = eye[1] + forwardY;
}

void idleFunc() {
    updatePlayerPosition();
    // 球体の回転角度を更新
    sphereRotationAngle += 0.025f;  // 回転速度を調整
    if (sphereRotationAngle >= 360.0f) {
        sphereRotationAngle -= 360.0f;
    }
    displayFunc();
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    // フルスクリーンモードに切り替え
    if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) < 0) {
        std::cerr << "Could not switch to fullscreen! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cerr << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
    }
    // 相対モードを有効にする
    if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
        std::cerr << "Failed to enable relative mouse mode: " << SDL_GetError() << std::endl;
        return -1;
    }

    InitSystem();
    InitWindow(window, context);
    
    createStaticObjectList();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                handleKeyboardEvent(event);
            } else if (event.type == SDL_MOUSEMOTION) {
                handleMouseMotionEvent();
            }
        }
        idleFunc();
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
