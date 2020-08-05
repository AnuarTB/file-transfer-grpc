#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "filetransfer.grpc.pb.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::steady_clock;

using filetransfer::Chunk;
using filetransfer::FileRequest;
using filetransfer::FileTransfer;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

steady_clock::time_point start, finish;
double total_time = 0.0;

class FileTransferClient {
 public:
  FileTransferClient(std::shared_ptr<Channel> channel)
      : stub_(FileTransfer::NewStub(channel)) {}

  void ReceiveFile(const std::string &filename) {
    FileRequest request;
    Chunk chunk;
    ClientContext context;

    request.set_filename(filename);

    std::ofstream fout("grpc_output.txt", std::ofstream::binary);

    std::unique_ptr<ClientReader<Chunk>> reader(
        stub_->ReceiveFile(&context, request));
    bool boo;
    while (1) {
      start = steady_clock::now();
      boo = reader->Read(&chunk);
      finish = steady_clock::now();
      total_time += duration_cast<duration<double>>(finish - start).count();
      if (!boo) break;
      fout.write(chunk.content().c_str(), chunk.content().length());
    }
    fout.close();

    Status status = reader->Finish();
    if (status.ok()) {
      std::cout << "Success" << std::endl;
    } else {
      std::cout << "Fail :(" << std::endl;
      std::cout << status.error_message() << std::endl;
    }
  }

 private:
  std::unique_ptr<FileTransfer::Stub> stub_;
};

int main(int argc, char **argv) {
  std::string address = argv[1];
  address += ":50051";
  FileTransferClient client(
      grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
  std::string filename = "file.txt";

  client.ReceiveFile(filename);

  std::cout << "Total time of stream read: ";
  std::cout << std::fixed << std::setprecision(4) << total_time << std::endl;
  return 0;
}