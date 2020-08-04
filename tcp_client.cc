#include <iostream>
#include <cstring>
#include <cassert>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BUFFER_SIZE = (1 << 10);

int main(int argc, char **argv) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  unsigned short port = 9002;

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  std::string ip_addr = argv[1];

  inet_aton(ip_addr.c_str(), &address.sin_addr);

  int status = connect(sock, (struct sockaddr *)&address, sizeof(address));
  assert(status != -1);

  char *buffer = new char[BUFFER_SIZE + 1];


  std::ofstream fout("tcp_output.txt", std::ofstream::binary);

  int total = 0, len = 4, r;
  while (total < len) {
    r = recv(sock, buffer, len - total, 0);
    total += r;
  }
  len = ntohl(*((int *)buffer));
  memset(buffer, 0, BUFFER_SIZE);
  total = 0;
  while (total < len) {
    int r = recv(sock, buffer, BUFFER_SIZE, 0);
    if (r < 0) {
      std::cout << total << "\n";
      std::cout << "Connection lost abruptly\n";
      fout.close();
      close(sock);
      return 0;
    }
    if(r == 0){
      break;
    }
    total += r;
    fout.write(buffer, r);
  }
  fout.close();
  std::cout << total << "\n";
  delete buffer;
  close(sock);
  return 0;
}