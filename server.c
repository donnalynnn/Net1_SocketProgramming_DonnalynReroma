#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555

const char* get_choice_name(int choice) {
    switch (choice) {
        case 1:
            return "ROCK";
        case 2:
            return "PAPER";
        case 3:
            return "SCISSORS";
        case 0:
            return "QUIT";
        default:
            return "UNKNOWN";
    }
}

void play_game(SOCKET player1_socket, SOCKET player2_socket) {
    char buffer[1024];
    int player1_choice, player2_choice;
    int player1_score = 0, player2_score = 0;

    while (1) {
        // Send game instructions to both players
        sprintf(buffer, "ROCK=1\nPAPER=2\nSCISSORS=3\n'0' to quit\n");
        send(player1_socket, buffer, sizeof(buffer), 0);
        send(player2_socket, buffer, sizeof(buffer), 0);

        // Receive and send choices for both players
        recv(player1_socket, (char*)&player1_choice, sizeof(player1_choice), 0);
        recv(player2_socket, (char*)&player2_choice, sizeof(player2_choice), 0);

        // // Check if either player wants to quit
        // if (player1_choice == 0 || player2_choice == 0) {
        //     if (player1_choice == 0) {
        //         sprintf(buffer, "Other player quit. You win!");
        //         send(player2_socket, buffer, sizeof(buffer), 0);
        //     } else {
        //         sprintf(buffer, "Other player quit. You win!");
        //         send(player1_socket, buffer, sizeof(buffer), 0);
        //     }
        //     break;
        // }

        // Send player2's choice to player1 and vice versa
        sprintf(buffer, "You chose: %s", get_choice_name(player1_choice));
        send(player1_socket, buffer, sizeof(buffer), 0);
        sprintf(buffer, "Player 2 chose: %s", get_choice_name(player2_choice));
        send(player1_socket, buffer, sizeof(buffer), 0);

        sprintf(buffer, "You chose: %s", get_choice_name(player2_choice));
        send(player2_socket, buffer, sizeof(buffer), 0);
        sprintf(buffer, "Player 1 chose: %s", get_choice_name(player1_choice));
        send(player2_socket, buffer, sizeof(buffer), 0);

        // Determine game result and send it to both players
        int result;
        if (player1_choice == player2_choice) {
            result = 0; // Tie
        } else if ((player1_choice == 1 && player2_choice == 3) ||
                   (player1_choice == 2 && player2_choice == 1) ||
                   (player1_choice == 3 && player2_choice == 2)) {
            result = 1; // Player 1 wins
            player1_score++;
        } else {
            result = 2; // Player 2 wins
            player2_score++;
        }

        // Send the result to both players
        sprintf(buffer, "Game result: %s.  |  Scores - Player 1: %d, Player 2: %d\n==============================================================\n", 
                (result == 0) ? "Tie" : (result == 1) ? "Player 1 wins" : "Player 2 wins",
                player1_score, player2_score);
        send(player1_socket, buffer, sizeof(buffer), 0);
        send(player2_socket, buffer, sizeof(buffer), 0);

        // Check if the players want to quit
        if (player1_choice == 0 || player2_choice == 0) {
            break;
        }
    }
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, player1_socket, player2_socket;
    struct sockaddr_in server_addr, player1_addr, player2_addr;
    int player1_len, player2_len;
    char buffer[1024];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock");
        return 1;
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed");
        return 1;
    }

    // Listen
    listen(server_socket, 2);

    printf("Server listening on port %d...\n", PORT);

    // Accept Player 1
    player1_socket = accept(server_socket, (struct sockaddr*)&player1_addr, &player1_len);
    printf("Player 1 connected\n");

    // Inform Player 1 to wait for Player 2
    sprintf(buffer, "You are Player 1. Waiting for Player 2 to connect...");
    send(player1_socket, buffer, sizeof(buffer), 0);

    // Accept Player 2
    player2_socket = accept(server_socket, (struct sockaddr*)&player2_addr, &player2_len);
    printf("Player 2 connected\n");

    // Inform Player 2 to wait for Player 1 and inform Player 1 that the game is starting
    sprintf(buffer, "You are Player 2. Waiting for Player 1 to make a move...");
    send(player2_socket, buffer, sizeof(buffer), 0);

    sprintf(buffer, "The game is starting!\n");
    send(player1_socket, buffer, sizeof(buffer), 0);
    sprintf(buffer, "The game is starting!\n");
    send(player2_socket, buffer, sizeof(buffer), 0);

    // Set flags to indicate both players are connected
    int player1_connected = 1;
    int player2_connected = 1;

    // Handle the game for both players
    if (player1_connected && player2_connected) {
        play_game(player1_socket, player2_socket);
    }

    // Cleanup Winsock
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
