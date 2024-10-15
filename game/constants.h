#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define DEF_PORT 50100
#define MAX_LEN_NAMES 10
#define MAX_PLAYERS 4
#define BUFFER_SIZE 512
#define MAX_LEN_ADDR 32
#define BROADCAST -1

struct PlayerState {
    float x, y, z;  // プレイヤーの位置
    float rotation; // プレイヤーの回転角度
    bool connected;
};

typedef struct {
  int cid;
  int sock;
  struct sockaddr_in addr;
  char name[MAX_LEN_NAMES];
} CLIENT;

typedef struct {
  int cid;
  char command;
  char message[BUFFER_SIZE];
} CONTAINER;
