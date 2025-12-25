#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
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

	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(server_fd, 10) < 0) {
		perror("listen");
		return 1;
	}

	std::cout << "Server Listening on port " << PORT << "\n";

	while (true) {
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) 
		{
			perror("Accept");
			continue;
		}

		std::string buffer;
		char temp[1024];

		while (true) {
			ssize_t bytes = recv(client_fd, temp, sizeof(temp), 0);
			if (bytes <= 0) {
				break;
			}

			buffer.append(temp,bytes);
			size_t pos;
			while ((pos = buffer.find("\n")) != std::string::npos) {
				std::string line = buffer.substr(0,pos);
				buffer.erase(0,pos + 1);
				
				std::cout << "Recieved: "<< line << std::endl;

				const char* reply = "OK\n";
				send(client_fd,reply, strlen(reply),0);
			}
		}
	}

}
