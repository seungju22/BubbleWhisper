#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    int clientId;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
    // 클라이언트 소켓 생성
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Client socket creation failed\n");
        return 1;
    }
    // 서버 주소
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP 주소
    serverAddr.sin_port = htons(PORT); // 서버 포트 번호

    // 서버에 연결
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Server connection failed\n");
        closesocket(clientSocket); 
        return 1;
    }

    printf("Connected to server\n");

    printf("Enter client ID (1 or 2): ");
    scanf("%d", &clientId); 
    getchar(); 

    // 메시지 송신
    while (1) {
        // 사용자로부터 메시지 입력 받기
        printf("Client %d: ", clientId);
        fgets(buffer, sizeof(buffer), stdin);
        send(clientSocket, buffer, strlen(buffer), 0); // 메시지 전송
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
