#include "server.h"

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
        if (msg) {
                for (int i = 0; i <client_no; i++) {
                        if (strcmp(msg, global_client[i]->name) == 0) {
                                socket_data = global_client[i]->socket;
                                break;
                        }
                }
                return socket_data;
        }
        return 0;
}