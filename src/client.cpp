#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

constexpr int REDIS_PORT = 6379;
constexpr size_t BUFFER_SIZE = 1024;

int client(int argc, char** argv) {
  cout << unitbuf;

  const char* server_ip = (argc > 1) ? argv[1] : "127.0.0.1";

  // client connecting to server
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(REDIS_PORT);
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr); // Convert IP string to network address
  
  // client connecting to server
  connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

  // send ping
  const char msg[] = "+PING\r\n";
  send(client_fd, msg, sizeof(msg) - 1, 0);
  
  // recieve PONG
  char buffer[BUFFER_SIZE];
  ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  if (n < 0) {
    cerr << "recv() failed\n";
  } else if (n > 0) {
    buffer[n] = '\0';
    cout << buffer << "\n";
  } else {
    cout << "Client recv returned\n";
  }

  // close socket
  close(client_fd);

  return 0;
}