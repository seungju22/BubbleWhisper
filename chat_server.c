#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <winsock2.h>

#define PORT 8080
#define MAX_CLIENTS 10

//클라이언트 ID정의
typedef struct {
    int clientSocket; 
    int clientId;     // 클라이언트 ID
} ThreadArg;

// 클라이언트와 통신하는 함수
void *handle_client(void *arg) {
    ThreadArg *threadArg = (ThreadArg *)arg; 
    int clientSocket = threadArg->clientSocket; 
    int clientId = threadArg->clientId;         // 클라이언트 ID
    char buffer[1024]; // 메시지를 저장할 버퍼
    int bytesReceived; // 바이트 수

    // 통신 시작
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0';
        printf("Client %d: %s", clientId, buffer); // 클라이언트 메시지 출력
        send(clientSocket, buffer, strlen(buffer), 0); // 클라이언트에게 다시 메시지 전송
    }

    //종료
    closesocket(clientSocket);
    printf("Client %d disconnected\n", clientId); //종료 메시지

    // 동적으로 할당된 구조체 메모리 해제
    free(threadArg);
    return NULL; //종료
}

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket; // 서버 및 클라이언트 소켓
    struct sockaddr_in serverAddr, clientAddr; //주소 구조체
    int clientAddrSize = sizeof(clientAddr); 
    pthread_t threadId[MAX_CLIENTS]; 
    int clientCount = 0; // 클라이언트 수

    // 소켓 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // 서버 소켓 생성
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Server socket creation failed\n");
        return 1;
    }

    // 서버 주소 설정
    serverAddr.sin_family = AF_INET; // IPv4 주소 체계
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 모든 네트워크 인터페이스로부터 접속 허용
    serverAddr.sin_port = htons(PORT); // 지정된 포트로 연결 대기

    // 서버 소켓에 주소
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Binding failed\n");
        closesocket(serverSocket);
        return 1;
    }

    // 클라이언트 접속 대기 상태
    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(serverSocket);
        return 1;
    }
    printf("Chat server started. Waiting for clients on port %d...\n", PORT);
    // 클라이언트 연결
    while (1) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize)) == INVALID_SOCKET){
            printf("Client connection acceptance failed\n");
            continue;
        }
        printf("Client connected\n");

        // 클라이언트 정보를 담은 구조체 할당
        ThreadArg *threadArg = (ThreadArg *)malloc(sizeof(ThreadArg));
        if (threadArg == NULL) {
            printf("Memory allocation failed\n");
            closesocket(clientSocket);
            continue;
        }
        threadArg->clientSocket = clientSocket;
        threadArg->clientId = clientCount + 1;

        if (pthread_create(&threadId[clientCount], NULL, handle_client, (void *)threadArg) != 0) {
            printf("Client thread creation failed\n");
            free(threadArg); //생성 실패 시 할당 해제
            closesocket(clientSocket);
        }
        clientCount++; // 클라이언트 수 증가
        // 최대 클라이언트 수에 도달하면 더 이상 접속을 받지 않음
        if (clientCount >= MAX_CLIENTS) {
            printf("Maximum number of clients reached\n");
            break;
        }
    }
    // 서버 닫기
    closesocket(serverSocket);

    // 윈도우 소켓 종료
    WSACleanup();

    return 0;
}

