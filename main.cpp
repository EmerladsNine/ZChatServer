#include <asm-generic/socket.h>
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

	std::cout << "Server Listening on port " << PORT << std::endl;

	while (true) {
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) 
		{
			perror("accept");
			continue;
		}
	}

}
