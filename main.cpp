#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

int main() {
  const int PORT = 9999;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(server_fd, 10) < 0) {
    perror("listen");
    return 1;
  }

  std::cout << "Server Listening on port " << PORT << "\n";
  std::vector<int> clientsConnected;
  while (true) {
    fd_set readfds;    // The set of clients ready to be read.
    FD_ZERO(&readfds); // Clear all bits
    FD_SET(server_fd, &readfds);

    // get max fd
    int max_fd = server_fd;
    for (int c : clientsConnected) {
      FD_SET(c, &readfds);
      max_fd = std::max(max_fd, c);
    }

    // waits until at least one socket is readable and removes every not
    // readable socket from readfds
    std::cout << "waiting ..." << std::endl;
    if (select(max_fd + 1, &readfds, nullptr, nullptr, nullptr) < 0) {
      perror("select");
      break;
    }
    std::cout << "a socket is readable" << std::endl;

    // if server socket is readable then there is a new client connects
    if (FD_ISSET(server_fd, &readfds)) {
      int client_fd = accept(server_fd, nullptr, nullptr);
      if (client_fd >= 0) {
        std::cout << "New Client Connected : " << client_fd << std::endl;
        clientsConnected.push_back(client_fd);
      }
    }

    char buffer[1024];
    for (auto it = clientsConnected.begin(); it != clientsConnected.end();it++) {
      int client = *it;
      std::cout << "Checking Client :" << client << std::endl;
      if (FD_ISSET(client, &readfds)) {
        std::cout << "a client is readable" << client << std::endl;
        ssize_t n = recv(client, buffer, sizeof(buffer), 0);
        if (n <= 0) {
          std::cout << "Client disconnected : " << client << "\n";
          close(client);
          it = clientsConnected.erase(it);
          continue;
        }

        std::cout << "BroadCasting... from " << client << std::endl;
        // Broadcast to all other clients
        for (int other : clientsConnected) {
          if (other != client) {
            send(other, buffer, n, 0);
            std::cout << "BroadCasting to " << other << std::endl;
          }
        }
      }
    }
  }
}
