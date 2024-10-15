/*
 *  ファイル名	: system.h
 *  機能	: 共通変数，外部関数の定義
 */
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>
#include <GL/glut.h>
#include <GL/glu.h> // GLUライブラリを追加
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
extern "C" {
#include <libqhull_r/libqhull_r.h>
}
#include <unordered_map>
#include <netinet/in.h>

/* ウインドウサイズ(ピクセル) */
#define WD_Width 1920
#define WD_Height 1080

/* 時間,距離など */
#define NORMAL_VELOCITY 500 // 通常の移動速度
#define GRAVITY -18.0f  // 重力の値

/* オブジェクトのデータを格納する構造体*/
struct Objdata {
    std::vector<GLfloat> box_min; //バウンディングボックスの最小値
    std::vector<GLfloat> box_max; //                        最大値
    std::vector<GLfloat> vertex;  //頂点
    std::vector<GLfloat> flat;    
    std::vector<GLfloat> sortedVertex;
    std::vector<GLfloat> sortedFlat;
    std::vector<GLfloat> UV;
    std::vector<int> index;
    std::string filename;       // ファイル名を格納
}; 
#define OBJ_NUM 667

struct OBB {
    glm::vec3 center;      // OBBの中心
    glm::vec3 halfSizes;   // OBBの各軸の半径（長さの半分）
    glm::mat3 orientation; // OBBの回転を表す3x3の行列
};

// キャラの関節の座標を格納する構造体
typedef struct {
    char type;      // 'r(右手)', 'l(左手)', 'R(右足)', 'L(左足)'など
    float x;       
    float y;        
    float z;        
} CharaJointData;

/* 初期座標を格納する構造体 */
typedef struct {
    float x;
    float y;
    float z;
} Data;

struct BulletHole {
    glm::vec3 position;      // 銃痕の座標
    glm::vec3 normal;
    Uint32 createTime;       // 銃痕が作成された時間
    Uint32 duration;         // 銃痕が表示される時間（ミリ秒）
};

/* キャラクタータイプ */
typedef enum {
    CT_Player   = 0,  // プレイヤー
    CT_Enemy    = 1,  // 敵
    CT_Boss     = 2,  // ボス
    CT_Barrier  = 3, // 障壁
    CT_BossHP   = 4, // ボスのHPバー
    CT_Time     = 5,  // 残り時間
    CT_Level    = 6,  // 現在のレベル
    CT_Null     = 7,  // 配置なし
    CT_PlayerHP_frame = 8, //プレイヤーのHPバーの枠
    CT_PlayerHP = 9,  // プレイヤーのHPバー
    CT_EXP_frame = 10,  // 経験値
    CT_EXP      = 11,   // プレイヤーのEXPバー
    CT_FixedEnemy = 12  // 六角形の敵
} CharaType;
#define CHARATYPE_NUM 13 // キャラタイプ総数

/* キャラクターの状態 */
typedef enum {
    CS_Disable  = 0, // 非表示
    CS_Normal   = 1, // 通常
    CS_Avoid    = 2, // ジャンプ
    CS_AvoidEnd = 3, // ジャンプの終わり
    CS_Shot     = 4, // 攻撃
    CS_ShotEnd  = 5, // 攻撃終わり
    CS_Hit      = 6, // 被弾
    CS_HitEnd   = 7,  // 被弾終わり
} CharaStts;

/* 武器の選択状態 */
typedef struct {
    SDL_bool AK_47;
    SDL_bool AWP;
    SDL_bool Sword;
    SDL_bool Knife;
} WeaponStts;

/* スキルの選択状態 */
typedef struct {
    SDL_bool Asteroid;
} SkillStts;

/* 必殺技の選択状態 */
typedef struct {
    SDL_bool Hujin;
} SpecialStts;

/* キャラクタータイプ別情報 */
typedef struct {
    Objdata obj;      // キャラの形状
    int w;            // キャラの幅
    int h;            // 高さ
    int cx;           // 画像1つの左上を0としたときの中心位置x
    int cy;           //                                     y
    float rad;        // 当たり判定の半径
    float vel;        // 速度の基準値
    char* path;       // 画像ファイル名
    GLuint* img;      // キャラのテクスチャーID
} CharaTypeInfo;

typedef struct CharaInfo_t {
    CharaStts stts;        // 現在の状態
    CharaType type;        // キャラクタータイプ
    WeaponStts weapon;
    SkillStts skill;
    SpecialStts special;
    CharaTypeInfo* entity; // タイプ別情報の実体
    SDL_FPoint dir;        // 現在の方向（大きさ最大1となる成分）
    struct {
        float x, y, z;     // 3次元空間での現在の座標
    } point;              
    int rest;              // 残り時間(特殊な動作をするのに必要な，フレーム数)
    union {
        int restfst; // restの初期値
    };
    bool isJumping;       // ジャンプ中かどうか
    float jumpSpeed;      // ジャンプの速度
    float rightArmRotation;  // 右腕の回転角度
    float leftArmRotation;   // 左腕の回転角度
    bool rightArmIncreasing; // 右腕の回転方向
    bool leftArmIncreasing;  // 左腕の回転方向

    // 足の回転角度と方向
    float rightLegRotation;  // 右足の回転角度
    float leftLegRotation;   // 左足の回転角度
    bool rightLegIncreasing; // 右足の回転方向
    bool leftLegIncreasing;  // 左足の回転方向
} CharaInfo;


/* メッセージ */
typedef enum {
    MSG_None     = 0, // 表示なし
    MSG_GameOver = 1, // ゲームオーバー
    MSG_Clear    = 2, // クリア
    MSG_Title    = 3, // タイトル
    MSG_Show     = 4, // サブ
    MSG_End      = 5  
} Msg;
#define MSG_NUM 6 // メッセージの数

/* ゲームパッド入力の状態 */
typedef struct {
    double ana_x;       // 左スティック状態x
    double ana_y;       //                 y                           
    double joy_x;       // 右スティック状態x
    double joy_y;       //                 y
    SDL_bool one;
    SDL_bool two;
    SDL_bool three;
    SDL_bool four;
    SDL_bool five;
    SDL_bool six;
    SDL_bool seven; 
    SDL_bool eight; 
    SDL_bool nine;
    SDL_bool ten;
    SDL_bool eleven;
    SDL_bool twelve;
} PadStts;

/* マウスの状態 */
typedef struct {
    int x;          // マウスのx座標
    int y;          // マウスのy座標
    SDL_bool left_button;   // 左ボタンの状態
    SDL_bool right_button;  // 右ボタンの状態
    SDL_bool middle_button; // 中央ボタンの状態
} MouseStts;

/* キーボードの状態 */
typedef struct {
    SDL_bool key_w;     // Wキーの状態
    SDL_bool key_a;     // Aキーの状態
    SDL_bool key_s;     // Sキーの状態
    SDL_bool key_d;     // Dキーの状態
    SDL_bool key_space; // スペースキーの状態
    SDL_bool key_shift; // シフトキーの状態
    SDL_bool key_ctrl;  // コントロールキーの状態
    SDL_bool key_alt;   // アルトキーの状態
    SDL_bool key_esc;   // エスケープキーの状態  
} KeyboardStts;


/* ゲームの状態 */
typedef enum {
    GS_WeaponSelect = 0, // 武器選択画面
    GS_Playing = 1,      // ゲーム中
} GameStts;

/* ゲームの情報 */
typedef struct {
    GameStts stts;                  // ゲームの状態
    MouseStts mouse;
    KeyboardStts key;     
    float timeStep;                 // 時間の増分(1フレームの時間,s)
    CharaInfo* player;              // プレイヤー
    Msg msg;                        // 表示中のメッセージ番号
    GLuint* msgTexts[MSG_NUM]; // メッセージ画像のテクスチャID
} GameInfo;

/* 変数 */
extern GameInfo gGame;
extern CharaTypeInfo gCharaType[CHARATYPE_NUM];
extern CharaInfo* gChara;
extern SDL_Window* window;
extern SDL_GLContext context;
extern int gCharaNum; // キャラ総数
extern int gWeaponNum; // 武器総数
extern CharaInfo* enemies;
extern CharaInfo* gWeapon;
extern int enemyCount;
//extern SDL_Joystick *joystick;	// ジョイスティックを特定・利用するための構造体
extern int selectedOption;
extern std::vector<Objdata> objDataArray;
extern std::vector<Objdata> charaDataArray;
extern Objdata obj;
extern float eye[3];
extern float cnt[3];
extern GLuint staticObjectList; // ディスプレイリストのID
extern int width;
extern int height;
extern int CenterX;
extern int CenterY;
extern float sphereRotationAngle;
extern GLuint buildingTexture;
extern GLuint skyTexture;
extern GLuint selectTexture;
extern float gCameraYaw;  // カメラの向きを表すグローバル変数
extern float cameraPitch;
extern std::unordered_map<std::string, std::vector<glm::vec3>> convexHullCache;
extern GLuint charaTexture1;
extern GLuint bodyTexture1;
extern GLuint elementTexture;
extern GLuint redTexture;
extern GLuint gun_arm_subTexture;
extern GLuint swordTexture;
extern float deltaTime;
extern Uint32 lastFrameTime;
extern Data AK47_muzzledata;
extern Data AWP_muzzledata;
extern Data AK47_effectdata;
extern Data AWP_effectdata;
extern bool effectActive;      // エフェクトが表示中かどうか
extern Uint32 effectStartTime;     // エフェクトが開始された時間
extern Uint32 effectDuration; 
extern bool isMousePressed;

/* 関数 */
// game.cpp
int PrintError(const char* str);
void calculateDeltaTime();
void displayFPS();
// system.cpp
int read_data_from_file(const char *filename, Data *data);
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
void createStaticObjectList();
void drawFloorWithGrid();
void drawCrosshair();
void renderHead(float r, float g, float b);
void renderBody(float r, float g, float b);
void renderRightArm(float r, float g, float b);
void renderLeftArm(float r, float g, float b);
void renderRightLeg(float r, float g, float b);
void renderLeftLeg(float r, float g, float b);
void renderPlayer(float r, float g, float b);
std::vector<glm::vec3> calculateConvexHullQhull(const std::vector<glm::vec3>& points);
bool overlapOnAxis(const std::vector<glm::vec3>& hull1, const std::vector<glm::vec3>& hull2, const glm::vec3& axis);
bool checkCollisionConvexHull(const std::vector<glm::vec3>& hull1, const std::vector<glm::vec3>& hull2);
bool checkCollisionForPlayerParts(const CharaInfo* player, const std::vector<Objdata>& objects);
void createBulletEffect(float r, float g, float b);
void createBulletHole(float x, float y, float z, Uint32 duration);
void renderBulletHoles();
glm::vec3 raycastFromCamera(float maxDistance);
bool rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& hitPosition);
// window.cpp
void draw_background(GLuint texture);
void draw_3d_character();
void draw_2d_ui();
void display();
void handle_mouse_click(int button, int state, int x, int y); 
void selectWeapon();
/* end of system.h */
