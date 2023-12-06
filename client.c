#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555

void play_game(SOCKET client_socket) {
    char buffer[1024];
    int choice;

    while (1) {
        // Receive game instructions from the server
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("%s", buffer);

        // Get user input
        printf("Enter your choice (rock=1/paper=2/scissors=3, enter '0' to quit): ");
        scanf("%d", &choice);

        // Send user input to the server
        send(client_socket, (char*)&choice, sizeof(choice), 0);

        // // Check if the other player has quit
        // recv(client_socket, buffer, sizeof(buffer), 0);
        // if (strcmp(buffer, "Other player quit. You win!") == 0) {
        //     printf("%s\n", buffer);
        //     break;
        // }

        // Check if the user wants to quit
        if (choice == 0) {
            break;
        }

        // Receive the opponent's move from the server
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("%s\n", buffer);

        // Receive the game result from the server
        recv(client_socket, buffer, sizeof(buffer), 0);
        printf("%s\n", buffer);

        
    }
}

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock");
        return 1;
    }

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.101.6"); // Change this to the server's IP address
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed");
        return 1;
    }

    // Receive the initial message from the server
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    // Play the game
    play_game(client_socket);

    // Cleanup Winsock
    closesocket(client_socket);
    WSACleanup();

    return 0;
}


/*
cd /d d:
cd D:\Donna-sama\USJR\3RD YEAR\Net 1\SP_SocketProg
taskkill /im server.exe /t /f
gcc -o server.exe server.c -lws2_32
gcc -o client.exe client.c -lws2_32

*/