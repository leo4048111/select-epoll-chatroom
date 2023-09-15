#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char** argv) {
    int ret = 0;

    if(argc != 3) {
        printf("Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(ret);
    }

    int csock;

    if((ret = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Failed to create client socket...\n");
        exit(ret);
    }

    csock = ret;

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &saddr.sin_addr.s_addr);

    if((ret = connect(csock, (const sockaddr*)&saddr, sizeof(saddr))) == -1) {
        printf("Failed to connect to %s:%s\n", argv[1], argv[2]);
        exit(ret);
    }

    // init fd set
    fd_set readFds, tmpFds;
    FD_ZERO(&readFds);
    // add stdin to fd set
    FD_SET(0, &readFds);
    // add socket to fd set
    FD_SET(csock, &readFds);
    int maxFd = csock;
    bool end = false;
    while(!end) {
        tmpFds = readFds;
        ret = select(maxFd + 1, &tmpFds, nullptr, nullptr, nullptr);

        if(ret == -1) {
            printf("Error when calling select...\n");
            break;
        }

        if(ret == 0) continue;

        for(int i = 0; i <= maxFd; i++) {
            if(!FD_ISSET(i, &tmpFds)) continue;

            if(i == 0) {
                char buf[2048];
                scanf("%s", buf);
                ret = send(csock, buf, sizeof(buf), 0);
            }
            else if(i == csock) { // received message from server
                char buf[2048];
                ret = recv(csock, buf, sizeof(buf), 0);
                if(ret == 0) {
                    end = true;
                    break;
                }
                printf("%s\n", buf);
            }
        }
    }

    close(csock);
    return 0;
}