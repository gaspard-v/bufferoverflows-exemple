#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <winsock2.h>
#include <signal.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 30
#define PORT 7777
#define BUFFER_SIZE 1536
#define OLD_BUFFER_SIZE 512

#ifdef DEBUG
#define PRINT_D(...) printf(__VA_ARGS__)
#else
#define PRINT_D(...)
#endif

#ifndef UNICODE
#define UNICODE
#endif

#define perror(x) error_message(x)

struct server_fd
{
    SOCKET sockfd;
    struct sockaddr_in addr;
    int addrlen;
};

static unsigned char keep_running = 1;

static struct server_fd *create_serverfd(void);

static int print_output(struct sockaddr_in *restrict addr, const char *const restrict s, const size_t n);

static void error_message(const char *const message);

static void vulnerable_function(const char *const s, const size_t n);

void INThandler(int);

static void vulnerable_function(const char *const s, const size_t n)
{
    if (n <= 0)
        return;
    char buffer[OLD_BUFFER_SIZE] = {0};
    // oh no .. the dev forgot to change the buffer size !
    // some code ...
    PRINT_D("[DEBUG] buffer address: %p\n", buffer);
    memcpy(buffer, s, n);
    // vulnerable memcpy !!
    // code etc ...
}

static void error_message(const char *const message)
{
    DWORD errCode = GetLastError();
    WCHAR *err;
    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        errCode,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
                        (LPWSTR)&err,
                        0,
                        NULL))
    {
        wprintf(L"Format message failed with 0x%x\n", GetLastError());
        return;
    }
    wprintf(L"%s: %ls, error code: 0x%x\n", message, err, GetLastError());
    LocalFree(err);
}

static int print_output(struct sockaddr_in *restrict addr,
                        const char *const restrict s, const size_t n)
{
    if (n <= 0)
        return n;
    int i = printf("Client [%s:%d] size %d send: \"0x", inet_ntoa(addr->sin_addr),
                   ntohs(addr->sin_port), (int)n);
    for (size_t y = 0; y < n; y++)
    {
        i += printf("%02x", s[y]);
    }
    i += printf("\"\n");
    return i;
}

static struct server_fd *create_serverfd(void)
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
                   (char *)&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    server_fd.addr.sin_family = AF_INET;
    server_fd.addr.sin_addr.s_addr = INADDR_ANY;
    server_fd.addr.sin_port = htons(PORT);
    // Forcefully attaching socket to the port 7777
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

void  INThandler(int sig)
{
    signal(sig, SIG_IGN);
    printf("Receive signal 0x%x, server is closing ...", sig);
    keep_running = 0;
}

int main(int argc, char *argv[])
{
    char buffer[BUFFER_SIZE] = {0};
    struct server_fd *server_fd = {0};
    WSADATA WSAData;

    // set of socket descriptors
    fd_set readfds;

    int max_sd, sd, activity, new_socket;
    int valread = 0;
    static int client_socket[MAX_CLIENTS] = {0};
    char hello_message[] = "Hello !\r\n";
    setlocale(LC_CTYPE, "");
    signal(SIGINT, INThandler);
    signal(SIGTERM, INThandler);
    signal(SIGABRT, INThandler);
    // select timeout
    const struct timeval timeout = { .tv_sec = 1L, .tv_usec = 0L};
    printf("starting ECHO1.0 deamon\n");
    if (WSAStartup(MAKEWORD(2, 0), &WSAData))
    {
        perror("WSAStartup");
        exit(EXIT_FAILURE);
    }
    server_fd = create_serverfd();
    printf("Waiting for connections ...\n");
    while (keep_running)
    {
        FD_ZERO(&readfds);
        // add master socket to set
        FD_SET(server_fd->sockfd, &readfds);
        max_sd = server_fd->sockfd;
        // add child sockets to set
        for (uint32_t i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }
        // wait for an activity on one of the sockets , timeout is NULL ,
        // so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }
        // If something happened on the master socket ,
        // then its an incoming connection
        if (FD_ISSET(server_fd->sockfd, &readfds))
        {
            if ((new_socket = accept(server_fd->sockfd,
                                     (struct sockaddr *)&(server_fd->addr), &(server_fd->addrlen))) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            // inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \
                  \n",
                   new_socket, inet_ntoa(server_fd->addr.sin_addr), ntohs(server_fd->addr.sin_port));

            // send new connection greeting message
            if ((size_t)send(new_socket, hello_message, strlen(hello_message), 0) != strlen(hello_message))
            {
                perror("send");
            }

            // add new socket to array of sockets
            for (uint32_t i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        // else its some IO operation on some other socket
        for (uint32_t i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing , and also read the
                // incoming message
                if ((valread = recv(sd, buffer, BUFFER_SIZE, 0)) == 0 ||
                    WSAGetLastError() == WSAETIMEDOUT ||
                    WSAGetLastError() == WSAECONNRESET)
                {
                    // Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&(server_fd->addr),
                                &(server_fd->addrlen));
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(server_fd->addr.sin_addr), ntohs(server_fd->addr.sin_port));
                    closesocket(sd);
                    client_socket[i] = 0;
                }
                else if (valread == -1)
                {
                    perror("Read failed");
                    exit(1);
                }
                else
                {
                    print_output(&(server_fd->addr), buffer, valread);
                    vulnerable_function(buffer, valread);
                }
            }
        }
    }
    closesocket(server_fd->sockfd);
    WSACleanup();
    return EXIT_SUCCESS;
}
