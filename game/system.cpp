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
std::string textureDirectoryPath = "../textures";

GameInfo gGame;
std::vector<Objdata> objDataArray;
namespace fs = std::filesystem;
GLuint buildingTexture;
GLuint skyTexture;
Objdata obj;

GLUquadric* quadric;
float pos[] = { 0.0, 0.0, 0.0 };
float scale[] = { 1.0, 1.0, 1.0 };
float angle[] = { 0.0, 0.0, 0.0 };
float lightPos[] = {10.0, 15.0, 10.0, 1.0};
float eye[] = { 200.0, 200.0, 0.0};
float cnt[] = { 0.0, 0.0, 0.5};
int width = 1920;
int height = 1080;
double fovY = 45.0;
float sphereRotationAngle = 0.0f;  // 球体の回転角度を管理

GLuint staticObjectList;

GLuint loadTexture(const char* path);
void loadObjvalData(const std::string& filename, Objdata& obj);
void loadObjData(const std::string& filename);
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

void loadObjData(const std::string& filename) {
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
    buildingTexture = loadTexture("../texture/wipe.jpg");
    skyTexture = loadTexture("../texture/sky.jpg");
    std::cout << "Total loaded objects: " << objDataArray.size() << std::endl;
}

void renderBuildings(float r, float g, float b) {
    loadObjData("../map/game_map.txt");
    makeObj(obj, r, g, b, buildingTexture); 
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

    eye[0] = gGame.player->point.x;
    eye[1] = gGame.player->point.y;
    eye[2] = gGame.player->point.z + 0.5;
}
