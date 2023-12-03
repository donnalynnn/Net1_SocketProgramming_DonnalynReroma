#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define MAX_BUFFER_SIZE 1024

typedef struct {
    char name[50];
    int choice;
} Player;

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error initializing Winsock");
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error connecting to server");
        return 1;
    }

    Player player;

    while (1) {
        printf("Enter your name (or '0' to exit): ");
        scanf("%s", player.name);

        if (player.name[0] == '0') {
            // If '0' is entered, break out of the loop
            break;
        }

        send(client_socket, player.name, strlen(player.name), 0);

        int choice;
        printf("Enter your choice, %s (1 for Rock, 2 for Paper, 3 for Scissors): ", player.name);
        scanf("%d", &choice);

        player.choice = choice;
        send(client_socket, (const char*)&player.choice, sizeof(int), 0);

        // Receive confirmation from the server
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Ensure null-termination
            if (strcmp(buffer, "OK") != 0) {
                fprintf(stderr, "Unexpected confirmation from server: %s\n", buffer);
                closesocket(client_socket);
                WSACleanup();
                return 1;
            }
        } else {
            perror("Error receiving confirmation from server");
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }

        // Clear the input buffer
        fflush(stdin);

        // Receive the result from the server
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Ensure null-termination
            printf("Result from server: %s\n", buffer);
        } else {
            perror("Error receiving result from server");
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
    }

    // Close the socket
    closesocket(client_socket);
    WSACleanup();
    return 0;
}

/*
cd /d d:
cd D:\Donna-sama\USJR\3RD YEAR\Net 1\SP_SocketProg
gcc -o server.exe server.c -lws2_32
gcc -o client.exe client.c -lws2_32
*/