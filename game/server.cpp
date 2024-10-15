#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>
#include "system.h"
#include "constants.h"

CLIENT clients[MAX_PLAYERS];     // クライアント情報
PlayerState players[MAX_PLAYERS]; // プレイヤーの状態を保存
int num_clients = 0;

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void send_data(int client_index, void* data, size_t size) {
    sendto(clients[client_index].sock, data, size, 0, (struct sockaddr*)&clients[client_index].addr, sizeof(clients[client_index].addr));
}

void setup_server(int num_cl, u_short port, int& sockfd) {
    struct sockaddr_in sv_addr, cl_addr;
    socklen_t len = sizeof(cl_addr);
    char buffer[BUFFER_SIZE];

    fprintf(stderr, "Server setup is started.\n");

    if (num_cl < 2) {
        fprintf(stderr, "At least 2 clients are required to start the game.\n");
        exit(1);
    }
    num_clients = num_cl;

    // UDPソケット作成
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        handle_error("socket()");
    }
    fprintf(stderr, "sock() for UDP socket is done successfully.\n");

    sv_addr.sin_family = AF_INET;
    sv_addr.sin_port = htons(port);
    sv_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
        handle_error("bind()");
    }
    fprintf(stderr, "bind() is done successfully.\n");

    char src[MAX_LEN_ADDR];

    // クライアントの受け入れ
    for (int i = 0; i < num_clients; i++) {
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cl_addr, &len);
        if (recv_len < 0) {
            handle_error("recvfrom()");
        }

        clients[i].cid = i;
        clients[i].sock = sockfd;
        clients[i].addr = cl_addr;
        memcpy(clients[i].name, buffer, recv_len);

        memset(src, 0, sizeof(src));
        inet_ntop(AF_INET, &cl_addr.sin_addr, src, sizeof(src));
        fprintf(stderr, "Client %d is accepted (name=%s, address=%s, port=%d).\n", i, clients[i].name, src, ntohs(cl_addr.sin_port));
    }

    // プレイヤーの初期座標をランダムに割り当て
    srand(time(NULL));
    float player_positions[MAX_PLAYERS][3] = {
        { 150.0f, 150.0f, 0.0f },
        { 72.0f, 178.0f, 0.0f },
        { 156.0f, 79.0f, 0.0f },
        { 78.0f, 62.0f, 0.0f }
    };
    std::random_shuffle(std::begin(player_positions), std::end(player_positions));

    // クライアントにデータを送信
    for (int i = 0; i < num_clients; i++) {
        sendto(clients[i].sock, &num_clients, sizeof(int), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
        sendto(clients[i].sock, &i, sizeof(int), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
        sendto(clients[i].sock, player_positions[i], sizeof(player_positions[i]), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));

        for (int j = 0; j < num_clients; j++) {
            sendto(clients[i].sock, &clients[j], sizeof(CLIENT), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
        }
    }

    fprintf(stderr, "Server setup is done.\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <num_clients> <port>" << std::endl;
        return 1;
    }

    int num_cl = atoi(argv[1]);
    u_short port = atoi(argv[2]);

    if (num_cl < 2 || num_cl > MAX_PLAYERS) {
        std::cerr << "Number of clients must be between 2 and " << MAX_PLAYERS << "." << std::endl;
        return 1;
    }

    int sockfd;
    setup_server(num_cl, port, sockfd);

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    while (true) {
        // データ受信
        int receivedBytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (receivedBytes > 0) {
            // クライアントのIDとプレイヤーの状態を受信
            int playerId;
            memcpy(&playerId, buffer, sizeof(int));

            if (playerId >= 0 && playerId < MAX_PLAYERS) {
                // プレイヤーの状態を更新
                memcpy(&players[playerId], buffer + sizeof(int), sizeof(PlayerState));
                players[playerId].connected = true;

                // クライアントアドレスを保存（初回接続時のみ）
                bool newClient = true;
                for (int i = 0; i < num_clients; ++i) {
                    if (clients[i].addr.sin_addr.s_addr == clientAddr.sin_addr.s_addr &&
                        clients[i].addr.sin_port == clientAddr.sin_port) {
                        newClient = false;
                        break;
                    }
                }

                if (newClient && num_clients < MAX_PLAYERS) {
                    clients[num_clients].addr = clientAddr;  // クライアントのアドレスを保存
                    num_clients++;
                    std::cout << "New client connected: " << num_clients << std::endl;
                }

                std::cout << "Player " << playerId << " moved to position ("
                          << players[playerId].x << ", " << players[playerId].y << ")" << std::endl;

                // 全クライアントにプレイヤー情報をブロードキャスト
                for (int i = 0; i < num_clients; ++i) {
                    sendto(sockfd, &players, sizeof(players), 0, (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
