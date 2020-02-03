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


#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "social.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using social::FollowReply;
using social::FollowRequest;
using social::ListReply;
using social::ListRequest;
using social::PostReply;
using social::Social;
using social::SocialNetwork;
using social::TimelineRequest;
using social::UnfollowReply;
using social::UnfollowRequest;
using social::User;

using namespace std;
class Client : public IClient
{
public:
    Client(const std::string &hname,
           const std::string &uname,
           const std::string &p)
        : hostname(hname), username(uname), port(p){}
    Client(shared_ptr<Channel> channel)
        : stub_(Social::NewStub(channel)) {}
    string Follow(string &user)
    {
        FollowRequest followreq;  // data sending to the server
        FollowReply followreply; // data recieving from the server

        /* TODO: update the current user's following text file
         * The reply already has the user name it just followed.
        */
        ClientContext context;

        Status status = stub_->Follow(&context, followreq, &followreply);

        if (status.ok())
        {
            return "SUCCESS";
        }
        else
        {
            return "FAILURE";
        }
    }
    string Unfollow(string &user)
    {
        UnfollowRequest unfollowreq;  // data sending to the server
        UnfollowReply unfollowreply; // data recieving from the server

        ClientContext context;
        /* TODO: update the current user's following text file
        * The reply already has the user name it just unfollowed.
       */

        Status status = stub_->Unfollow(&context, unfollowreq, &unfollowreply);

        // TODO: figure out what other cases we will get
        // TODO: look at https://github.com/grpc/grpc/blob/master/doc/statuscodes.md
        // for all the kinds of status we can receive from the server
        if (status.ok())
        {
            return "SUCCESS";
        }
        else
        {
            return "FAILURE";
        }
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

int main(int argc, char **argv)
{

    std::string hostname = "localhost";
    std::string username = "default";
    std::string port = "3010";
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:u:p:")) != -1)
    {
        switch (opt){
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

    //TODO: create three files here for users.txt, user_timeline.txt, user_followers.txt
    std::string file = "user_data/";

    // for adding username to file name
    std::string fileu = file.append(username);


    // for appending to the user.txt file
    std::string users = "user_data/users.txt";
    char *fname_user = new char[users.length() + 1];
    std::strcpy(fname_user, (users).c_str());
//    if (fd_user = open(fname_user,O_WRONLY | O_CREAT | O_TRUNC,0666) < 0){
//        perror("Problem in opening the file");
//        exit(1);
//    };

    int fd_user = open(fname_user,O_WRONLY | O_CREAT| O_TRUNC,0666);
    char buff[MAX_DATA];
    strcpy(buff, username.c_str());
    size_t nbytes = username.length();
    ssize_t write_bytes;
    //TODO: FIX THIS FIRST BEFORE MOVING ON. WONT WRITE TO USERS.TXT FILE
    //write_bytes  = write(fd_user, "u1", 5);
    //lseek(fd_user,0, SEEK_SET);
    write(fd_user, "hello",5);
    close(fd_user);
    /*
     // adding timeline or followers
    std::string file_timeline = fileu.append("_timeline");
    std::string file_following = fileu.append("_following");

    //adding the txt file extenstion
    std::string file_following_txt = file_timeline.append(".txt");
    std::string file_timeline_txt = file_following.append(".txt");

    //creating the file name
    char *fname_timeline = new char[file_timeline_txt.length() + 1];
    char *fname_following = new char[file_following_txt.length() + 1];

    std::strcpy(fname_timeline, (file_timeline_txt).c_str());
    std::strcpy(fname_following, (file_following_txt).c_str());
    // for creating to the user_timeline.txt file
    int fd_time;
    if (fd_time = open(fname_timeline,O_RDWR | O_CREAT | O_APPEND,S_IRWXU) < 0){
        perror("Problem in opening the file for timeline");
        exit(1);
    };

    // for creating to the user_following.txt file
    int fd_follow;
    if (fd_follow = open(fname_following,O_RDWR | O_CREAT | O_APPEND,S_IRWXU) < 0){
        perror("Problem in opening the file for following");
        exit(1);
    };
*/

    //Client myc(hostname, username, port);
    // You MUST invoke "run_client" function to start business logic
    //myc.run_client();
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

    Client client(grpc::CreateChannel("localhost:3010", grpc::InsecureChannelCredentials()));

    return 1; // return 1 if success, otherwise return -1
}

IReply Client::processCommand(std::string &input)
{
    // ------------------------------------------------------------
    // GUIDE 1:
    // In this function, you are supposed to parse the given input
    // command and create your own message so that you call an
    // appropriate service method. The input command will be one
    // of the followings:
    //
    // FOLLOW <username>
    // UNFOLLOW <username>
    // LIST
    // TIMELINE
    //
    // - JOIN/LEAVE and "<username>" are separated by one space.
    // ------------------------------------------------------------

    vector<string> command = split(input);

    // TODO: figure out how we want to handle what we receive from the server.
    if (command[0] == "FOLLOW")
    {
        string response = this->Follow(command[1]);
    }
    else if (command[0] == "UNFOLLOW")
    {
        string response = this->Unfollow(command[1]);
    }
    //TODO: comment this out once unfollow and follow work perfectly
//    else if (command[0] == "LIST")
//    {
//        string response = social.List();
//    }
//    else if (command[0] == "TIMELINE")
//    {
//        string response = social.Timeline();
//    }

    // ------------------------------------------------------------
    // GUIDE 2:
    // Then, you should create a variable of IReply structure
    // provided by the client.h and initialize it according to
    // the result. Finally you can finish this function by returning
    // the IReply.
    // ------------------------------------------------------------

    /* if (response == "SUCCESS")
    {
        enum IStatus comm_status = SUCCESS;
    }
    else if (response == "FAILURE_ALREADY_EXISTS")
    {
        enum IStatus comm_status = FAILURE_ALREADY_EXISTS;
    }
    else if (response == "FAILURE_NOT_EXISTS")
    {
        enum IStatus comm_status = FAILURE_NOT_EXISTS;
    }
    else if (response == "FAILURE_INVALID")
    {

        enum IStatus comm_status = FAILURE_INVALID;
    }
    else if (response == "FAILURE_UNKNOWN")
    {
        enum IStatus comm_status = FAILURE_UNKNOWN;
    }
    */

    IReply ire;

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
}

//class SocialClient
//{
//public:
//    SocialClient(shared_ptr<channel> channel)
//        : stub_(SocialClient::NewStub(channel)) {}
//
//
//}