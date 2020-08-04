#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "filetransfer.grpc.pb.h"

using filetransfer::Chunk;
using filetransfer::FileRequest;
using filetransfer::FileTransfer;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

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

    std::unique_ptr<ClientReader<Chunk> > reader(
        stub_->ReceiveFile(&context, request));
  
    while (reader->Read(&chunk)) {
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
  FileTransferClient client(grpc::CreateChannel(
      address, grpc::InsecureChannelCredentials()));
  std::string filename = "file.txt";

  client.ReceiveFile(filename);

  return 0;
}