#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <winsock2.h>

#define PORT 8080
#define MAX_CLIENTS 10

typedef struct {
    int clientSocket;
    int clientId;
} ThreadArg;

void *handle_client(void *arg) {
    ThreadArg *threadArg = (ThreadArg *)arg;
    int clientSocket = threadArg->clientSocket;
    int clientId = threadArg->clientId;
    char buffer[1024];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0';
        printf("Client %d: %s", clientId, buffer);
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    closesocket(clientSocket);
    printf("Client %d disconnected\n", clientId);
    free(threadArg);
    return NULL;
}

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    pthread_t threadId[MAX_CLIENTS];
    int clientCount = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Server socket creation failed\n");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Binding failed\n");
        closesocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(serverSocket);
        return 1;
    }

    printf("Chat server started. Waiting for clients on port %d...\n", PORT);

    while (1) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize)) == INVALID_SOCKET) {
            printf("Client connection acceptance failed\n");
            continue;
        }

        printf("Client connected\n");

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
            free(threadArg);
            closesocket(clientSocket);
        }

        clientCount++;

        if (clientCount >= MAX_CLIENTS) {
            printf("Maximum number of clients reached\n");
            break;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
