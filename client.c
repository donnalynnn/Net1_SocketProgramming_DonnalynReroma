#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error initializing Winsock");
        return 1;
    }

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        perror("Error creating socket");
        return 1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error connecting to server");
        return 1;
    }

    printf("Welcome to Rock, Paper, Scissors!\n");
    printf("Enter 0 for rock, 1 for paper, 2 for scissors.\n");
    printf("Type \"exit\" to end the game.\n");

    char buffer[MAX_BUFFER_SIZE];

    while (1) {
        // Get user's choice
        printf("Enter your choice: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character

        // Send user's choice to the server
        send(client_socket, buffer, strlen(buffer), 0);

        // Check for exit command
        if (strcmp(buffer, "exit") == 0) {
            // Receive confirmation and exit
            recv(client_socket, buffer, sizeof(buffer), 0);
            printf("%s\n", buffer);
            printf("Exiting the game.\n");
            break;
        }

        // Receive and print the result from the server
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }

    // Close the connection
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
