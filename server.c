#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#define PORT 8888
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 2

typedef struct {
    char name[50];
    int choice;
} Player;

char* play_round(Player player1, Player player2) {
    static char resultBuffer[256];

    if (player1.choice == player2.choice) {
        snprintf(resultBuffer, sizeof(resultBuffer), "It's a tie!");
    } else if (
        (player1.choice == 1 && player2.choice == 3) ||
        (player1.choice == 2 && player2.choice == 1) ||
        (player1.choice == 3 && player2.choice == 2)
    ) {
        snprintf(resultBuffer, sizeof(resultBuffer), "%s wins!", player1.name);
    } else {
        snprintf(resultBuffer, sizeof(resultBuffer), "%s wins!", player2.name);
    }

    return resultBuffer;
}

unsigned int __stdcall handle_client(void* client_socket_ptr) {
    SOCKET client_socket = *((SOCKET*)client_socket_ptr);

    Player player;

    int bytes_received = recv(client_socket, player.name, sizeof(player.name), 0);
    if (bytes_received <= 0) {
        perror("Error receiving player name from client");
        closesocket(client_socket);
        _endthreadex(0);
    }

    player.name[bytes_received] = '\0';

    bytes_received = recv(client_socket, (char*)&player.choice, sizeof(int), 0);
    if (bytes_received <= 0) {
        perror("Error receiving player choice from client");
        closesocket(client_socket);
        _endthreadex(0);
    }

    send(client_socket, "OK", strlen("OK"), 0);

    // Print broadcasted choices using printf
    printf("Broadcasted choice to %s: %d\n", player.name, player.choice);

    // Send the player's choice to the other client
    send(client_socket, (const char*)&player.choice, sizeof(int), 0);

    _endthreadex(0);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    int addr_size = sizeof(client_addr);
    int connected_clients = 0;
    HANDLE thread_handles[MAX_CLIENTS];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("Error initializing Winsock");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        perror("Error listening for connections");
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept connections from clients in a loop until 0 is pressed
    while (1) {
        client_socket[connected_clients] = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_socket[connected_clients] == INVALID_SOCKET) {
            perror("Error accepting connection");
            continue;
        }

        printf("Connected by %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Start a new thread to handle the client
        thread_handles[connected_clients] = (HANDLE)_beginthreadex(NULL, 0, &handle_client, &client_socket[connected_clients], 0, NULL);

        // Increment the number of connected clients
        connected_clients++;

        // Check if maximum clients reached or continue accepting
        if (connected_clients >= MAX_CLIENTS) {
            printf("Maximum clients reached. Press 0 to exit.\n");

            // Wait for all threads to finish
            WaitForMultipleObjects(connected_clients, thread_handles, TRUE, INFINITE);

            // Close the sockets
            for (int i = 0; i < MAX_CLIENTS; i++) {
                closesocket(client_socket[i]);
            }

            connected_clients = 0;

            // Check if '0' is pressed to exit the loop
            char input;
            scanf(" %c", &input);
            if (input == '0') {
                break;
            }
        }
    }

    // Close the server socket
    closesocket(server_socket);

    // Cleanup and close Winsock
    WSACleanup();

    return 0;
}
