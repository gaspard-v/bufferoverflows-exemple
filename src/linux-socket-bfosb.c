#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 7777
#define BUFFER_SIZE 256

struct server_fd {
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen;
};

struct server_fd* create_serverfd(void);
struct server_fd* create_serverfd(void)
{
    static struct server_fd server_fd;
    int opt = 1;
    server_fd.addrlen = sizeof(server_fd.addr);
    // Creating socket file descriptor
    if ((server_fd.sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd.sockfd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    server_fd.addr.sin_family = AF_INET;
    server_fd.addr.sin_addr.s_addr = INADDR_ANY;
    server_fd.addr.sin_port = htons(PORT);
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd.sockfd, (struct sockaddr *)&(server_fd.addr),
             sizeof(server_fd.addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd.sockfd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return &server_fd;
}

int main(int argc, char *argv[])
{
    int new_socket = 0;
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread = 0;
    struct server_fd* server_fd = {0};

    printf("starting ECHO1.0 deamon\n");
    server_fd = create_serverfd();
    printf("Waiting for connections ...\n");
    if ((new_socket = accept(server_fd->sockfd, (struct sockaddr *)&(server_fd->addr),
                             &(server_fd->addrlen))) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        valread = read(new_socket, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);
    }
    return EXIT_SUCCESS;
}
