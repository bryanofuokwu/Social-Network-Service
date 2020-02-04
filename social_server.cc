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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "social.grpc.pb.h"
using std::chrono::system_clock;

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

        // checking if the user exists.
        int fileread = open("user_data/users.txt", O_RDONLY);
        char buffer[MAX_DATA];
        ssize_t inlen;
        //std::cout << "who to follow : " <<  (frequest->to_follow()).length() << std::endl;
        while(inlen = read(fileread, buffer, (frequest->to_follow()).length()) > 0) {
            const char cstr[(frequest->to_follow()).length() + 1];
            strcpy(cstr, (frequest->to_follow()).c_str());
            // we check if user to follow is in the network
            if((strcmp(cstr, buffer)) == 0){
                social::SocialNetwork social_network;
                for (int i = 0; i < social_network.user_size(); i++) {
                    const social::User& user = social_network.user(i);
                    std::cout << "i am in the server with user: " << user.name() << std::endl;
                    if(user.name().compare((frequest->from_user()).name()) == 0) {
                        user.add_following_users(cstr);

                    }
                }
                close(fileread);
                return Status::OK;
            }
        }
        return Status::CANCELLED;


    }


   /* Status Unfollow(ServerContext* context, const UnfollowRequest* ufrequest,
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
    }*/

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