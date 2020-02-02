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
#include "social.grpc.pb.h"

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
        social::SocialNetwork social_network;
        for (int i = 0; i < social_network.user_size(); i++) {
            const social::User& user = social_network.user(i);
            if((user.name().compare(frequest->name())) == 0) {
                freply->set_status("SUCCESS");
                // will return the user just followed
                freply->set_reply(user.name());
                return Status::OK;
                }
            }
        freply->set_status("FAILURE_INVALID_USERNAME");
        return Status::OK;
        }


    Status Unfollow(ServerContext* context, const UnfollowRequest* ufrequest,
                  UnfollowReply* ufreply) override {
        social::SocialNetwork social_network;
        for (int i = 0; i < social_network.user_size(); i++) {
            const social::User& user = social_network.user(i);
            if((user.name().compare(ufrequest->name())) == 0) {
                ufreply->set_status("SUCCESS");
                ufreply->set_reply(user.name());
                return Status::OK;
            }
        }
        ufreply->set_status("FAILURE_INVALID_USERNAME");
        return Status::OK;
    }

    Status List(ServerContext* context, const ListRequest* lrequest,
                  ListReply* lreply) override {
        social::SocialNetwork social_network;
        for (int i = 0; i < lrequest->mutable_user()->following_users_size(); i++) {
            lreply->add_following_users(user);
        }
        freply->set_status("SUCCESS");
        return Status::OK;
    }

    }


};

void RunServer(port) {
    std::string server_address("localhost");
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
                p = atoi(optarg)
                break;
            default:
                std::cerr << "Invalid Command Line Argument\n";
        }
    }
    std::string server_address(p);
    RunServer(db);

    return 0;
}