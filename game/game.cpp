/*
 * ファイル名： game.cpp
 * 機能　　　： メインルーチン
 */
#include "system.h"
#include "constants.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <chrono>

// フレームレート計測用の変数
std::chrono::time_point<std::chrono::steady_clock> startTime;
int frameCount = 0;
float gCameraYaw = -90.0f;
float cameraPitch = 0.0f;
bool firstMouse = true;
float lastX = 0.0f, lastY = 0.0f;
int CenterX, CenterY;
float SphereRotationSpeed = 2.5f; 
// フレームごとの経過時間を保持する変数
Uint32 lastFrameTime = 0; // 前フレームの時刻
float deltaTime = 0.0f;    // 経過時間
// エフェクトの表示状態を管理する変数
bool effectActive = false;      // エフェクトが表示中かどうか
Uint32 effectStartTime = 0;     // エフェクトが開始された時間
Uint32 effectDuration = 500;    // エフェクトの表示時間 (ミリ秒)
bool isMousePressed = false;  // マウスが押されているかどうかを管理するフラグ
glm::vec3 surfaceNorm(0.0f, 0.0f, 1.0f); //壁の法線方向

SDL_Window* window;
SDL_GLContext context;

// 経過時間を計算する関数
void calculateDeltaTime() {
    Uint32 currentFrameTime = SDL_GetTicks();
    deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;

    // deltaTimeが0.1秒（10FPS以下相当）を超えないようにする
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }

    lastFrameTime = currentFrameTime;
}

void displayFPS() {
    static Uint32 lastTime = 0;
    static int frames = 0;
    Uint32 currentTime = SDL_GetTicks();
    frames++;

    if (currentTime - lastTime > 1000) { // 1秒ごとにFPSを表示
        float fps = frames * 1000.0f / (currentTime - lastTime);
        std::cout << "FPS: " << fps << std::endl;
        frames = 0;
        lastTime = currentTime;
    }
}

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

void handleMouseButtonEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            std::cout << "Left mouse button pressed at (" << event.button.x << ", " << event.button.y << ")" << std::endl;

            isMousePressed = true;  // マウスが押されたらフラグをオン

            if (gGame.player->weapon.AK_47 == SDL_TRUE || gGame.player->weapon.AWP == SDL_TRUE) {
                std::cout << "AK47 or AWP selected, triggering bullet effect" << std::endl;
                effectActive = true;  // エフェクトを有効化
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            std::cout << "Left mouse button released" << std::endl;
            isMousePressed = false;
            effectActive = false;
        }
    }
}

void handleMouseMotionEvent() {
    int xOffset, yOffset;

    // 相対モードでのマウスの動きを取得
    SDL_GetRelativeMouseState(&xOffset, &yOffset);
    // 初回の大きな動作を無視
    if (firstMouse) {
        // 大きなオフセットが発生した場合は無視する
        if (abs(xOffset) > 100 || abs(yOffset) > 100) {
            std::cout << "Ignoring large mouse movement offset (xOffset: " << xOffset << ", yOffset: " << yOffset << ")" << std::endl;
            return;
        }
        firstMouse = false;
        return;
    }

    // 感度の調整
    float yawSensitivity = 0.2f;    // 横方向の感度（強め）
    float pitchSensitivity = 0.05f;  // 縦方向の感度（弱め）

    float yawOffset = -xOffset * yawSensitivity;
    float pitchOffset = yOffset * pitchSensitivity;

    // カメラの回転角度を更新
    gCameraYaw += yawOffset;
    cameraPitch -= pitchOffset; 

    // ピッチ角を制限（上下方向の視界を制限）
    cameraPitch = std::clamp(cameraPitch, -89.0f, 89.0f);

    // カメラの注視点を更新
    float radius = 10.0f; // 注視点が回転する半径
    cnt[0] = eye[0] + radius * cos(glm::radians(gCameraYaw)) * cos(glm::radians(cameraPitch));
    cnt[1] = eye[1] + radius * sin(glm::radians(gCameraYaw)) * cos(glm::radians(cameraPitch));
    cnt[2] = eye[2] + radius * sin(glm::radians(cameraPitch));
}

void centerMouseCursor(SDL_Window* window) {
    int width, height;

    // ウィンドウのサイズを取得
    SDL_GetWindowSize(window, &width, &height);

    // 中心座標を計算
    int centerX = width / 2;
    int centerY = height / 2;

    // マウスカーソルをウィンドウの中央に移動
    SDL_WarpMouseInWindow(window, centerX, centerY);
}

void displayFunc() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setCamera();
    
    glCallList(staticObjectList);
    
    drawTexturedSphere(200.0f, skyTexture);
    renderPlayer(0,0,0);
    createBulletEffect(1.0f, 1.0f, 1.0f);
    if (isMousePressed) {
        glm::vec3 hitPosition = raycastFromCamera(1000.0f);
        if (hitPosition != glm::vec3(0, 0, 0)) {
            // 銃痕を作成（3秒後に消えるように設定）
            createBulletHole(hitPosition.x, hitPosition.y, hitPosition.z, 2000);  // 3000ミリ秒 (3秒)後に消える
        }
    }
    renderBulletHoles();
    drawCrosshair();
    SDL_GL_SwapWindow(window);
}

void updateRightArmRotationByCameraPitch() {
    // カメラのピッチ角度に基づいて右腕の回転を制御
    float maxArmRotation = -45.0f;  // 最大回転角度
    float minArmRotation = 45.0f; // 最小回転角度

    // cameraPitchは上下の向きなので、その値を-89度〜89度にマッピング
    float normalizedPitch = glm::clamp(cameraPitch, -89.0f, 89.0f);

    // ピッチに応じて回転角度を設定
    gGame.player->rightArmRotation = glm::mix(minArmRotation, maxArmRotation, (normalizedPitch + 89.0f) / 178.0f);
}

void updatePlayerPosition() {
    float moveSpeed = 5.0f;
    float jumpForce = 5.0f;  // ジャンプ力
    float yawRadians = glm::radians(gCameraYaw);

    float forwardX = cos(yawRadians);
    float forwardY = sin(yawRadians);

    float rightX = cos(yawRadians + glm::radians(90.0f));
    float rightY = sin(yawRadians + glm::radians(90.0f));

    bool isMoving = false;
    float moveX = 0.0f;
    float moveY = 0.0f;

    // 現在のプレイヤーの位置を保存
    float originalX = gGame.player->point.x;
    float originalY = gGame.player->point.y;
    float originalZ = gGame.player->point.z;

    // プレイヤーの移動処理
    if (gGame.key.key_w) {
        moveX += forwardX;
        moveY += forwardY;
        isMoving = true;
    }
    if (gGame.key.key_s) {
        moveX -= forwardX;
        moveY -= forwardY;
        isMoving = true;
    }
    if (gGame.key.key_a) {
        moveX += rightX;
        moveY += rightY;
        isMoving = true;
    }
    if (gGame.key.key_d) {
        moveX -= rightX;
        moveY -= rightY;
        isMoving = true;
    }

    // 移動ベクトルの正規化
    float length = sqrt(moveX * moveX + moveY * moveY);
    if (length > 0) {
        moveX /= length;
        moveY /= length;
    }

    // 正規化したベクトルに速度を掛けて移動
    gGame.player->point.x += moveX * moveSpeed * deltaTime;
    gGame.player->point.y += moveY * moveSpeed * deltaTime;

    // ジャンプ処理
    if (gGame.key.key_space && !gGame.player->isJumping) {
        gGame.player->isJumping = true;
        gGame.player->jumpSpeed = jumpForce;  // ジャンプ開始時の速度
    }

    if (gGame.player->isJumping) {
        // ジャンプ中の処理
        gGame.player->point.z += gGame.player->jumpSpeed * deltaTime;  // ジャンプ中の移動にdeltaTimeを掛ける
        gGame.player->jumpSpeed += GRAVITY * deltaTime; // マクロ定義の重力を使用

        // 地面に戻ったらジャンプ終了
        if (gGame.player->point.z <= 0.0f) {
            gGame.player->point.z = 0.0f;  // 地面に位置を固定
            gGame.player->isJumping = false;
            gGame.player->jumpSpeed = 0.0f;
        }
    }

    // プレイヤーの各部位ごとの当たり判定を実施
    if (checkCollisionForPlayerParts(gGame.player, objDataArray)) {
        // 衝突が発生した場合、プレイヤーの位置を元に戻す
        gGame.player->point.x = originalX;
        gGame.player->point.y = originalY;
        std::cout << "Collision detected, reverting player position." << std::endl;
    }

    // 手足の回転処理はここで復元
    float armRotationSpeed = 100.0f;
    float legRotationSpeed = 100.0f;

    if (isMoving) {
        if (gGame.player->leftArmIncreasing) {
            gGame.player->leftArmRotation -= armRotationSpeed * deltaTime;
            if (gGame.player->leftArmRotation < -30.0f) {
                gGame.player->leftArmIncreasing = false;
            }
        } else {
            gGame.player->leftArmRotation += armRotationSpeed * deltaTime;
            if (gGame.player->leftArmRotation > 30.0f) {
                gGame.player->leftArmIncreasing = true;
            }
        }

        if (gGame.player->rightLegIncreasing) {
            gGame.player->rightLegRotation -= legRotationSpeed * deltaTime;
            if (gGame.player->rightLegRotation < -30.0f) {
                gGame.player->rightLegIncreasing = false;
            }
        } else {
            gGame.player->rightLegRotation += legRotationSpeed * deltaTime;
            if (gGame.player->rightLegRotation > 30.0f) {
                gGame.player->rightLegIncreasing = true;
            }
        }

        if (gGame.player->leftLegIncreasing) {
            gGame.player->leftLegRotation += legRotationSpeed * deltaTime;
            if (gGame.player->leftLegRotation > 30.0f) {
                gGame.player->leftLegIncreasing = false;
            }
        } else {
            gGame.player->leftLegRotation -= legRotationSpeed * deltaTime;
            if (gGame.player->leftLegRotation < -30.0f) {
                gGame.player->leftLegIncreasing = true;
            }
        }
    } else {
        // 移動していない場合、手足の回転をリセット
        gGame.player->rightArmRotation = 0.0f;
        gGame.player->leftArmRotation = 0.0f;
        gGame.player->rightLegRotation = 0.0f;
        gGame.player->leftLegRotation = 0.0f;
    }

    // --- カメラの位置を更新 ---
    float cameraDistance = -0.1f;
    float pitchRadians = glm::radians(cameraPitch); 
    eye[0] = gGame.player->point.x - forwardX * cameraDistance;
    eye[1] = gGame.player->point.y - forwardY * cameraDistance;
    eye[2] = gGame.player->point.z + 0.6f;

    cnt[0] = eye[0] + forwardX;
    cnt[1] = eye[1] + forwardY;
    cnt[2] = eye[2] + sin(pitchRadians);
    printf("%f %f %f, \n", gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
}


void idleFunc() {
    calculateDeltaTime(); // 毎フレームの経過時間を計算
    updatePlayerPosition();
    updateRightArmRotationByCameraPitch();
    // 球体の回転角度を更新
    sphereRotationAngle += SphereRotationSpeed * deltaTime;   // 回転速度を調整
    if (sphereRotationAngle >= 360.0f) {
        sphereRotationAngle -= 360.0f;
    }
    displayFunc();
    displayFPS();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
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

    InitSystem();
    InitWindow(window, context);
    
    // ゲームのフェーズに応じて処理を変更
    if (gGame.stts == GS_WeaponSelect) {
        selectWeapon(); // 武器選択画面を表示
        centerMouseCursor(window);
    } 
    // 相対モードを有効にする
    if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
        std::cerr << "Failed to enable relative mouse mode: " << SDL_GetError() << std::endl;
        return -1;
    }
    lastFrameTime = SDL_GetTicks();
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
            } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                handleMouseButtonEvent(event);  // マウスボタンイベントの処理
            }
        }
        idleFunc();
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
