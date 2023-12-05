// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 5555

// Enumeration for game choices
enum GameChoice {
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3
};

// Function to determine the game result
int determine_game_result(int choice1, int choice2) {
    if (choice1 == choice2) {
        return 0; // Draw
    } else if ((choice1 == ROCK && choice2 == SCISSORS) ||
               (choice1 == PAPER && choice2 == ROCK) ||
               (choice1 == SCISSORS && choice2 == PAPER)) {
        return 1; // Player 1 wins
    } else {
        return 2; // Player 2 wins
    }
}

// Function to handle a single game between two players
void play_game(SOCKET player1_socket, SOCKET player2_socket) {
    char buffer[1024];
    int player1_choice, player2_choice;
    int result;

    // Inform both players that the game is starting
    send(player1_socket, "Both players are connected. The game is starting!", sizeof(buffer), 0);
    send(player2_socket, "Both players are connected. The game is starting!", sizeof(buffer), 0);

    // Game loop
    while (1) {
        int dataRev=0;
        int bytesReceived;
        // Get Player 1's choice
        send(player1_socket, "Make your move (rock=1/paper=2/scissors=3, enter '0' to quit):", sizeof(buffer), 0);
        recv(player1_socket, buffer, sizeof(buffer), 0);

        bytesReceived = recv(player1_socket, buffer, sizeof(buffer), 0);
        if (bytesReceived >= 0 && bytesReceived <= 3)
            dataRev=0;
        // Check if Player 1 wants to quit
        player1_choice = atoi(buffer);
        if (player1_choice == 0) {
            break;
        }

        // Get Player 2's choice
        send(player2_socket, "Make your move (rock=1/paper=2/scissors=3, enter '0' to quit):", sizeof(buffer), 0);
        recv(player2_socket, buffer, sizeof(buffer), 0);

        bytesReceived = recv(player2_socket, buffer, sizeof(buffer), 0);
        if (bytesReceived >= 0 && bytesReceived <= 3)
            dataRev=1;

        // Check if Player 2 wants to quit
        player2_choice = atoi(buffer);
        if (player2_choice == 0) {
            break;
        }

        send(player1_socket, (char*)&dataRev, sizeof(dataRev), 0);
        send(player2_socket, (char*)&dataRev, sizeof(dataRev), 0);

        if(dataRev!=0){
        // Send choices to players
        sprintf(buffer, "Player 1 chose: %d", player1_choice);
        send(player1_socket, buffer, sizeof(buffer), 0);

        sprintf(buffer, "Player 2 chose: %d", player2_choice);
        send(player2_socket, buffer, sizeof(buffer), 0);

        // Determine the result and send it to both players
        result = determine_game_result(player1_choice, player2_choice);
        if (result == 0) {
            send(player1_socket, "It's a draw!", sizeof(buffer), 0);
            send(player2_socket, "It's a draw!", sizeof(buffer), 0);
        } else if (result == 1) {
            send(player1_socket, "Player 1 wins!", sizeof(buffer), 0);
            send(player2_socket, "Player 1 wins!", sizeof(buffer), 0);
        } else {
            send(player1_socket, "Player 2 wins!", sizeof(buffer), 0);
            send(player2_socket, "Player 2 wins!", sizeof(buffer), 0);
        }

        }else{
            send(player1_socket, "Wait for the other player...", sizeof(buffer), 0);
            send(player2_socket, "Wait for the other player...", sizeof(buffer), 0);
        }
    }
}


// Function to handle a single client connection
void handle_client(SOCKET client_socket, int player_number) {
    char buffer[1024];

    // Inform the player about their number
    sprintf(buffer, "You are Player %d. Waiting for the other player to connect...", player_number);
    send(client_socket, buffer, sizeof(buffer), 0);

    // Wait for the other player to connect
    if (player_number == 1) {
        send(client_socket, "Waiting for Player 2 to connect...", sizeof(buffer), 0);
    } else {
        send(client_socket, "Waiting for Player 1 to make a move...", sizeof(buffer), 0);
    }

    // Accept the other player's connection
    SOCKET other_player_socket;
    struct sockaddr_in other_player_addr;
    int other_player_len = sizeof(other_player_addr);

    other_player_socket = accept(client_socket, (struct sockaddr*)&other_player_addr, &other_player_len);
    printf("Player %d connected\n", 3 - player_number);

    // Inform both players that the game is starting
    sprintf(buffer, "Both players are connected. The game is starting!");
    send(client_socket, buffer, sizeof(buffer), 0);
    send(other_player_socket, buffer, sizeof(buffer), 0);

    // Play the game
    if (player_number == 1) {
        play_game(client_socket, other_player_socket);
    } else {
        play_game(other_player_socket, client_socket);
    }

    // Close the connection for both players
    closesocket(client_socket);
    closesocket(other_player_socket);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket;
    struct sockaddr_in server_addr;

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
    char buffer[1024];
    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed");
        return 1;
    }

    // Listen to incoming connections
    listen(server_socket, 2);

    printf("Server listening on port %d\n", PORT);

    // Accept connections for Player 1 and Player 2
    SOCKET player1_socket, player2_socket;
    struct sockaddr_in player1_addr, player2_addr;
    int player1_len = sizeof(player1_addr);
    int player2_len = sizeof(player2_addr);

    int player1_connected = 0;
    int player2_connected = 0;

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
    sprintf(buffer, "Player 2 has joined. The game is starting!");
    send(player1_socket, buffer, sizeof(buffer), 0);

    // Set flags to indicate both players are connected
    player1_connected = 1;
    player2_connected = 1;

    // Handle the game for both players
    if (player1_connected && player2_connected) {
        handle_client(player1_socket, 1);
        handle_client(player2_socket, 2);
    }

    // Cleanup Winsock
    closesocket(server_socket);
    WSACleanup();

    return 0;
}