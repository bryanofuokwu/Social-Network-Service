#include <iostream>
//#include <memory>
//#include <thread>
//#include <vector>
#include <string>
#include <unistd.h>
#include <grpc++/grpc++.h>
#include "client.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <grpc++/grpc++.h>
#include <sstream>

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <cstring>
#include <string>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "social.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
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
using social::TimeStamp;
using social::UnfollowReply;
using social::UnfollowRequest;
using social::User;

using namespace std;
string trim(string input)
{
    int i = 0;
    while (i < input.size() && input[i] == ' ')
        i++;
    if (i < input.size())
        input = input.substr(i);
    else
    {
        return "";
    }

    i = input.size() - 1;
    while (i >= 0 && input[i] == ' ')
        i--;
    if (i >= 0)
        input = input.substr(0, i + 1);
    else
        return "";

    return input;
}
vector<string> split(string line, string separator = " ")
{
    vector<string> result;
    while (line.size())
    {
        size_t found = line.find(separator);
        size_t fquote = line.find('\"');
        size_t fq = line.find('\'');
        if (found == string::npos)
        {
            string lastpart = trim(line);
            if (lastpart.at(0) == '\"')
            {
                lastpart = lastpart.substr(1, lastpart.size() - 2);
            }
            if (lastpart.at(0) == '\'')
            {
                lastpart = lastpart.substr(1, lastpart.size() - 2);
            }
            if (lastpart.size() > 0)
            {
                result.push_back(lastpart);
            }
            //cout << "last result push back: " << lastpart << endl;
            break;
        }
        string segment = trim(line.substr(0, found));
        if ((fquote != string::npos) && (fquote < found))
        {
            size_t bquote = line.find('\"', fquote + 1);
            if (separator == " ")
            {
                segment = line.substr(1, bquote - 1);
            }
            // if quotes are found goes through here
            else if (found < bquote && (separator == "|"))
            {
                segment = trim(line);
                if (segment.at(0) == '\"')
                {
                    segment = segment.substr(1, segment.size() - 2);
                }
                if (segment.size() > 0)
                {
                    result.push_back(segment);
                }
                break;
            }
            else
            {
                segment = line.substr(0, bquote + 1);
            }
            line = line.substr(bquote + 1);
            result.push_back(segment);
        }
        // if single quotes are found goes through here
        else if ((fq != string::npos) && (fq < found) && (separator != "|"))
        {
            size_t bq = line.find('\'', fq + 1);
            if (separator == " ")
            {
                segment = line.substr(1, bq - 1);
            }
            else
            {
                segment = line.substr(0, bq + 1);
            }
            line = line.substr(bq + 1);
            result.push_back(segment);
        }
        else
        {
            if (segment.size() != 0)
            {
                result.push_back(segment);
                //cout << "segment: " << segment << endl;
            }
            line = line.substr(found + 1);
        }
    }
    return result;
}

class Client : public IClient
{
public:
    Client(const std::string &hname,
           const std::string &uname,
           const std::string &p, std::shared_ptr<Channel> channel)
        : hostname(hname), username(uname), port(p), stub_(Social::NewStub(channel)) {}

    //    Client(std::shared_ptr<Channel> channel)
    //        : stub_(Social::NewStub(channel)) {}

    std::string get_user() { return username; }
    string Follow(string user_to_follow, IReply *reply, string from_user)
    {
        FollowRequest followreq; // data sending to the server
        FollowReply followreply; // data recieving from the server
        followreq.set_to_follow(user_to_follow);
        followreq.set_from_user(get_user());
        ::google::protobuf::Timestamp *timestamp = new ::google::protobuf::Timestamp();

        timestamp->set_seconds(time(NULL));
        timestamp->set_nanos(0);
        followreq.set_allocated_fr_timestamp(timestamp);

        ClientContext context;

        Status status = stub_->Follow(&context, followreq, &followreply);

        if (status.ok())
        {
            reply->grpc_status = Status::OK;
            ////////////////////////// ERROR CHECKING //////////////////////////////
            if (user_to_follow == from_user)
            {
                return "FAILURE_INVALID_USERNAME";
            }
            ////////////////////////////////////////////////////////////////////

            else
            {
                std::string user_following = "users_following/";
                user_following.append(from_user);
                user_following.append("_following.txt");
                char *fname_f = new char[user_following.length() + 1];
                std::strcpy(fname_f, user_following.c_str());
                char buff[MAX_DATA];
                memset(buff, 0, sizeof(buff));
                //user_to_follow.append(" ");

                // handle the file to put in
                if (user_to_follow.length() <= 2)
                {
                    user_to_follow.append(" :");
                }
                else
                {
                    user_to_follow.append(":");
                }
                std::stringstream ss;
                ss << followreq.fr_timestamp().seconds();
                std::string ts = ss.str();
                user_to_follow.append(ts);

                std::strcpy(buff, user_to_follow.c_str());
                int filewrite = open(fname_f, O_WRONLY | O_APPEND);
                write(filewrite, buff, user_to_follow.length());
                return "SUCCESS";
            }
        }
        else
        {
            return "FAILURE_INVALID_USERNAME";
        }
    }
    string Unfollow(string user_to_unfollow, IReply *reply, string from_user)
    {
        UnfollowRequest unfollowreq; // data sending to the server
        UnfollowReply unfollowreply; // data recieving from the server
        unfollowreq.set_to_unfollow(user_to_unfollow);
        unfollowreq.set_from_user(from_user);

        ClientContext context;

        Status status = stub_->Unfollow(&context, unfollowreq, &unfollowreply);

        if (status.ok())
        {
            reply->grpc_status = Status::OK;
            ////////////////////////// ERROR CHECKING //////////////////////////////
            if (user_to_unfollow == from_user)
            {
                return "FAILURE_INVALID_USERNAME";
            }
            ////////////////////////////////////////////////////////////////////
            std::vector<string> followers;
            std::string user_following = "users_following/";
            user_following.append(from_user);
            user_following.append("_following.txt");
            char *fname_f = new char[user_following.length() + 1];
            std::strcpy(fname_f, user_following.c_str());
            char buffer[MAX_DATA];
            memset(buffer, 0, sizeof(buffer));
            int fileread = open(fname_f, O_RDONLY);
            ssize_t inlen;
            while (inlen = read(fileread, buffer, sizeof(buffer)) > 0)
            {
                std::string s = "";
                for (int i = 0; i < sizeof(buffer); i++)
                {
                    s = s + buffer[i];
                }
                int pos = s.find(user_to_unfollow);
                std::string unf_str = s.substr(pos, 14);
                char cstr[unf_str.length() + 1];
                std::strcpy(cstr, unf_str.c_str());
                if ((strcmp(cstr, buffer)) == 0)
                {
                    continue;
                }
                else
                {
                    followers.push_back(buffer);
                }
                close(fileread);
            }
            close(fileread);
            fileread = open(fname_f, O_TRUNC, 0666);
            close(fileread);

            for (int i = 0; i < followers.size(); ++i)
            {
                char buff[MAX_DATA];
                strcpy(buff, followers[i].c_str());
                fileread = open(fname_f, O_WRONLY);
                write(fileread, buff, user_to_unfollow.length());
                close(fileread);
            }
            return "SUCCESS";
        }
        else
        {
            return "FAILURE_INVALID_USERNAME";
        }
    }
    string List(string from_user, IReply *reply)
    {
        ListRequest listreq; // data sending to the server
        ListReply listreply; // data recieving from the server
        listreq.set_from_user(from_user);

        ClientContext context;

        Status status = stub_->List(&context, listreq, &listreply);
        if (status.ok())
        {
            reply->grpc_status = Status::OK;
            cout << "this is my list reply " << std::endl;

            vector<string> split_fusers = split(listreply.following_users(), ",");
            vector<string> split_nusers = split(listreply.network_users(), ",");
            reply->following_users = split_fusers;
            reply->all_users = split_nusers;
            return "SUCCESS";
        }
        else
        {
            reply->grpc_status = Status::CANCELLED;
            return "FAILURE";
        }
    }

    void Timeline(string user)
    {
        ClientContext context;
        std::shared_ptr<ClientReaderWriter<Post, PostReply>> stream(
            stub_->Timeline(&context));

        Post preq;
        // first write will just be the user who wrote this
        preq.set_from_user(get_user());
        stream->Write(preq);
        std::thread writer([stream, user]() {
            while (1)
            {
                Post preq;
                std::string message = getPostMessage();
                preq.set_from_user(user);
                preq.set_message(message);
                ::google::protobuf::Timestamp *timestamp = new ::google::protobuf::Timestamp();
                timestamp->set_seconds(time(NULL));
                timestamp->set_nanos(0);
                preq.set_allocated_post_timestamp(timestamp);
                stream->Write(preq);
            }
            stream->WritesDone();
        });

        std::thread reader([stream]() {
            PostReply preply;
            while (stream->Read(&preply))
            {
                // parse string
                // sender is a string
                std::string sender = preply.author();
                // message is a string
                std::string message = preply.message();
                // covert time
                const char *time;
                time = preply.time_date().c_str();
                time_t t;
                t = (time_t)atoll(time);

                displayPostMessage(sender, message, t);
            }
        });

        //Wait for the threads to finish
        writer.join();
        reader.join();
    }

protected:
    virtual int connectTo();
    virtual IReply processCommand(std::string &input);
    virtual void processTimeline();

private:
    std::string hostname;
    std::string username;
    std::string port;
    // You can have an instance of the client stub as a member variable.
    std::unique_ptr<Social::Stub> stub_;
};

Client *myc;
int main(int argc, char **argv)
{

    std::string hostname = "localhost";
    std::string username = "default";
    std::string port = "3010";
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:u:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            hostname = optarg;
            break;
        case 'u':
            username = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        default:
            std::cerr << "Invalid Command Line Argument\n";
        }
    }

    std::string file = "user_data/";

    // for adding username to file name
    std::string fileu = file.append(username);

    // for appending to the user.txt file
    std::string users = "user_data/users.txt";

    char *fname_user = new char[users.length() + 1];
    std::strcpy(fname_user, (users).c_str());
    int fd_user = open(fname_user, O_RDWR | O_CREAT | O_APPEND, 0666);

    char semi[MAX_DATA];
    memset(semi, 0, sizeof(semi));
    read(fd_user, semi, MAX_DATA);
    if(!(strstr(semi,username.c_str())))
    {
        char buff[MAX_DATA];
        memset(buff, 0, sizeof(buff));
        strcpy(buff, username.c_str());
        write(fd_user, buff, strlen(buff));
        close(fd_user);
    }





    // creating following directory
    std::string file_2 = "users_following/";
    std::string filef = file_2.append(username);

    // creating timeline directory
    std::string file_3 = "users_timeline/";
    std::string filet = file_3.append(username);

    std::string file_4 = "users_followers/";
    std::string filer = file_4.append(username);

    // for appending to the user.txt file
    std::string file_timeline = filet.append("_timeline");
    std::string file_following = filef.append("_following");
    std::string file_followers = filer.append("_followers");

    //adding the txt file extenstion
    std::string file_timeline_txt = file_timeline.append(".txt");
    std::string file_following_txt = file_following.append(".txt");
    std::string file_followers_txt = file_followers.append(".txt");

    //creating the file name
    char *fname_timeline = new char[file_timeline_txt.length() + 1];
    char *fname_following = new char[file_following_txt.length() + 1];
    char *fname_followers = new char[file_followers_txt.length() + 1];


    std::strcpy(fname_timeline, (file_timeline_txt).c_str());
    std::strcpy(fname_following, (file_following_txt).c_str());
    std::strcpy(fname_followers, (file_followers_txt).c_str());

    int fd_follower = open(fname_followers, O_WRONLY | O_CREAT | O_APPEND, 0666);
    char buff_follower[MAX_DATA];
    memset(buff_follower, 0, sizeof(buff_follower));
    strcpy(buff_follower, username.c_str());
    write(fd_follower, buff_follower, strlen(buff_follower));
    close(fd_follower);

    // for creating to the user_timeline.txt file
    int fd_time = open(fname_timeline, O_WRONLY | O_CREAT | O_APPEND, 0666);
    close(fd_time);

    // for creating to the user_following.txt file
    int fd_follow = open(fname_following, O_WRONLY | O_CREAT | O_APPEND, 0666);
    std::string follow_self;
    follow_self.append(username);
    if (username.length() == 2)
    {
        follow_self.append(" :0000000000");
    }
    else
    {
        follow_self.append(":0000000000");
    }

    char follow_self_buff[MAX_DATA];
    memset(follow_self_buff, 0, sizeof(follow_self_buff));
    strcpy(follow_self_buff, follow_self.c_str());
    write(fd_follow, follow_self_buff, strlen(follow_self_buff));
    close(fd_follow);

    social::SocialNetwork socialNetwork;
    User *user = socialNetwork.add_user();
    user->set_name(username);
    std::string server;
    server.append(hostname);
    server.append(":");
    server.append(port);
    myc = new Client(hostname, username, port, grpc::CreateChannel(server, grpc::InsecureChannelCredentials()));
    IReply ire;
    myc->Follow(username, &ire, username);
    myc->run_client();
    return 0;
}

int Client::connectTo()
{
    // ------------------------------------------------------------
    // In this function, you are supposed to create a stub so that
    // you call service methods in the processCommand/porcessTimeline
    // functions. That is, the stub should be accessible when you want
    // to call any service methods in those functions.
    // I recommend you to have the stub as
    // a member variable in your own Client class.
    // Please refer to gRpc tutorial how to create a stub.
    // ------------------------------------------------------------
    //Client client(grpc::CreateChannel("localhost:3010", grpc::InsecureChannelCredentials()));
    return 1; // return 1 if success, otherwise return -1
}

IReply Client::processCommand(std::string &input)
{

    vector<string> command = split(input);
    IReply ire;
    std::string response;
    //Client client(grpc::CreateChannel("localhost:3010", grpc::InsecureChannelCredentials()));

    // TODO: figure out how we want to handle what we receive from the server.
    if (command[0] == "FOLLOW")
    {
        response = myc->Follow(command[1], &ire, myc->get_user());
    }

    else if (command[0] == "UNFOLLOW")
    {
        response = myc->Unfollow(command[1], &ire, myc->get_user());
    }

    else if (command[0] == "LIST")
    {
        std::string user = this->get_user();
        response = myc->List(user, &ire);
    }

    // ------------------------------------------------------------
    // GUIDE 2:
    // Then, you should create a variable of IReply structure
    // provided by the client.h and initialize it according to
    // the result. Finally you can finish this function by returning
    // the IReply.
    // ------------------------------------------------------------

    if (response == "SUCCESS")
    {
        ire.comm_status = IStatus::SUCCESS;
    }
    else if (response == "FAILURE_ALREADY_EXISTS")
    {
        ire.comm_status = IStatus::FAILURE_ALREADY_EXISTS;
    }
    else if (response == "FAILURE_NOT_EXISTS")
    {
        ire.comm_status = IStatus::FAILURE_NOT_EXISTS;
    }
    else if (response == "FAILURE_INVALID_USERNAME")
    {
        ire.comm_status = IStatus::FAILURE_INVALID_USERNAME;
    }
    else if (response == "FAILURE_UNKNOWN")
    {
        ire.comm_status = IStatus::FAILURE_UNKNOWN;
    }

    // ------------------------------------------------------------
    // HINT: How to set the IReply?
    // Suppose you have "Join" service method for JOIN command,
    // IReply can be set as follow:==]-00
    //
    //     // some codes for creating/initializing parameters for
    //     // service method
    //     IReply ire;
    //     grpc::Status status = stub_->Join(&context, /* some parameters */);
    //     ire.grpc_status = status;
    //     if (status.ok()) {
    //         ire.comm_status = SUCCESS;
    //     } else {
    //         ire.comm_status = FAILURE_NOT_EXISTS;
    //     }
    //
    //      return ire;
    //
    // IMPORTANT:
    // For the command "LIST", you should set both "all_users" and
    // "following_users" member variable of IReply.
    // ------------------------------------------------------------

    //////////////////////
    return ire;
}

void Client::processTimeline()
{
    // ------------------------------------------------------------
    // In this function, you are supposed to get into timeline mode.
    // You may need to call a service method to communicate with
    // the server. Use getPostMessage/displayPostMessage functions
    // for both getting and displaying messages in timeline mode.
    // You should use them as you did in hw1.
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // IMPORTANT NOTICE:
    //
    // Once a user enter to timeline mode , there is no way
    // to command mode. You don't have to worry about this situation,
    // and you can terminate the client program by pressing
    // CTRL-C (SIGINT)
    // ------------------------------------------------------------

    // we handle the while loop inside this timeline function
    myc->Timeline(myc->get_user());
}
