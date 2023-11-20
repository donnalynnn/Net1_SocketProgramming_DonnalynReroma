#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 1024

char* play_round(int client_choice1, int client_choice2) {
    static char resultBuffer[256];  // Static buffer to store the result

    if (client_choice1 == client_choice2) {
        snprintf(resultBuffer, sizeof(resultBuffer), "It's a tie!");
    } else if (
        (client_choice1 == 0 && client_choice2 == 2) ||
        (client_choice1 == 1 && client_choice2 == 0) ||
        (client_choice1 == 2 && client_choice2 == 1)
    ) {
        snprintf(resultBuffer, sizeof(resultBuffer), "Client 1 wins!");
    } else {
        snprintf(resultBuffer, sizeof(resultBuffer), "Client 2 wins!");
    }

    return resultBuffer;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket[2];
    struct sockaddr_in server_addr, client_addr;
    int addr_size = sizeof(client_addr);
    int connected_clients = 0;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error initializing Winsock");
        return 1;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Error creating socket");
        return 1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error binding socket");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 2) == SOCKET_ERROR) {  // Allow up to 2 connections
        perror("Error listening for connections");
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept connections from two clients
    while (connected_clients < 2) {
        client_socket[connected_clients] = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_socket[connected_clients] == INVALID_SOCKET) {
            perror("Error accepting connection");
            return 1;
        }

        printf("Connected by %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        connected_clients++;
    }

    int client_choice[2];
    int choices_received = 0;  // Track the number of clients who have submitted their choices
    char buffer[MAX_BUFFER_SIZE];  // Declare buffer to store received data

    while (1) {
        for (int i = 0; i < connected_clients; i++) {
            // Receive client's choice
            int bytes_received = recv(client_socket[i], buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break;
            }

            buffer[bytes_received] = '\0';

            // Check for exit command
            if (strcmp(buffer, "exit") == 0) {
                // Send confirmation and exit
                send(client_socket[i], "exit", strlen("exit"), 0);
                printf("Client %d has requested to exit. Confirming...\n", i + 1);
                connected_clients--;
                closesocket(client_socket[i]);
                continue;
            }

            // Convert choice to integer
            client_choice[i] = atoi(buffer);
            choices_received++;
        }

        if (choices_received == connected_clients) {
            // Broadcast choices to both clients
            send(client_socket[0], (const char*)&client_choice[1], sizeof(int), 0);
            send(client_socket[1], (const char*)&client_choice[0], sizeof(int), 0);

            // Print broadcasted choices
            printf("Broadcasted choice to Client 1: %d\n", client_choice[1]);
            printf("Broadcasted choice to Client 2: %d\n", client_choice[0]);

            choices_received = 0;  // Reset the count for the next round

            // Play a round and send the result to both clients
            char* result = play_round(client_choice[0], client_choice[1]);
            send(client_socket[0], result, strlen(result), 0);
            send(client_socket[1], result, strlen(result), 0);
        }

        if (connected_clients == 0) {
            // All clients have exited, break the loop
            break;
        }
    }

    // Close the server socket
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
