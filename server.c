#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 1024

char* play_round(int client_choice) {
    int server_choice = rand() % 3;

    if (client_choice == server_choice) {
        return "It's a tie!";
    } else if (
        (client_choice == 0 && server_choice == 2) ||
        (client_choice == 1 && server_choice == 0) ||
        (client_choice == 2 && server_choice == 1)
    ) {
        return "You win!";
    } else {
        return "You lose!";
    }
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

    int client_choice;
    char buffer[MAX_BUFFER_SIZE];

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
            client_choice = atoi(buffer);

            // Play a round and send the result to the client
            char* result = play_round(client_choice);
            send(client_socket[i], result, strlen(result), 0);
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
