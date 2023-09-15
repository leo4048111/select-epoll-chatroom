#include <iostream>
#include <list>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const size_t MAX_EPOLL_EVENTS = 64;
static ::std::list<int> clientfds;
static const char* WELCOME_MSG = ""
".____                  __         _________ .__            __   \n"
"|    |    ____   _____/  |_  _____\\_   ___ \\|  |__ _____ _/  |_ \n"
"|    |  _/ __ \\_/ __ \\   __\\/  ___/    \\  \\/|  |  \\__  \\   __\\ \n"
"|    |__\\  ___/\\  ___/|  |  \\___ \\     \\___|   Y  \\/ __ \\|  | \n"
"|_______ \\___  >\\___  >__| /____  >\\______  /___|  (____  /__| \n"
"        \\/   \\/     \\/          \\/        \\/     \\/     \\/ \n";

static void broadcast(int ignoreClientfd, char* msg, size_t len) noexcept {
    int ret = 0;
    for(auto& clientfd : clientfds) {
        if(clientfd == ignoreClientfd) continue;

        ret = send(clientfd, msg, len, 0);
        if(ret == -1) {
            printf("Failed to send msg to client socket %d...", clientfd);
        }
    }
}

static void welcome(int clientfd) noexcept {
    int ret = send(clientfd, WELCOME_MSG, strlen(WELCOME_MSG), 0);
    if(ret == -1) {
        printf("Failed to send welcome message to client socket %d...", clientfd);
    }
}
 
int main(int argc, char** argv)
{
    int ret = 0;

    if(argc != 2) {
        printf("Usage: %s <Server Port>\n", argv[0]);
        exit(ret);
    }

    int ssock;

    if((ret = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Failed to create server socket...\n");
        exit(ret);
    }

    ssock = ret;

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[1]));
    saddr.sin_addr.s_addr = INADDR_ANY;

    if((ret = bind(ssock, (const sockaddr*)&saddr, sizeof(saddr))) == -1) {
        printf("Failed to bind server socket to port %s...\n", argv[1]);
        exit(ret);
    }

    if((ret = listen(ssock, 64)) == -1) {
        printf("Failed to start listening on ssock...\n");
        exit(ret);
    }

    printf("Server running on port %d...\n", atoi(argv[1]));

    // create epfd
    int epfd = epoll_create1(0);

    // add server fd to epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = ssock;

    epoll_ctl(epfd, EPOLL_CTL_ADD, ssock, &event);

    while(true) {
        struct epoll_event events[MAX_EPOLL_EVENTS];
        
        ret = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);

        if(ret == -1) {
            printf("Error when calling epoll_wait...\n");
            break;
        }

        if(ret == 0) continue;

        for(int i = 0; i < ret; i++) {
            auto e = events[i];

            if(e.data.fd == ssock) // new connection from client
            {
                struct sockaddr_in clientAddr;
                socklen_t len;
                int clientfd = accept(ssock, (sockaddr*)&clientAddr, &len);
                if(clientfd == -1) {
                    printf("Error when accepting new connection...\n");
                    break;
                }

                printf("New connection from %s...\n", inet_ntoa(clientAddr.sin_addr));
                event.data.fd = clientfd;
                event.events = EPOLLIN;

                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event);
                clientfds.push_back(clientfd);
                welcome(clientfd);
            }
            else { // data from client
                char buf[2048];
                memset(buf, 0, sizeof(buf));
                ret = recv(e.data.fd, buf, sizeof(buf), 0);
                if(ret == -1) {
                    printf("Error when receiving from %d...\n", e.data.fd);
                    break;
                }
                else if(ret == 0) // connection closed
                {
                    close(e.data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, e.data.fd, nullptr);
                    clientfds.remove(e.data.fd);
                }
                else {
                    printf("Received \"%s\" from %d...\n", buf, e.data.fd);
                    broadcast(e.data.fd, buf, ret);
                }
            }
        }
    }

    for(auto& clientfd : clientfds) close(clientfd);

    close(ssock);
}