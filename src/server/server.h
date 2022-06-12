//
// Created by ardi0 on 6/12/2022.
//

#ifndef CLICAT_CLIENT_HANDLE_H
#define CLICAT_CLIENT_HANDLE_H
#include "../init/clicat.h"

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
void push_data_client(client_info *cli);
void trim_newline_str(char* arr, int length);
void client_list();
void send_msg(int sockid, char *s, int type);
void push_data_sock(int arr[], int n, int val);
int check_sock_private(int arr[], int n, int sock);
char *get_list_client(int sockfd);
int get_socket_by_name(char msg[]);
int start_server();
#endif //CLICAT_CLIENT_HANDLE_H
