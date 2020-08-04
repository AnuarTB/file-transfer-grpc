#include <iostream>
#include <cstring>
#include <fstream>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const int BUFFER_SIZE = (1 << 10);

int main() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock != 0);
  unsigned short port = 9002;

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;

  assert(bind(sock, (struct sockaddr*)&address, sizeof(address)) != -1);

  assert(listen(sock, 100) != -1);
  std::cout << "Listening on port " << port << "\n";
  char* buffer = new char[BUFFER_SIZE + 1];
  memset(buffer, 0, BUFFER_SIZE + 1);

  while (1) {
    int client_sock = accept(sock, NULL, NULL);

    assert(client_sock >= 0);

    std::ifstream fin("file.txt", std::ifstream::binary);

    if (fin.fail()) {
      std::cout << "File doesn't exist" << std::endl;
      return 0;
    }
    
    fin.seekg(0, fin.end);
    uint32_t length = fin.tellg();
    fin.seekg(0, fin.beg);

    char* tmp = new char[4];
    *((int*)tmp) = htonl(length);
    int total = 0, len = 4;
    int s;
    bool boo = true;
    while (total < len) {
      s = send(client_sock, tmp, len - total, 0);
      if(s == -1){
        std::cout << "Hmm\n";
        boo = false;
        break;
      }
      total += s;
    }
    if(!boo)
      continue;
    while (!fin.eof()) {
      len = fin.read(buffer, BUFFER_SIZE).gcount();
      total = 0;
      while (total < len) {
        s = send(client_sock, buffer, len - total, 0);
        if(s == -1){
          std::cout << "SO WHAT NOW?\n";
        }
        total += s;
      }
    }
    close(client_sock);
    fin.close();
  }
  delete buffer;
  close(sock);
  return 0;
}