#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "talk.h"

#define PORT 1234
#define MAX_CLIENT 20
#define MAX_CHAT_REACH 100
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_PUBLIC_CHAT  "\x1b[36m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_PRIVATE "\x1b[93m"
#define ANSI_COLOR_TEXT    "\x1b[100m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int client_no = 0;
typedef struct user_info{
    int socket;
    char name[30];
    struct sockaddr_in address;
    int uid;
} client_info;

client_info *global_client[MAX_CLIENT];

void push_data_client(client_info *cli)
{
    for(int i=0; i<MAX_CLIENT; i++){
        if(!global_client[i]) {
            global_client[i] = cli;
            break;
        }
    }
}
void trim_newline_str(char* arr, int length)
{
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void client_list()
{
	for(int i=0; i<client_no; i++)
	{
        printf("%s \n", global_client[i]->name);
	}
}
void send_msg(int sockid, char *s, int type)
{
    for(int i=0; i<MAX_CHAT_REACH; i++) {
        if(global_client[i]){
            if(type == 0){
                if (global_client[i]->socket != sockid) {
                    if (send(global_client[i]->socket, s, 4096, 0) < 0) {
                        perror("Send");
                        break;
                    }
                }
            } else {
                if (global_client[i]->socket == sockid) {
                    if (send(global_client[i]->socket, s, 4096, 0) < 0) {
                        perror("Send");
                        break;
                    }
                }
            }
        }
    }
}

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
char *get_list_client(int sockfd)
{
    char bigString[1000];
    char *p = bigString;

    p = strcat(p, "===== Select the name you want chat it ===== \n");
    for(int i=0; i<client_no; i++) {
        if(sockfd != global_client[i]->socket){
            if(*global_client[i]->name != '\0') {
                p = strcat(p, global_client[i]->name);
            }
        }
    }
    p = strcat(p, "\n");
    return p;
}

int get_socket_by_name(char msg[])
{
    int socket_data;
    printf("[GET SOCKET BY NAME] Name : %s \n", msg);
    if (msg) {
        for (int i = 0; i <client_no; i++) {
            if (strcmp(msg, global_client[i]->name) == 0) {
                printf("[GET SOCKET BY NAME] Socket %d \n", global_client[i]->socket);
                socket_data = global_client[i]->socket;
                break;
            }
        }
        return socket_data;
    }
    return 0;
}

void push_data_sock(int arr[], int n, int val)
{
    int top = 0;
    for(int i=0; i<n; i++){
        if(arr[i] != 0) {
            top++;
        }
    }
    if(top > n) {
        perror("Stack if overflow");
    } else {
        arr[top] = val;
        top++;
    }
}

int check_sock_private(int arr[], int n, int sock)
{
    for(int i=0; i<n; i++)
    {
        if(arr[i] == sock) {
            return arr[i];
        } else {
            return 0;
        }
    }
}

void recv_and_send_msg(int server_fd, struct sockaddr_in address, int addrlen)
{
    char buffer[1024] = { 0 };
    char read_msg[4096];
    char user_data[100];
    int opt = 1, new_events;
    int new_socket = 0, valread, get_client_sock;

    /** DEPRECATED SYSCALL
//    fd_set master_set, working_set;
//    FD_ZERO(&master_set);
//    FD_SET(server_fd, &master_set);
     **/

    int max_sd = server_fd;
    int private_flag = 0;
    int private_sock[20] = {};

    struct kevent change_event[4],
            event[4];
    int kq = kqueue();

    // Create event 'filter', these are the events we want to monitor.
    // Here we want to monitor: socket_listen_fd, for the events: EVFILT_READ
    // (when there is data to be read on the socket), and perform the following
    // actions on this kevent: EV_ADD and EV_ENABLE (add the event to the kqueue
    // and enable it).
    EV_SET(change_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    // Register kevent with the kqueue.
    if (kevent(kq, change_event, 1, NULL, 0, NULL) == -1)
    {
        perror("kevent");
        exit(1);
    }

    while (1) {
        new_events = kevent(kq, NULL, 0, event, 1, NULL);
        if (new_events == -1)
        {
            perror("kevent");
            exit(1);
        }
        /** DEPRECATED SYSCALL
//        memcpy(&working_set, &master_set, sizeof(master_set));
//        int rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);
//        if (rc < 0) {
//            perror("select() failed");
//            break;
//        }
//        if (rc == 0)
//        {
//            printf("  select() timed out.  End program.\n");
//            break;
//        }
         **/
        for (int i = 0; new_events > i; i++) {
            int event_fd = event[i].ident;
            if (event[i].flags & EV_EOF)
            {
                printf("Client has disconnected \n");
                client_no--;
                global_client[event_fd] = NULL;
                close(event_fd);
            }
//            if (FD_ISSET(i, &working_set)) {
            if (event_fd == server_fd) {
                if ((new_socket = accept(event_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) < 0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                memset(user_data, 0, 100);
                int recv_usr = recv(new_socket, user_data, 30, 0);
                client_info* newClient = (client_info*)malloc(sizeof(client_info));
                create_client(newClient, new_socket, address, buffer, user_data);

                EV_SET(change_event, new_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kq, change_event, 1, NULL, 0, NULL) < 0)
                {
                    perror("kevent error");
                }

                if (new_socket > max_sd) {
                    max_sd = new_socket;
                }
            } else if(event[i].filter & EVFILT_READ){
                if (private_flag == 0) {
                    memset(buffer, 0, 1024);
                    memset(read_msg, 0, 4096);
                    valread = recv(event_fd, buffer, 1024, 0);
                    if(valread > 0) {
                        printf("[LOG] Public chat \n");
                        if (strcmp(buffer, "./private") == 10) {
                            char private_name[30];
                            send_msg(event_fd, get_list_client(event_fd), 1);
                            memset(private_name, 0, 30);
                            int recv_name = recv(event_fd, private_name, 1024, 0);
                            printf("[LOG] Private name : %s \n", private_name);
                            get_client_sock = get_socket_by_name(private_name);
                            if (get_client_sock > 0) {
                                printf("Private flag set to 1 \n");
                                push_data_sock(private_sock, 20, event_fd);
                                private_flag = 1;
                            } else {
                                printf("[LOG] Name not found \n");
                                for (int j = 0; j < client_no; j++) {
                                    if (event_fd == global_client[j]->socket) {
                                        printf("[LOG] Same Sock Private Failed \n");
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
//                            printf("%s", read_msg);
                            send_msg(event_fd, read_msg, 0);
                        }
                    }
                } else if(private_flag > 0) {
                    memset(buffer, 0, 1024);
                    memset(read_msg, 0, 4096);
                    printf("[LOG] Private chat \n");
                    valread = recv(event_fd, buffer, 1024, 0);

                    printf("[PRIVATE] Read : %s \n ", buffer);
                    int private_sock_data = check_sock_private(private_sock, 20, event_fd);
                    if (valread > 0) {
                        printf("[PRIVATE] Valread > 0 \n");
                        for (int j = 0; j < client_no; j++) {
                            printf("[PRIVATE] Loop \n");
                            printf("[PRIVATE] Event fd : %d \n",event_fd);
                            printf("[PRIVATE] Socket : %d \n", global_client[j]->socket);
                            if (event_fd == global_client[j]->socket) {
                                printf("Same \n");
                                sprintf(read_msg, ANSI_COLOR_PRIVATE "%s" ANSI_COLOR_RESET ANSI_COLOR_TEXT ": %s" ANSI_COLOR_RESET , global_client[j]->name, buffer);
                            }
                        }
                        printf("[PRIVATE] Message : %s \n", buffer);
                        printf("[PRIVATE] Private Sock : %d \n", private_sock_data);
                        if(private_sock_data != get_client_sock && private_sock_data != 0){
                            printf("[PRIVATE] not same sock, client sock : %d \n", get_client_sock);
                            if(strcmp(buffer, "./quit_private") == 10) {
                                private_flag = 0;
                                send_msg(private_sock_data, "===== Public Chat ===== \n", 1);
                            } else {
                                send_msg(get_client_sock, read_msg, 1);
                            }
                        } else {
                            printf("[PRIVATE] Same sock \n");
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


int main(int argc, char const *argv[])
{
    run_server();
    return 0;
}