#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "social.grpc.pb.h"

#define MAX_DATA 256

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using social::User;
using social::FollowRequest;
using social::FollowReply;
using social::Post;
using social::PostReply;
using social::UnfollowRequest;
using social::UnfollowReply;
using social::TimelineRequest;
using social::ListRequest;
using social::ListReply;
using social::SocialNetwork;
using social::Social;
using std::chrono::system_clock;

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "social.grpc.pb.h"



class SocialService final : public Social::Service {
    // The client will invoke this server method and we need to send back if
    // we want to make sure that username exists
    Status Follow(ServerContext* context, const FollowRequest* frequest,
            FollowReply* freply) override {
        /*social::SocialNetwork social_network;
        for (int i = 0; i < social_network.user_size(); i++) {
            const social::User& user = social_network.user(i);
            if((user.name().compare(frequest->to_follow())) == 0) {
                freply->set_status("SUCCESS");
                // will return the user just followed
                freply->set_reply(user.name());
                return Status::OK;
                }
            }
        freply->set_status("FAILURE_INVALID_USERNAME");
        return Status::OK;*/

        int fileread = open("user_data/users.txt", O_RDONLY);
        char buffer[MAX_DATA];
        ssize_t inlen;
        while(inlen = read(fileread, buffer, sizeof(buffer)) > 0) {
            std::cout << buffer << std::endl;
            char cstr[frequest->to_follow().size() + 1];
            strcpy(cstr, (frequest->to_follow()).c_str());
            std::cout << cstr << std::endl;
            if((strcmp(cstr, buffer))== 0){
                std::cout << "they are the same!" << std::endl;
            }
        }
        close(fileread);
        return Status::OK;

    }


    Status Unfollow(ServerContext* context, const UnfollowRequest* ufrequest,
                  UnfollowReply* ufreply) override {
        social::SocialNetwork social_network;
        for (int i = 0; i < social_network.user_size(); i++) {
            const social::User& user = social_network.user(i);
            if((user.name().compare(ufrequest->to_unfollow())) == 0) {
                ufreply->set_status("SUCCESS");
                ufreply->set_reply(user.name());
                return Status::OK;
            }
        }
        ufreply->set_status("FAILURE_INVALID_USERNAME");
        return Status::OK;
    }

//    Status List(ServerContext* context, const ListRequest* lrequest,
//                  ListReply* lreply) override {
//        social::SocialNetwork social_network;
//        for (int i = 0; i < lrequest->user()->following_users_size(); i++) {
//            lreply->add_following_users(user);
//        }
//        freply->set_status("SUCCESS");
//        return Status::OK;
//    }

};

void RunServer(std::string port) {
    std::string host = "localhost:";
    std::string s_addr = host.append("3010");
    std::string server_address(s_addr);
    SocialService service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}
int main(int argc, char** argv) {
    std::string hostname = "localhost";
    std::string port = "3010";
    int p = 0;
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:p:")) != -1){
        switch(opt) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                std::cerr << "Invalid Command Line Argument\n";
        }
    }
    std::cout<< "After getting opt " << std::endl;
    RunServer(port);

    return 0;
}