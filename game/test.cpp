// ビル群の側面との当たり判定は、ｘ，ｙ座標つまり上から見下ろしていると仮定して行う
// ビル群の上面との当たり判定は、上記の条件かつｚ座標との比較を行う

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <GL/glut.h>
#include <GL/glu.h> // GLUライブラリを追加
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct ObjData
{
    std::vector<GLfloat> vertex;
    std::vector<GLfloat> flat;
    std::vector<GLfloat> sortedVertex;
    std::vector<GLfloat> sortedFlat;
    std::vector<GLfloat> UV;
    std::vector<int> index;
};

ObjData obj;
float pos[] = { 0.0, 0.0, 0.0 };
float scale[] = { 1.0, 1.0, 1.0 };
float angle[] = { 0.0, 0.0, 0.0 };
float lightPos[] = {10.0, 15.0, 10.0, 1.0};
static float eye[] = { 200.0, 200.0, 0.0};  // カメラを球の内側に配置
static float cnt[] = { 0.0, 0.0, 0.0}; // カメラの視線をZ軸方向に設定
int width = 1980;
int height = 1080;
double fovY = 45.0;
GLuint texture1;
GLuint texture2;

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // テクスチャパラメータの設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
         GLenum format = GL_RGB;  // デフォルトでRGBとして扱う
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        // テクスチャ画像をバインドする
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // 手動でミップマップを生成する
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void setLight()
{
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

void setCamera()
{
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], cnt[0], cnt[1], cnt[2], 0.0, 0.0, 1.0);
}

void loadObjData(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind("v,", 0) == 0)
        {
            GLfloat x, y, z;
            sscanf(line.c_str(), "v,%f,%f,%f", &x, &y, &z);
            obj.vertex.push_back(x);
            obj.vertex.push_back(y);
            obj.vertex.push_back(z);
        }
        else if (line.rfind("n,", 0) == 0)
        {
            GLfloat nx, ny, nz;
            sscanf(line.c_str(), "n,%f,%f,%f", &nx, &ny, &nz);
            obj.flat.push_back(nx);
            obj.flat.push_back(ny);
            obj.flat.push_back(nz);
        }
        else if (line.rfind("i,", 0) == 0)
        {
            int idx1, idx2, idx3;
            sscanf(line.c_str(), "i,%d,%d,%d", &idx1, &idx2, &idx3);
            obj.index.push_back(idx1);
            obj.index.push_back(idx2);
            obj.index.push_back(idx3);
        }
        else if (line.rfind("u,", 0) == 0)
        {
            GLfloat u, v;
            sscanf(line.c_str(), "u,%f,%f", &u, &v);
            obj.UV.push_back(u);
            obj.UV.push_back(v);
        }
    }

    obj.sortedVertex.resize(obj.index.size() * 3);
    obj.sortedFlat.resize(obj.index.size() * 3);
    for (size_t i = 0; i < obj.index.size(); i++)
    {
        int idx = obj.index[i];
        obj.sortedVertex[i * 3] = obj.vertex[idx * 3];
        obj.sortedVertex[i * 3 + 1] = obj.vertex[idx * 3 + 1];
        obj.sortedVertex[i * 3 + 2] = obj.vertex[idx * 3 + 2];
        obj.sortedFlat[i * 3] = obj.flat[idx * 3];
        obj.sortedFlat[i * 3 + 1] = obj.flat[idx * 3 + 1];
        obj.sortedFlat[i * 3 + 2] = obj.flat[idx * 3 + 2];
    }
}

void makeObj(float r, float g, float b, GLuint textureID)
{
    glVertexPointer(3, GL_FLOAT, 0, obj.sortedVertex.data());
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(textureID != 0) {
        glEnable(GL_TEXTURE_2D);
         // テクスチャと glColor の色を混合しない設定
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    glNormalPointer(GL_FLOAT, 0, obj.sortedFlat.data());
    glColor3f(r, g, b);

    for (size_t i = 0; i < obj.index.size() / 3; i++)
    {
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

// 球体描画用の変数
GLUquadric* quadric;

// 球体の描画関数
void drawTexturedSphere(GLfloat radius, GLuint textureID) {
    glPushMatrix();
    
    // 球体を(120, 120, 0)に移動
    glTranslatef(120.0f, 120.0f, 0.0f);

    // テクスチャをバインドして有効化
    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_TEXTURE_2D);

    // カリングを無効化して球体の内側を描画
    glDisable(GL_CULL_FACE);

    // 球体を描画（球の内側から見たときにテクスチャが見えるように描画）
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluSphere(quadric, radius, 50, 50);

    // カリングとテクスチャを再度無効化
    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

// 初期化関数の追加
void initSphere() {
    quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
}

void draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    setCamera();
    makeObj(1.0f, 1.0f, 1.0f, texture1); // 色とテクスチャIDを指定

    // テクスチャ付きの球を描画 (半径 100)
    drawTexturedSphere(120.0f, texture2);

    glutSwapBuffers();
}

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    setLight();
    glShadeModel(GL_SMOOTH);
    initSphere(); // 球体の初期化を追加
    
}

void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovY, (double)w/(double)h, 0.1, 500.0); // 視野範囲を大きく
    glMatrixMode(GL_MODELVIEW);
    width = w;
    height = h;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Example");

    glutReshapeFunc(resize);
    glutDisplayFunc(draw);

    init();
    texture1 = loadTexture("../texture/wipe.jpg"); 
    texture2 = loadTexture("../texture/sky.jpg");
    loadObjData("../map/game_map.txt");

    glutMainLoop();
    return 0;
}
