#include <iostream>
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

    std::unique_ptr<ClientReader<Chunk> > reader(
        stub_->ReceiveFile(&context, request));

    while (reader->Read(&chunk)) {
      std::cout << chunk.content() << std::endl;
    }

    Status status = reader->Finish();
    if (status.ok()) {
      std::cout << "Success\n";
    } else {
      std::cout << "Fail :(\n";
    }
  }
 private:
  std::unique_ptr<FileTransfer::Stub> stub_;
};

int main() {
  FileTransferClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  std::string filename("file.txt");
  
  client.ReceiveFile(filename);

  return 0;
}