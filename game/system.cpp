/**
 ファイル名	：system.cpp
 機能：ゲームシステム処理
 **/
#include "system.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string objDirectoryPath = "../obj";
std::string charaCollisionDirectoryPath = "../chara_collision";
std::string textureDirectoryPath = "../texture";

GameInfo gGame;
std::vector<Objdata> objDataArray;
std::vector<Objdata> charaDataArray;
namespace fs = std::filesystem;
GLuint buildingTexture;
GLuint skyTexture;
GLuint charaTexture1;
GLuint bodyTexture1;
GLuint selectTexture;
GLuint elementTexture;
GLuint redTexture;
GLuint gun_arm_subTexture;
GLuint swordTexture;
GLuint bulletTexture;
Objdata obj;
Data AK47_bulletdata;
Data AWP_bulletdata;
// 凸包のキャッシュ
std::unordered_map<std::string, std::vector<glm::vec3>> convexHullCache;

GLUquadric* quadric;
float pos[] = { 0.0, 0.0, 0.0 };
float scale[] = { 1.0, 1.0, 1.0 };
float angle[] = { 0.0, 0.0, 0.0 };
float lightPos[] = {10.0, 15.0, 10.0, 1.0};
float eye[] = { 200.0, 200.0, 0.0};
float cnt[] = { 0.0, 0.0, 1.0};
int width = 1920;
int height = 1080;
double fovY = 45.0;
float sphereRotationAngle = 0.0f;  // 球体の回転角度を管理

GLuint staticObjectList;

GLuint loadTexture(const char* path);
void loadObjvalData(const std::string& filename, Objdata& obj);
Objdata loadObjData(const std::string& filename);
std::vector<Objdata> loadAllObjData(const std::string& directoryPath);
void makeObj(const Objdata& obj, float r, float g, float b, GLuint textureID);
void initializeRendering();
void renderBuildings(float r, float g, float b);
void setLight();
void setCamera();
void drawTexturedSphere(GLfloat radius, GLuint textureID);
void initSphere();
void drawStatic();
void resize(int w, int h);
void InitWindow(SDL_Window*& window, SDL_GLContext& context);
void InitSystem();
void drawFloorWithGrid();
void drawCrosshair();

void InitWindow(SDL_Window*& window, SDL_GLContext& context) {
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // ウィンドウのサイズを取得
    SDL_GetWindowSize(window, &width, &height);

    // 中心座標を計算
    CenterX = width / 2;
    CenterY = height / 2;
}

void InitSystem() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovY, (double)width / (double)height, 0.1, 500.0);

    glMatrixMode(GL_MODELVIEW);

    setLight();
    glShadeModel(GL_SMOOTH);
    initSphere();
    initializeRendering();

    gGame.player = new CharaInfo();
    gGame.player->point.x = 150.0f;
    gGame.player->point.y = 150.0f;
    gGame.player->point.z = 0.0f;
    gGame.player->isJumping = false;
    gGame.player->jumpSpeed = 0.0f;
    gGame.stts = GS_WeaponSelect;

    eye[0] = gGame.player->point.x;
    eye[1] = gGame.player->point.y;
    eye[2] = gGame.player->point.z + 0.6;
}

// ファイルからデータを読み取り、構造体に格納する関数
int read_data_from_file(const char *filename, Data *data) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;  // ファイルが開けなかった場合
    }

    // ファイルから3つの浮動小数点数を読み取る
    if (fscanf(file, "%f,%f,%f", &data->x, &data->y, &data->z) != 3) {
        fprintf(stderr, "Error reading data from file\n");
        fclose(file);
        return -1;  // データの読み取りに失敗した場合
    }

    fclose(file);  // ファイルを閉じる
    return 0;  // 成功
}

void createStaticObjectList() {
    staticObjectList = glGenLists(1);
    glNewList(staticObjectList, GL_COMPILE);
    renderBuildings(1.0f, 1.0f, 1.0f);
    drawFloorWithGrid();
    
    glEndList();
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
         GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void loadObjvalData(const std::string& filename, Objdata& obj) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("s,", 0) == 0) {
            GLfloat sx, sy, sz;
            sscanf(line.c_str(), "s,%f,%f,%f", &sx, &sy, &sz);
            obj.box_min.push_back(sx);
            obj.box_min.push_back(sy);
            obj.box_min.push_back(sz);
        } else if (line.rfind("b,", 0) == 0) {
            GLfloat bx, by, bz;
            sscanf(line.c_str(), "b,%f,%f,%f", &bx, &by, &bz);
            obj.box_max.push_back(bx);
            obj.box_max.push_back(by);
            obj.box_max.push_back(bz);
        } else if (line.rfind("v,", 0) == 0) {
            GLfloat x, y, z;
            sscanf(line.c_str(), "v,%f,%f,%f", &x, &y, &z);
            obj.vertex.push_back(x);
            obj.vertex.push_back(y);
            obj.vertex.push_back(z);
        } else if (line.rfind("n,", 0) == 0) {
            GLfloat nx, ny, nz;
            sscanf(line.c_str(), "n,%f,%f,%f", &nx, &ny, &nz);
            obj.flat.push_back(nx);
            obj.flat.push_back(ny);
            obj.flat.push_back(nz);
        } else if (line.rfind("i,", 0) == 0) {
            int idx1, idx2, idx3;
            sscanf(line.c_str(), "i,%d,%d,%d", &idx1, &idx2, &idx3);
            obj.index.push_back(idx1);
            obj.index.push_back(idx2);
            obj.index.push_back(idx3);
        } else if (line.rfind("u,", 0) == 0) {
            GLfloat ux, uy, uz;
            sscanf(line.c_str(), "u,%f,%f,%f", &ux, &uy, &uz);
            obj.UV.push_back(ux);
            obj.UV.push_back(uy);
            obj.UV.push_back(uz);
        }
    }

    obj.sortedVertex.resize(obj.index.size() * 3);
    obj.sortedFlat.resize(obj.index.size() * 3);
    for (size_t i = 0; i < obj.index.size(); i++) {
        int idx = obj.index[i];
        obj.sortedVertex[i * 3] = obj.vertex[idx * 3];
        obj.sortedVertex[i * 3 + 1] = obj.vertex[idx * 3 + 1];
        obj.sortedVertex[i * 3 + 2] = obj.vertex[idx * 3 + 2];
        obj.sortedFlat[i * 3] = obj.flat[idx * 3];
        obj.sortedFlat[i * 3 + 1] = obj.flat[idx * 3 + 1];
        obj.sortedFlat[i * 3 + 2] = obj.flat[idx * 3 + 2];
    }
}

Objdata loadObjData(const std::string& filename) {
    Objdata obj;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("v,", 0) == 0) {
            GLfloat x, y, z;
            sscanf(line.c_str(), "v,%f,%f,%f", &x, &y, &z);
            obj.vertex.push_back(x);
            obj.vertex.push_back(y);
            obj.vertex.push_back(z);
        } else if (line.rfind("n,", 0) == 0) {
            GLfloat nx, ny, nz;
            sscanf(line.c_str(), "n,%f,%f,%f", &nx, &ny, &nz);
            obj.flat.push_back(nx);
            obj.flat.push_back(ny);
            obj.flat.push_back(nz);
        } else if (line.rfind("i,", 0) == 0) {
            int idx1, idx2, idx3;
            sscanf(line.c_str(), "i,%d,%d,%d", &idx1, &idx2, &idx3);
            obj.index.push_back(idx1);
            obj.index.push_back(idx2);
            obj.index.push_back(idx3);
        } else if (line.rfind("u,", 0) == 0) {
            GLfloat u, v;
            sscanf(line.c_str(), "u,%f,%f", &u, &v);
            obj.UV.push_back(u);
            obj.UV.push_back(v);
        }
    }

    obj.sortedVertex.resize(obj.index.size() * 3);
    obj.sortedFlat.resize(obj.index.size() * 3);
    for (size_t i = 0; i < obj.index.size(); i++) {
        int idx = obj.index[i];
        obj.sortedVertex[i * 3] = obj.vertex[idx * 3];
        obj.sortedVertex[i * 3 + 1] = obj.vertex[idx * 3 + 1];
        obj.sortedVertex[i * 3 + 2] = obj.vertex[idx * 3 + 2];
        obj.sortedFlat[i * 3] = obj.flat[idx * 3];
        obj.sortedFlat[i * 3 + 1] = obj.flat[idx * 3 + 1];
        obj.sortedFlat[i * 3 + 2] = obj.flat[idx * 3 + 2];
    }

    return obj;
}

std::vector<Objdata> loadAllObjData(const std::string& directoryPath) {
    std::vector<fs::path> filePaths;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            std::cout << "Found file: " << entry.path() << std::endl;
            filePaths.push_back(entry.path());
        }
    }

    std::sort(filePaths.begin(), filePaths.end());

    std::vector<Objdata> loadedObjects;
    for (const auto& path : filePaths) {
        Objdata obj;
        loadObjvalData(path.string(), obj);
        if (!obj.vertex.empty()) {
            obj.filename = path.filename().string();
            loadedObjects.push_back(obj);
            std::cout << "Loaded file: " << obj.filename << std::endl;
        }
    }

    return loadedObjects;
}

void makeObj(const Objdata& obj, float r, float g, float b, GLuint textureID) {
    glVertexPointer(3, GL_FLOAT, 0, obj.sortedVertex.data());
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(textureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    glNormalPointer(GL_FLOAT, 0, obj.sortedFlat.data());
    glColor3f(r, g, b);

    for (size_t i = 0; i < obj.index.size() / 3; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(obj.sortedFlat[i * 3], obj.sortedFlat[i * 3 + 1], obj.sortedFlat[i * 3 + 2]);
        glTexCoord2d(obj.UV[i * 6], obj.UV[i * 6 + 1]);
        glVertex3f(obj.sortedVertex[i * 9], obj.sortedVertex[i * 9 + 1], obj.sortedVertex[i * 9 + 2]);
        glTexCoord2d(obj.UV[i * 6 + 2], obj.UV[i * 6 + 3]);
        glVertex3f(obj.sortedVertex[i * 9 + 3], obj.sortedVertex[i * 9 + 4], obj.sortedVertex[i * 9 + 5]);
        glTexCoord2d(obj.UV[i * 6 + 4], obj.UV[i * 6 + 5]);
        glVertex3f(obj.sortedVertex[i * 9 + 6], obj.sortedVertex[i * 9 + 7], obj.sortedVertex[i * 9 + 8]);
        glEnd();
    }
    if(textureID != 0) {
        glDisable(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initializeRendering() {
    std::cout << "Loading objects from directory: " << objDirectoryPath << std::endl;
    objDataArray = loadAllObjData(objDirectoryPath);
    std::cout << "Loading objects from directory: " << charaCollisionDirectoryPath << std::endl;
    charaDataArray = loadAllObjData(charaCollisionDirectoryPath);
    buildingTexture = loadTexture("../texture/wipe.jpg");
    skyTexture = loadTexture("../texture/sky.jpg");
    charaTexture1 = loadTexture("../texture/face1.png");
    bodyTexture1 = loadTexture("../texture/body1.png");
    selectTexture = loadTexture("../texture/select.jpg"); 
    elementTexture = loadTexture("../texture/element.jpg");
    redTexture = loadTexture("../texture/red.jpg");
    gun_arm_subTexture = loadTexture("../texture/gun_arm.jpg");
    swordTexture = loadTexture("../texture/sword.jpg");
    bulletTexture = loadTexture("../texture/bullet.jpg");
    read_data_from_file("../weapon/AK-47_bulletPos.txt", AK47_bulletdata);
    read_data_from_file("../weapon/AWP_bulletPos.txt", AWP_bulletdata);
    std::cout << "Total loaded objects: " << objDataArray.size() << std::endl;
    std::cout << "Total loaded charaData: " << charaDataArray.size() << std::endl;
}

void renderBuildings(float r, float g, float b) {
    obj = loadObjData("../map/game_map.txt");
    makeObj(obj, r, g, b, buildingTexture); 
}

// キャラ1
//頭部
void renderHead(float r, float g, float b){
    obj = loadObjData("../chara/head.txt");
    
    glPushMatrix();

    glTranslatef(gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
    // プレイヤーの体をカメラの方向に回転させる
    glRotatef(gCameraYaw + 90.0f, 0.0f, 0.0f, 1.0f);  // Z軸（上下）方向に基づいて回転
    makeObj(obj, r, g, b, charaTexture1);

    glPopMatrix();
}

//胴部
void renderBody(float r, float g, float b){
    obj = loadObjData("../chara/body.txt");
    
    glPushMatrix();

    glTranslatef(gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
    // プレイヤーの体をカメラの方向に回転させる
    glRotatef(gCameraYaw + 90.0f, 0.0f, 0.0f, 1.0f);  // Z軸（上下）方向に基づいて回転
    makeObj(obj, r, g, b, bodyTexture1);

    glPopMatrix();
}

//右腕
void renderRightArm(float r, float g, float b) {
    GLuint texture;
    if(gGame.player->weapon.AK_47 == SDL_TRUE){
        obj = loadObjData("../select/AK-47_rightArm_sub.txt");  
        texture = gun_arm_subTexture;  
    }
    else if(gGame.player->weapon.AWP == SDL_TRUE){
        obj = loadObjData("../select/AWP_rightArm_sub.txt");  
        texture = gun_arm_subTexture;  
    }
    else if(gGame.player->weapon.Sword == SDL_TRUE){
        obj = loadObjData("../select/Sword_rightArm_sub.txt");  
        texture = swordTexture;  
    }
    else if(gGame.player->weapon.Knife == SDL_TRUE){
        obj = loadObjData("../select/Knife_rightArm_sub.txt");  
        texture = swordTexture;  
    }
    else if(gGame.player->weapon.AK_47 == SDL_FALSE && gGame.player->weapon.AWP == SDL_FALSE && gGame.player->weapon.Sword == SDL_FALSE && gGame.player->weapon.Knife == SDL_FALSE){
        obj = loadObjData("../chara/rightArm.txt");
        texture = bodyTexture1;
    }

    glPushMatrix();

    // 全身の胴体とともに回転させる
    glTranslatef(gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
    glRotatef(gCameraYaw + 90.0f, 0.0f, 0.0f, 1.0f); // 胴体の回転に合わせて全体を回転

    // 肩の位置（胴体からの相対位置）
    glTranslatef(-0.185f, 0.0f, 0.65f);

    // 腕の回転を行う
    glRotatef(gGame.player->rightArmRotation, 1.0f, 0.0f, 0.0f);

    glTranslatef(0.185f, 0.0f, -0.65f);

    // 右腕を描画
    makeObj(obj, r, g, b, texture);

    glPopMatrix();
}


//左腕
void renderLeftArm(float r, float g, float b){
    obj = loadObjData("../chara/leftArm.txt");
    
    glPushMatrix();

    // 全身の胴体とともに回転させる
    glTranslatef(gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
    glRotatef(gCameraYaw + 90.0f, 0.0f, 0.0f, 1.0f); // 胴体の回転に合わせて全体を回転

    // 肩の位置（胴体からの相対位置）
    glTranslatef(0.185f, 0.0f, 0.65f);

    // 腕の回転を行う
    glRotatef(gGame.player->leftArmRotation, 1.0f, 0.0f, 0.0f);

    glTranslatef(-0.185f, 0.0f, -0.65f);

    // 左腕を描画
    makeObj(obj, r, g, b, bodyTexture1);

    glPopMatrix();
}

//右足
void renderRightLeg(float r, float g, float b){
    obj = loadObjData("../chara/rightLeg.txt");
    glPushMatrix();

    // (-0.075, 0.0, 0.3)
    // 全身の胴体とともに回転させる
    glTranslatef(gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
    glRotatef(gCameraYaw + 90.0f, 0.0f, 0.0f, 1.0f); // 胴体の回転に合わせて全体を回転

    //（胴体からの相対位置）
    glTranslatef(-0.075f, 0.0f, 0.3f);

    glRotatef(gGame.player->rightLegRotation, 1.0f, 0.0f, 0.0f);

    glTranslatef(0.075f, 0.0f, -0.3f);

    makeObj(obj, r, g, b, bodyTexture1);

    glPopMatrix();
}

//左足
void renderLeftLeg(float r, float g, float b){
    obj = loadObjData("../chara/leftLeg.txt");
    glPushMatrix();

   // 全身の胴体とともに回転させる
    glTranslatef(gGame.player->point.x, gGame.player->point.y, gGame.player->point.z);
    glRotatef(gCameraYaw + 90.0f, 0.0f, 0.0f, 1.0f); // 胴体の回転に合わせて全体を回転

    //（胴体からの相対位置）
    glTranslatef(0.075f, 0.0f, 0.3f);

    // 腕の回転を行う
    glRotatef(gGame.player->leftLegRotation, 1.0f, 0.0f, 0.0f);

    glTranslatef(-0.075f, 0.0f, -0.3f);
    // 右腕を描画
    makeObj(obj, r, g, b, bodyTexture1);

    glPopMatrix();
}

void renderPlayer(float r, float g, float b){
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glColor3f(0.0f, 0.0f, 0.0f);
    
    // 個々の部品を描画
    renderHead(r, g, b);
    renderBody(r, g, b);
    renderRightArm(r, g, b);
    renderLeftArm(r, g, b);
    renderRightLeg(r, g, b);
    renderLeftLeg(r, g, b);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void createBullet(float r, float g, float b){
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);
    if(gGame.player->weapon.AK_47 == SDL_TRUE){
        obj = loadObjData("../weapon/AK-47_bullet.txt");  
        texture = bulletTexture;  
        glPushMatrix();

        glTranslatef(gGame.player->point.x + AK47_bulletdata.x, gGame.player->point.y + AK47_bulletdata.y, gGame.player->point.z + AK47_bulletdata.z);
        makeObj(obj, r, g, b, texture);
        glPopMatrix();
    }
    else if(gGame.player->weapon.AWP == SDL_TRUE){
        obj = loadObjData("../weapon/AWP_bullet.txt");  
        texture = bulletTexture;  
        glPushMatrix();

        glTranslatef(gGame.player->point.x + AWP_bulletdata.x, gGame.player->point.y + AWP_bulletdata.y, gGame.player->point.z + AWP_bulletdata.z);
        makeObj(obj, r, g, b, texture);
        glPopMatrix();
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void setLight() {
    float lightDiffuse0[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float lightAmbient0[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float lightSpecular0[] = {1.0, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
}

void setCamera() {
    
    // モデルビュー行列の設定
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // カメラをプレイヤーの位置に設定し、進行方向を向くように設定
    gluLookAt(eye[0], eye[1], eye[2],    // カメラ位置
              cnt[0], cnt[1], cnt[2],       // 注視点（handleMouseMotionEventで計算済み）
              0.0, 0.0, 1.0);               // 上方向ベクトル
}

void drawTexturedSphere(GLfloat radius, GLuint textureID) {
    glPushMatrix();

    // まず、平行移動を行い、その位置を基準に回転させる
    glTranslatef(120.0f, 120.0f, 0.0f);

    // 球体の回転を適用
    glRotatef(sphereRotationAngle, 0.0f, 0.0f, 1.0f); // Y軸周りに回転

    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_TEXTURE_2D);

    glDisable(GL_CULL_FACE);

    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluSphere(quadric, radius, 50, 50);

    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}


void initSphere() {
    quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
}

void drawFloorWithGrid() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glVertex3f(-100.0f, -100.0f, 0.0f);
    glVertex3f( 400.0f, -100.0f, 0.0f);
    glVertex3f( 400.0f,  400.0f, 0.0f);
    glVertex3f(-100.0f,  400.0f, 0.0f);
    glEnd();

    glColor3f(1.0f, 0.0f, 1.0f);

    glBegin(GL_LINES);
    for (float i = -100.0f; i <= 400.0f; i += 2.0f) {
        glVertex3f(i, -100.0f, 0.02f);
        glVertex3f(i,  400.0f, 0.02f);
        
        glVertex3f(-100.0f, i, 0.02f);
        glVertex3f( 400.0f, i, 0.02f);
    }
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void drawCrosshair() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_LINES);
    glVertex2f(width / 2 - 10, height / 2);
    glVertex2f(width / 2 + 10, height / 2);
    glVertex2f(width / 2, height / 2 - 10);
    glVertex2f(width / 2, height / 2 + 10);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

std::vector<glm::vec3> calculateConvexHullQhull(const std::vector<glm::vec3>& points) {
    std::vector<glm::vec3> hull;

    // Qhull用にデータを準備
    int numPoints = points.size();
    int dimension = 3;  // 3次元
    std::vector<coordT> qhullPoints;
    for (const auto& point : points) {
        qhullPoints.push_back(point.x);
        qhullPoints.push_back(point.y);
        qhullPoints.push_back(point.z);
    }

    // Qhullの初期化 (qhT型のポインタを使用)
    qhT *qh = (qhT*) malloc(sizeof(qhT));  // ポインタでメモリ確保
    if (!qh) {
        std::cerr << "Memory allocation failed for qhT" << std::endl;
        return hull;
    }
    qh_zero(qh, nullptr);  // Qhullの初期化関数を呼び出し

    // qh_new_qhull呼び出し
    int exitcode = qh_new_qhull(qh, dimension, numPoints, qhullPoints.data(), False, (char*)"qhull Qt", NULL, NULL);
    if (exitcode != 0) {
        std::cerr << "Qhull error: Failed to compute convex hull" << std::endl;
        qh_freeqhull(qh, True);
        free(qh);  // メモリ解放
        return hull;
    }

    // 凸包の頂点を取得
    facetT *facet;
    vertexT *vertex, **vertexp;
    FORALLfacets {
        FOREACHvertex_(facet->vertices) {
            coordT *coords = vertex->point;
            hull.emplace_back(glm::vec3(coords[0], coords[1], coords[2]));
        }
    }

    // Qhullのメモリ解放
    qh_freeqhull(qh, True);
    free(qh);  // mallocで確保したメモリを解放
    return hull;
}


// 分離軸定理(SAT)を使って、2つの凸包が重なっているかを判定する関数
bool overlapOnAxis(const std::vector<glm::vec3>& hull1, const std::vector<glm::vec3>& hull2, const glm::vec3& axis) {
    auto project = [](const std::vector<glm::vec3>& hull, const glm::vec3& axis) -> std::pair<float, float> {
        float min = glm::dot(hull[0], axis);
        float max = min;
        for (const auto& point : hull) {
            float projection = glm::dot(point, axis);
            min = std::min(min, projection);
            max = std::max(max, projection);
        }
        return {min, max};
    };

    auto [min1, max1] = project(hull1, axis);
    auto [min2, max2] = project(hull2, axis);

    // 重なっているかを確認
    return !(max1 < min2 || max2 < min1);
}

// 凸包同士の衝突判定を行う関数
bool checkCollisionConvexHull(const std::vector<glm::vec3>& hull1, const std::vector<glm::vec3>& hull2) {
    // 凸包の面法線を使って、衝突判定を行う
    std::vector<glm::vec3> axes;

    // 凸包1の各面の法線を抽出
    for (size_t i = 0; i < hull1.size(); i++) {
        glm::vec3 edge = hull1[(i + 1) % hull1.size()] - hull1[i];
        axes.push_back(glm::normalize(glm::cross(edge, glm::vec3(0, 0, 1))));
    }

    // 凸包2の各面の法線を抽出
    for (size_t i = 0; i < hull2.size(); i++) {
        glm::vec3 edge = hull2[(i + 1) % hull2.size()] - hull2[i];
        axes.push_back(glm::normalize(glm::cross(edge, glm::vec3(0, 0, 1))));
    }

    // 分離軸定理(SAT)を使って各軸でオーバーラップをチェック
    for (const auto& axis : axes) {
        if (!overlapOnAxis(hull1, hull2, axis)) {
            return false;  // オーバーラップがない軸があるなら衝突していない
        }
    }

    return true;  // すべての軸でオーバーラップしているなら衝突している
}

// 建物などの静的オブジェクトの凸包をキャッシュする関数
std::vector<glm::vec3> getOrCreateConvexHull(const Objdata& obj) {
    if (convexHullCache.find(obj.filename) != convexHullCache.end()) {
        return convexHullCache[obj.filename];  // キャッシュされたデータを利用
    }
    
    // 新たに凸包を計算
    std::vector<glm::vec3> points;
    for (size_t i = 0; i < obj.vertex.size(); i += 3) {
        points.emplace_back(obj.vertex[i], obj.vertex[i + 1], obj.vertex[i + 2]);
    }
    auto hull = calculateConvexHullQhull(points);
    convexHullCache[obj.filename] = hull;  // 計算結果をキャッシュ
    return hull;
}

bool checkCollisionForPlayerParts(const CharaInfo* player, const std::vector<Objdata>& objects) {
    // 衝突判定に使うプレイヤーの周囲の半径
    const float collisionRadius = 30.0f;  // 50ユニットを範囲とする（調整可能）

    // プレイヤー全体のオブジェクトを取得
    const Objdata& playerObj = charaDataArray[0];
    std::vector<glm::vec3> playerPoints;

    // プレイヤーの全体モデルの頂点に現在座標を加算
    for (size_t i = 0; i < playerObj.vertex.size(); i += 3) {
        glm::vec3 transformedVertex(
            playerObj.vertex[i] + player->point.x, 
            playerObj.vertex[i + 1] + player->point.y,
            playerObj.vertex[i + 2] + player->point.z
        );
        playerPoints.push_back(transformedVertex);
    }

    // プレイヤー全体の凸包をQhullで計算
    std::vector<glm::vec3> playerConvexHull = calculateConvexHullQhull(playerPoints);

    // 各オブジェクトとの衝突を判定
    for (const auto& obj : objects) {
        // オブジェクトの中心点を計算
        glm::vec3 objectCenter(
            (obj.box_min[0] + obj.box_max[0]) / 2,
            (obj.box_min[1] + obj.box_max[1]) / 2,
            (obj.box_min[2] + obj.box_max[2]) / 2
        );

        // プレイヤーとの距離を計算
        glm::vec3 playerPosition(player->point.x, player->point.y, player->point.z);
        float distance = glm::distance(objectCenter, playerPosition);

        // 距離が一定の範囲内であれば当たり判定を行う
        if (distance < collisionRadius) {
            // オブジェクトの凸包をQhullで計算
            std::vector<glm::vec3> objectConvexHull = getOrCreateConvexHull(obj);

            // プレイヤーの全体モデルとオブジェクトの衝突判定
            if (checkCollisionConvexHull(playerConvexHull, objectConvexHull)) {
                std::cout << "Collision detected with object: " << obj.filename << std::endl;
                return true;  // 衝突が検出されたら即座に終了
            }
        }
    }

    return false;  // 衝突がなければfalseを返す
}

