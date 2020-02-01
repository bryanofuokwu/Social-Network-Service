#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "helper.h"
#include "route_guide.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;




void RunServer(server_address, port) {
    GreeterServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart(), &port);
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}
int main(int argc, char** argv) {
    std::string hostname = "localhost";
    std::string username = "default";
    std::string port = "3010";
    int p = 0;
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:u:p:")) != -1){
        switch(opt) {
            case 'h':
                hostname = optarg;
                break;
            case 'u':
                username = optarg;
                break;
            case 'p':
                port = optarg;
                p = atoi(optarg)
                break;
            default:
                std::cerr << "Invalid Command Line Argument\n";
        }
    }
    std::string server_address(hostname, p);
    RunServer(db);

    return 0;
}