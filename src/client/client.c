#include "client.h"

#if defined(_WIN32)
#include <conio.h>
#endif
//void str_overwrite_stdout()
//{
//    printf("\r%s", "> ");
//    fflush(stdout);
//}

int run_client(char *ip, char *port)
{
#if defined(_WIN32)
        WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif



        printf("Configuring remote address...\n");
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo *peer_address;
        if (getaddrinfo(ip, port, &hints, &peer_address)) {
                fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
                return 1;
        }


        printf("Remote address is: ");
        char address_buffer[100];
        char service_buffer[100];
        getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                    address_buffer, sizeof(address_buffer),
                    service_buffer, sizeof(service_buffer),
                    NI_NUMERICHOST);
        printf("%s %s\n", address_buffer, service_buffer);


        printf("Creating socket...\n");
        SOCKET socket_peer;
        socket_peer = socket(peer_address->ai_family,
                             peer_address->ai_socktype, peer_address->ai_protocol);
        if (!ISVALIDSOCKET(socket_peer)) {
                fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
                return 1;
        }


        printf("Connecting...\n");
        if (connect(socket_peer,
                    peer_address->ai_addr, peer_address->ai_addrlen)) {
                fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
                return 1;
        }
        freeaddrinfo(peer_address);

        printf("Connected.\n");

        printf("Enter your name : ");
        char usr_name[100];
        fgets(usr_name, 100, stdin);
//    trim_newline_str(usr_name, sizeof(usr_name));
        send(socket_peer, usr_name, strlen(usr_name), 0);


        printf("To send data, enter text followed by enter.\n");
        while(1) {

                fd_set reads;
                FD_ZERO(&reads);
                FD_SET(socket_peer, &reads);
#if !defined(_WIN32)
                FD_SET(0, &reads);
#endif

                struct timeval timeout;
                timeout.tv_sec = 0;
                timeout.tv_usec = 100000;

                if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
                        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
                        return 1;
                }

                if (FD_ISSET(socket_peer, &reads)) {
                        char read[4096];
                        memset(read, 0, 4096);
                        int bytes_received = recv(socket_peer, read, 4096, 0);
                        if (bytes_received < 1) {
                                printf("Connection closed by peer.\n");
                                break;
                        }
                        printf("%s",read);
                }

#if defined(_WIN32)
                if(_kbhit()) {
#else
                if(FD_ISSET(0, &reads)) {
#endif
                        char read[4096];
                        if (!fgets(read, 4096, stdin)) break;
                        send(socket_peer, read, strlen(read), 0);
                }
        } //end while(1)

        printf("Closing socket...\n");
        CLOSESOCKET(socket_peer);

#if defined(_WIN32)
        WSACleanup();
#endif

        printf("Finished.\n");
        return 0;
}