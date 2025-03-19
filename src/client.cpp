#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }

    // 3. Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        return 1;
    }

    std::cout << "Connected to server!\n";

    // 4. Send message to server
    std::string message = "Hello Server!";
    send(sock, message.c_str(), message.length(), 0);
    std::cout << "Message sent to server\n";

    // 5. Receive server response
    int valread = recv(sock, buffer, sizeof(buffer), 0);
    std::cout << "Server: " << buffer << "\n";

    // 6. Close the socket
    close(sock);
    return 0;
}
