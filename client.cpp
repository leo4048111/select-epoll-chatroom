#include <iostream>
#include <cstring>
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

    char buf[2048];
    memset(buf, 0, sizeof(buf));
    ret = recv(csock, buf, sizeof(buf), 0);
    std::cout << buf << std::endl;

    return 0;
}