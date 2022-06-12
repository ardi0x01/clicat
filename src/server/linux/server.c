#include "../server.h"

void create_client(client_info *client, int new_socket, struct sockaddr_in address,char *buff1,char *buff2) {
        client_no++;
        memset(client->name, 0,30);
        memset(buff1, 0, 1024);
        client->socket = new_socket;
        client->address = address;
        client->uid = client_no;
        strcpy(client->name, buff2);

        sprintf(buff1, "%s has joined the party \n", client->name);
        printf("%s", buff1);
        push_data_client(client);
}

void recv_and_send_msg(int server_fd, struct sockaddr_in address, int addrlen)
{
        char buffer[1024] = { 0 };
        char read_msg[4096];
        char user_data[100];
        int new_socket = 0, valread, get_client_sock, epollfd, new_events;
        int max_sd = server_fd;
        int private_flag = 0;
        int private_sock[20] = {};

        struct epoll_event ev, events[MAX_EVENTS];
        epollfd = epoll_create1(0);
        if (epollfd == -1) {
                perror("epoll_create1");
                exit(EXIT_FAILURE);
        }

        ev.events = EPOLLIN;
        ev.data.fd = server_fd;

        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
                perror("epoll_ctl: listen_sock");
                exit(EXIT_FAILURE);
        }

        while (1) {
                new_events = epoll_wait(epollfd, events, MAX_EVENTS, -1);
                if (new_events == -1)
                {
                        perror("epoll_wait");
                        exit(EXIT_FAILURE);
                }
                for (int i = 0; new_events > i; i++) {
                        int event_fd = events[i].data.fd;
                        if (event_fd == -1)
                        {
                                printf("Client has disconnected \n");
                                client_no--;
                                global_client[event_fd] = NULL;
                                close(event_fd);
                        }
                        if (event_fd == server_fd) {
                                if ((new_socket = accept(event_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) < 0)
                                {
                                        perror("accept");
                                        exit(EXIT_FAILURE);
                                }
                                ev.events = EPOLLIN | EPOLLET;
                                ev.data.fd = new_socket;
                                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_socket,
                                              &ev) == -1) {
                                        perror("epoll_ctl: conn_sock");
                                        exit(EXIT_FAILURE);
                                }

                                memset(user_data, 0, 100);
                                recv(new_socket, user_data, 30, 0);
                                client_info* newClient = (client_info*)malloc(sizeof(client_info));
                                create_client(newClient, new_socket, address, buffer, user_data);

                                if (new_socket > max_sd) {
                                        max_sd = new_socket;
                                }
                        } else {
                                if (private_flag == 0) {
                                        memset(buffer, 0, 1024);
                                        memset(read_msg, 0, 4096);
                                        valread = recv(event_fd, buffer, 1024, 0);
                                        if(valread > 0) {
                                                if (strcmp(buffer, "./private") == 10) {
                                                        char private_name[30];
                                                        send_msg(event_fd, get_list_client(event_fd), 1);
                                                        memset(private_name, 0, 30);
                                                        recv(event_fd, private_name, 1024, 0);
                                                        get_client_sock = get_socket_by_name(private_name);
                                                        if (get_client_sock > 0) {
                                                                push_data_sock(private_sock, 20, event_fd);
                                                                private_flag = 1;
                                                        } else {
                                                                for (int j = 0; j < client_no; j++) {
                                                                        if (event_fd == global_client[j]->socket) {
                                                                                send_msg(global_client[j]->socket, "Name Not Found! \n", 0);
                                                                        }
                                                                }
                                                        }
                                                } else {
                                                        for (int j = 0; j < client_no; j++) {
                                                                if (event_fd == global_client[j]->socket) {
                                                                        sprintf(read_msg, ANSI_COLOR_PUBLIC_CHAT "%s" ANSI_COLOR_RESET  ANSI_COLOR_TEXT ": %s" ANSI_COLOR_RESET ,
                                                                                global_client[j]->name, buffer);
                                                                }
                                                        }
                                                        send_msg(event_fd, read_msg, 0);
                                                }
                                        }
                                } else if(private_flag > 0) {
                                        memset(buffer, 0, 1024);
                                        memset(read_msg, 0, 4096);
                                        printf("[LOG] Private chat \n");
                                        valread = recv(event_fd, buffer, 1024, 0);

                                        int private_sock_data = check_sock_private(private_sock, 20, event_fd);
                                        if (valread > 0) {
                                                for (int j = 0; j < client_no; j++) {
                                                        if (event_fd == global_client[j]->socket) {
                                                                sprintf(read_msg, ANSI_COLOR_PRIVATE "%s" ANSI_COLOR_RESET ANSI_COLOR_TEXT ": %s" ANSI_COLOR_RESET , global_client[j]->name, buffer);
                                                        }
                                                }
                                                if(private_sock_data != get_client_sock && private_sock_data != 0){
                                                        if(strcmp(buffer, "./quit_private") == 10) {
                                                                private_flag = 0;
                                                                send_msg(private_sock_data, "===== Public Chat ===== \n", 1);
                                                        } else {
                                                                send_msg(get_client_sock, read_msg, 1);
                                                        }
                                                } else {
                                                        if(strcmp(buffer, "./quit_private") == 10) {
                                                                private_flag = 0;
                                                                send_msg(get_client_sock, "===== Public Chat ===== \n", 1);
                                                        } else {
                                                                send_msg(private_sock[0], read_msg, 1);
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}

void run_server()
{
        printf("Server is run \n");
        int server_fd;
        struct sockaddr_in address;
        int addrlen = sizeof(address);

        int opt = 1;
        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
                perror("socket failed");
                exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                       &opt, sizeof(opt)))
        {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr*)&address,
                 sizeof(address)) < 0)
        {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 3) < 0)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        recv_and_send_msg(server_fd, address, addrlen);
}



int start_server()
{
        run_server();
        return 0;
}