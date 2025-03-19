/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blatifat <blatifat@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 03:30:27 by blatifat          #+#    #+#             */
/*   Updated: 2025/03/19 05:50:53 by blatifat         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <map>

#define PORT 8080
#define MAX_CLIENTS 10

// Structure to hold client information
struct Client {
    int fd;            // Client file descriptor (socket)
    int id;            // Unique client ID
};

int main() {
    int server_fd, client_fd, poll_count;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1; // Initially, we only have the server socket to monitor
    std::map<int, Client> clients; // Map to store client information using ID

    // Client ID counter
    int client_counter = 1;

    // 1. Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        return 1;
    }

    // 2. Bind to port 8080
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // 3. Start listening for clients
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        return 1;
    }

    // 4. Add server socket to poll array
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        // 5. Monitor sockets for activity using poll()
        poll_count = poll(fds, nfds, -1);  // -1 means wait forever
        if (poll_count == -1) {
            perror("Poll failed");
            return 1;
        }

        // 6. Check if the server socket is ready to accept a new client
        if (fds[0].revents & POLLIN) {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("Accept failed");
                continue;
            }

            // Assign a unique ID to the new client
            Client new_client = {client_fd, client_counter++};

            // Add new client to the clients map
            clients[new_client.id] = new_client;

            // Add new client socket to poll array
            fds[nfds].fd = client_fd;
            fds[nfds].events = POLLIN;
            nfds++;

            std::cout << "New client connected! ID: " << new_client.id << "\n";
        }

        // 7. Check if any client has sent data
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                char buffer[1024] = {0};
                int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                if (bytes_received <= 0) {
                    // Client disconnected or error occurred
                    std::cout << "Client disconnected!\n";
                    close(fds[i].fd);
                    clients.erase(fds[i].fd); // Remove client from map
                    fds[i] = fds[nfds - 1];  // Move the last client socket to this position
                    nfds--;
                } else {
                    // Identify the client that sent the message using its ID
                    std::cout << "Client ID " << clients[fds[i].fd].id << " says: " << buffer << "\n";

                    // Respond with a personalized message
                    std::string response = "Hello Client ID " + std::to_string(clients[fds[i].fd].id) + "!";
                    send(fds[i].fd, response.c_str(), response.length(), 0);
                }
            }
        }
    }

    // 8. Close server socket
    close(server_fd);
    return 0;
}
