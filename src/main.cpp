#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include <variant>
#include <memory>

using namespace std;

constexpr int REDIS_PORT = 6379;
constexpr int BACKLOG = 5;
constexpr size_t BUFFER_SIZE = 1024;

void handle_client(int client_fd) {
    // Simple (code crafters) immediate solution
    char buffer[BUFFER_SIZE];
    while(true) {
        ssize_t num_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
        if (num_bytes <= 0) {
          cout << "Failed to read payload.\n" << endl;
          close(client_fd);
          return;
        }

        // convert buffer to istream
        string s(buffer, sizeof(buffer) - 1);
        istringstream iss(s);
        
        // call parser
        resp_parser(num_bytes, iss);

        string response = "+PONG\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}

int main(int argc, char **argv) {
    cout << "main\n";

    // Flush after every std::cout / std::cerr
    cout << unitbuf;
    cerr << unitbuf;
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
    cerr << "Failed to create server socket\n";
    return 1;
    }
    
    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
      cerr << "setsockopt failed\n";
      return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(REDIS_PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
      cerr << "Failed to bind to port 6379\n";
      return 1;
    }
    
    if (listen(server_fd, BACKLOG) != 0) {
      cerr << "listen failed\n";
      return 1;
    }
    
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while (true)
    {
        cout << "Waiting for a client to connect...\n";
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        cout << "Client connected\n";

        thread client_thread(handle_client, client_fd);
        client_thread.detach();
    }

    close(server_fd);

    return 0;
}