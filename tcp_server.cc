#include <iostream>
#include <cstring>
#include <fstream>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int BUFFER_SIZE = (1 << 15);

int main() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock != 0);
  unsigned short port = 10001;

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;

  assert(bind(sock, (struct sockaddr*)&address, sizeof(address)) != -1);

  assert(listen(sock, 100) != -1);

  int client_sock = accept(sock, NULL, NULL);

  assert(client_sock >= 0);

  std::ifstream fin("file.txt", std::ifstream::binary);

  if (fin.fail()) {
    std::cout << "File doesn't exist" << std::endl;
    return 0;
  }
  char* buffer = new char[BUFFER_SIZE];
  fin.seekg(0, fin.end);
  uint32_t length = fin.tellg();
  fin.seekg(0, fin.beg);

  //std::cout << length << "\n";
  std::cout << "Listening on port " << port << "\n";
  char* tmp = new char[4];
  *((int*)tmp) = htonl(length);
  int total = 0, len = 4;
  while (total < len) {
    total += send(client_sock, tmp, len - total, 0);
  }

  while (!fin.eof()) {
    fin.read(buffer, BUFFER_SIZE);
    len = strlen(buffer);
    total = 0;
    while (total < len) {
      total += send(client_sock, buffer, len - total, 0);
    }
  }
  close(client_sock);
  close(sock);
  return 0;
}