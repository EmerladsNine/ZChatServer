#include "client_data.h"
#include "utils.h"
#include <algorithm>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

void my_send(ClientData client, std::vector<char> &buffer) {
  size_t totalSent = 0;
  while (totalSent < buffer.size()) {
    size_t n = send(client.fd, buffer.data(), buffer.size(), 0);
    if (n <= 0) {
      perror("send");
      exit(1);
    }
    totalSent += n;
  }
}

int main() {
  const int PORT = 9999;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr;
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
  std::vector<ClientData> clientsConnected;
  while (true) {
    fd_set readfds;    // The set of clients ready to be read.
    FD_ZERO(&readfds); // Clear all bits
    FD_SET(server_fd, &readfds);

    // get max fd
    int max_fd = server_fd;
    for (ClientData client : clientsConnected) {
      FD_SET(client.fd, &readfds);
      max_fd = std::max(max_fd, client.fd);
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
        ClientData client;
        client.fd = client_fd;
        client.expectedSize = -1;
        client.head = -1;
        clientsConnected.push_back(client);
      }
    }

    char temp[4096];
    std::vector<ClientData> disconnected;
    for (auto it = clientsConnected.begin(); it != clientsConnected.end();
         it++) {
      ClientData& client = *it;
      if (!FD_ISSET(client.fd, &readfds)) {
        continue;
      }
      std::cout << "a client is readable" << client.fd << std::endl;
      ssize_t n = recv(client.fd, temp, sizeof(temp), 0);
      std::cout << "recv size :" << n << std::endl;
      if (n <= 0) {
        std::cout << "Client disconnected : " << client.fd << "\n";
        close(client.fd);
        disconnected.push_back(client);
        continue;
      }

      if (client.head == -1) {
        client.head = temp[0];
        std::cout << "Head : " << client.head << std::endl;
      }

      // bodyless units
      switch (client.head) {
      case 0: {
        char pong = 1;
        std::cout << "received ping , sending pong" << std::endl;
        send(client.fd, &pong, 1, 0);
	client.head = -1;
        continue;
      }
      case 1: {
        std::cout << "received pong" << std::endl;
	client.head = -1;
        continue;
      }
      }

      client.buf.insert(client.buf.end(), temp, temp + n);
      if (client.expectedSize == -1) {
        if (client.buf.size() < 3) {
          continue;
        }
        client.expectedSize = readUint16FromBuffer(client.buf);
        std::cout << "Size :" << client.expectedSize << std::endl;
      }

      if (client.buf.size() < client.expectedSize)
        continue;

      switch (client.head) {
      case 2: {
        std::cout << "BroadCasting... from " << client.fd << std::endl;
        for (ClientData other : clientsConnected) {
          if (other.fd != client.fd) {
            my_send(other, client.buf);
            std::cout << "BroadCasting to " << other.fd << std::endl;
          }
        }
      }
      }
      client.buf.clear();
      client.expectedSize = -1;
      client.head = -1;
    }

    for (ClientData c : disconnected) {
      clientsConnected.erase(
          std::remove(clientsConnected.begin(), clientsConnected.end(), c),
          clientsConnected.end());
    }
  }
}
