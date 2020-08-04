#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "filetransfer.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::Status;
using grpc::StatusCode;
using grpc::ServerWriter;
using grpc::ServerContext;
using filetransfer::FileTransfer;
using filetransfer::Chunk;
using filetransfer::FileRequest;

const int BUFFER_SIZE = 1024;

class FileTransferServiceImpl final : public FileTransfer::Service {
  Status ReceiveFile(ServerContext* context, const FileRequest* request,
                    ServerWriter<Chunk>* writer) override {
    //std::cout << request->filename() << "\n";
    std::string filename = request->filename();
    std::ifstream fin(filename.c_str(), std::ifstream::binary);
    if(fin.fail()){
      return Status(StatusCode::UNKNOWN, "File doesn't exist.");
    }
    char* buffer = new char[BUFFER_SIZE];
    Chunk chunk;

    while(!fin.eof()){
      fin.read(buffer, BUFFER_SIZE);
      chunk.set_content(buffer);
      writer->Write(chunk);  
    }
    fin.close();
    return Status::OK;
  }
};

void RunServer(){
  std::string server_address("0.0.0.0:50051");
  FileTransferServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  
  server->Wait();
}

int main(int argc, char **argv){ 
  RunServer();
  return 0;
}