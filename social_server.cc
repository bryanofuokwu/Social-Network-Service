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
#include <vector>
#include <map>

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
using social::FollowReply;
using social::FollowRequest;
using social::ListReply;
using social::ListRequest;
using social::Post;
using social::PostReply;
using social::Social;
using social::SocialNetwork;
using social::TimelineRequest;
using social::UnfollowReply;
using social::UnfollowRequest;
using social::User;
using std::chrono::system_clock;

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "social.grpc.pb.h"

class SocialService final : public Social::Service
{
    // The client will invoke this server method and we need to send back if
    // we want to make sure that username exists
public:
    /*SocialService( std::map<std::string, std::vector<std::string> > user_followers_,
            std::map<std::string, std::vector<std::string> > user_following_posts_,
            std::map<std::string , std::vector<std::string>> user_own_post_)
            : user_followers(user_followers_), user_following_posts(user_following_posts_), user_own_post(user_own_post_){}
*/
    //    void createMaps(std::map<std::string, std::vector<std::string>> m1,  std::map<std::string, std::vector<std::string> > m4,
    //            std::map<std::string, std::vector<std::string> > m3){
    //        user_followers
    //    }
    Status Follow(ServerContext *context, const FollowRequest *frequest,
                  FollowReply *freply) override
    {

        // checking if the user exists.
        int fileread = open("user_data/users.txt", O_RDONLY);
        char buffer[MAX_DATA];
        ssize_t inlen;
        std::cout << "who to follow : " << (frequest->to_follow()).length() << std::endl;
        while (inlen = read(fileread, buffer, (frequest->to_follow()).length()) > 0)
        {
            // we want to make a char* of the string to follow
            char cstr[(frequest->to_follow()).length() + 1];
            strcpy(cstr, (frequest->to_follow()).c_str());
            if ((strcmp(cstr, buffer)) == 0)
            {
                close(fileread);
                return Status::OK;
            }
        }
        return Status::CANCELLED;
    }

    Status Unfollow(ServerContext *context, const UnfollowRequest *ufrequest,
                    UnfollowReply *ufreply) override
    {

        std::vector<string> followers;
        int fileread = open("user_data/users.txt", O_RDONLY);
        char buffer[MAX_DATA];
        ssize_t inlen;
        while (inlen = read(fileread, buffer, (ufrequest->to_unfollow()).length()) > 0)
        {
            // we want to make a char* of the string to follow
            char cstr[(ufrequest->to_unfollow()).length() + 1];
            strcpy(cstr, (ufrequest->to_unfollow()).c_str());
            if ((strcmp(cstr, buffer)) == 0)
            {
                return Status::OK;
            }

            if ((strcmp(cstr, buffer)) == 0)
            {
                continue;
            }
            else
            {
                followers.push_back(buffer);
                close(fileread);
            }
        }
        close(fileread);

        fileread = open("user_data/users.txt", O_TRUNC, 0666);
        close(fileread);

        for (int i = 0; i < followers.size(); ++i)
        {
            char buff[MAX_DATA];
            strcpy(buff, followers[i].c_str());
            fileread = open("user_data/users.txt", O_WRONLY);
            write(fileread, buff, (ufrequest->to_unfollow()).length());
            close(fileread);
        }

        return Status::CANCELLED;
    }

    /*Status List(ServerContext* context, const ListRequest* lrequest,
                  ListReply* lreply) override {
        social::SocialNetwork social_network;
        for (int i = 0; i < lrequest->user()->following_users_size(); i++) {
            lreply->add_following_users(user);
        }
        freply->set_status("SUCCESS");
        return Status::OK;
    }*/

private:
    // used for follow and unfollow
    //std::map<std::string, std::vector<std::string>> user_followers;
    // used for timelines
    //map of user to the posts of who it follows
    std::map<std::string, std::vector<std::string>> user_following_posts;
    //map of user to the posts of itself
    std::map<std::string, std::vector<std::string>> user_own_post;
};

void RunServer(std::string port)
{
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
int main(int argc, char **argv)
{
    std::string hostname = "localhost";
    std::string port = "3010";
    int p = 0;
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (opt)
        {
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

    std::cout << "After getting opt " << std::endl;
    RunServer(port);

    return 0;
}