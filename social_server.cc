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
#include <sstream>
#include <iterator>
#include <map>

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
    Status Follow(ServerContext *context, const FollowRequest *frequest,
                  FollowReply *freply) override
    {

        // checking if the user exists.
        int fileread = open("user_data/users.txt", O_RDONLY);
        char buffer[MAX_DATA];
        ssize_t inlen;
        std::cout << "who to follow : " << (frequest->to_follow()) << std::endl;
        while (inlen = read(fileread, buffer, (frequest->to_follow()).length()) > 0)
        {
            // we want to make a char* of the string to follow
            char cstr[(frequest->to_follow()).length() + 1];
            strcpy(cstr, (frequest->to_follow()).c_str());
            if ((strcmp(cstr, buffer)) == 0)
            {
                close(fileread);
                users_followers[frequest->to_follow()].push_back(frequest->from_user());
                users_following[frequest->from_user()].push_back(frequest->to_follow());

                for (auto it = users_followers.begin(); it != users_followers.end(); ++it) {
                    for (auto follower : it->second) {
                    }
                    std::cout << std::endl;
                }
                return Status::OK;
            }
        }
        close(fileread);


        return Status::CANCELLED;
    }

    Status Unfollow(ServerContext *context, const UnfollowRequest *ufrequest,
                    UnfollowReply *ufreply) override
    {

        // checking if the user exists.
        int fileread = open("user_data/users.txt", O_RDONLY);
        char buffer[MAX_DATA];
        ssize_t inlen;
        std::cout << "who to unfollow : " << (ufrequest->to_unfollow()) << std::endl;
        while (inlen = read(fileread, buffer, (ufrequest->to_unfollow()).length()) > 0)
        {
            // we want to make a char* of the string to follow
            char cstr[(ufrequest->to_unfollow()).length() + 1];
            strcpy(cstr, (ufrequest->to_unfollow()).c_str());
            if ((strcmp(cstr, buffer)) == 0)
            {
                close(fileread);
                return Status::OK;
            }
        }
        close(fileread);
        return Status::CANCELLED;
    }


    Status List(ServerContext* context, const ListRequest* lrequest,
                  ListReply* lreply) override {
       std::string user = lrequest->from_user();
       std::string user_following = "users_following/";
       //std::cout<< "size in list user " << user.length() << std::endl;
       user_following.append(user);
       user_following.append("_following.txt");
       char *fname_following = new char[user_following.length() + 1];
       std::strcpy(fname_following, (user_following).c_str());
       int file_follow_read = open(fname_following, O_RDONLY);
       //std::cout<< "file_follow_read " << fname_following<< std::endl;

       char buffer[2];
       ssize_t inlen;

       std::string follow_users;
//       while(inlen = read(file_follow_read, buffer, 2) > 0) {
//           // we want to make a char* of the string to follow
//           std::cout<< "read buffer " << buffer ;
//           follow_users.append(buffer);
//           follow_users.append(",");
//       }
        for (auto it = users_following.begin(); it != users_following.end(); ++it) {
            if (it->first == user) {
                for (auto following : it->second) {
                    follow_users.append(following);
                    follow_users.append(",");
                }
            }
        }

        std::cout<< " " << std::endl;
        std::cout<< " follow_users " << follow_users ;

        lreply->set_following_users(follow_users);

        int file_all = open("user_data/users.txt", O_RDONLY);
        std::string net_users;

        while(inlen = read(file_all, buffer, 2) > 0) {
           net_users.append(buffer);
           net_users.append(",");
       }
       lreply->set_network_users(net_users);
       return Status::OK;
    }

    Status Timeline(ServerContext* context,
            ServerReaderWriter<PostReply, Post>* stream) override {

        Post p;

        while(stream->Read(&p)) {

            std::string msg = p.message();

            std::string user_timeline = "users_timeline/";
            std::string from_user = p.from_user();

            user_timeline.append(from_user);
            user_timeline.append("_timeline.txt");
            char *fname_timeline = new char[user_timeline.length() + 1];
            std::strcpy(fname_timeline, (user_timeline).c_str());

            if (msg.length() == 3){
                msg = msg.substr(0, 2);
                msg.append(" :");
            }
            else if (msg.length() == 4){
                msg = msg.substr(0, 3);
                msg.append(":");
            }
            char charTime[14];

            // writing to file system
            time_t seconds = p.post_timestamp().seconds();
            sprintf(charTime,"%d", seconds);
            std::stringstream ss;
            ss << seconds;
            std::string ts = ss.str();
            msg.append(ts);

            int fd_time = open(fname_timeline, O_WRONLY | O_CREAT | O_APPEND, 0666);
            char semi[MAX_DATA];
            memset(semi, 0, sizeof(semi));
            strcpy(semi, msg.c_str());
            size_t nbytes = msg.length();
            ssize_t write_bytes;
            write(fd_time, semi, strlen(semi));

            users_own_timeline[from_user].push_back(msg);
            std::cout << "adding to map:" << users_own_timeline[from_user] << " " <<   msg<< std::endl;

            if ( client_streams.find(p.from_user()) == client_streams.end() ) {
                std::cout << "need to add to stream map " << std::endl;
                client_streams.insert(std::make_pair(p.from_user(), stream));

                // TODO: check if the users it follows post sizes ARE above 20
                for (auto it = users_following.begin(); it != users_following.end(); ++it) {
                    if (it->first == p.from_user()){
                        for (auto following : it->second) {
                            auto stream_to_write_to = client_streams.find(p.from_user());
                            std::cout << it->first <<  " follows: "<< following << std::endl;
                            if (users_own_timeline[following].size() >=20){
                                int indexer = users_own_timeline[following].size();
                                // TODO: do a while loop instead of a for loop
                                int last_to_read = users_own_timeline[following].size() -20 ;
                                for (int i = indexer; i >= last_to_read ; i--){
                                    std::cout << users_own_timeline[following][indexer] << std::endl;
                                    PostReply post_reply;
                                    post_reply.set_message(users_own_timeline[following][indexer]);
                                    post_reply.set_time_date(ts);
                                    post_reply.set_author(following);
                                    if (stream_to_write_to != client_streams.end()) { // if exists
                                        stream_to_write_to->second->Write(post_reply);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }

            }

            PostReply post_reply;
            post_reply.set_message(p.message());
            post_reply.set_time_date(ts);
            post_reply.set_author(p.from_user());


            for (auto it = users_followers.begin(); it != users_followers.end(); ++it) {
                if (it->first == p.from_user()){
                    for (auto follower : it->second) {
                        auto stream_to_write_to = client_streams.find(follower);
                        if (stream_to_write_to != client_streams.end()) { // if exists
                            stream_to_write_to->second->Write(post_reply);
                        }
                    }
                    break;
                }
                std::cout << std::endl;
            }

            std::cout << "this is the user's post size: " << users_own_timeline[from_user].size() << std::endl;




        }


        return Status::OK;
    }


private:
    // used for follow and unfollow
    std::map<std::string, std::vector<std::string>> users_followers;
    std::map<std::string, std::vector<std::string>> users_following;
    // used for timelines
    //map of user to the posts of who it follows
    std::map<std::string, std::vector<std::string>> users_own_timeline;
    std::map<std::string, ServerReaderWriter<PostReply, Post>* > client_streams;
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