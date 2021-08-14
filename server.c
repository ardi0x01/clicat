#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "talk.h"

#define PORT 8080
#define MAX_CLIENT 20
#define MAX_CHAT_REACH 100

int client_no = 0;
typedef struct user_info{
    int socket;
    char name[30];
    struct sockaddr_in address;
    int uid;
} client_info;

client_info *global_client[MAX_CLIENT];

void push_data_client(client_info *cli){
    for(int i=0; i<MAX_CLIENT; i++){
        if(!global_client[i]) {
            global_client[i] = cli;
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
void send_msg(int sockid, char *s, int type){
    for(int i=0; i<MAX_CHAT_REACH; i++) {
        if(global_client[i]){
            if(type == 0)
            {
                if (global_client[i]->socket != sockid) {
                    if (send(global_client[i]->socket, s, 4096, 0) < 0) {
                        perror("Send");
                        break;
                    }
                }
            }
        	else
            {
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
    client->socket = new_socket;
    client->address = address;
    client->uid = client_no;
    strcpy(client->name, buff2);

    sprintf(buff1, "%s has joined \n", client->name);
    printf("%s", buff1);
    push_data_client(client);
}

int chk_if_private(char msg[])
{
    int socket_data;
    if (msg)
    {
        printf("Limiter %s \n", msg);
        for (int i = 0; i <client_no; i++)
        {
            if (strcmp(msg, global_client[i]->name) == 0)
            {
                printf("Socket %d \n", global_client[i]->socket);
                socket_data = global_client[i]->socket;
                break;
            }
        }
        return socket_data;
    }
    
    
    return 0;
}



void recv_and_send_msg(int server_fd, struct sockaddr_in address, int addrlen)
{
    char buffer[1024] = { 0 };
    char read_msg[4096];
    char user_data[100];
    int opt = 1;
    int new_socket = 0, valread, get_client_sock;
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    int max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    int private_flag = 0;
    while (1) {
        memcpy(&working_set, &master_set, sizeof(master_set));
        int rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);
        if (rc < 0) {
            perror("select() failed");
            break;
        }
        if (rc == 0)
        {
            printf("  select() timed out.  End program.\n");
            break;
        }
        for (int i = 1; i <= max_sd; ++i) {
            if (FD_ISSET(i, &working_set)) {
                if (i == server_fd) {
                    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                        (socklen_t*)&addrlen)) < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    int recv_usr = recv(new_socket, user_data, 30, 0);

                    client_info* newClient = (client_info*)malloc(sizeof(client_info));
                    create_client(newClient, new_socket, address, buffer, user_data);

                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd) {
                        max_sd = new_socket;
                    }
                }
                else {
                    if (private_flag == 0) {
                        memset(buffer, 0, 1024);
                        memset(read_msg, 0, 4096);

                        valread = recv(i, buffer, 1024, 0);
                        
                        if(valread > 0)
                        {
                            printf("Public chat \n");
                            if (strcmp(buffer, "./private") == 10)
                            {
                                char private_name[30];
                                int recv_name = recv(i, private_name, 1024, 0);
                                get_client_sock = chk_if_private(private_name);
                                if (get_client_sock > 0)
                                    private_flag = 1;
                            }
                            else
                            {
                                for (int j = 0; j < client_no; j++) {
                                    if (i == global_client[j]->socket) {
                                        sprintf(read_msg, "%s : %s", global_client[j]->name, buffer);
                                    }
                                }
                                printf("%s \n", read_msg);
                                send_msg(i, read_msg, 0);
                            }
                        }
                    	else
                        {
                            FD_CLR(i, &master_set);
                            close(i);
                            client_no--;
                            global_client[i] = NULL;
                            printf("User disconnected \n");
                        }
                        
                        
                    }
                    else if(private_flag > 0)
                    {
                        memset(buffer, 0, 1024);
                        memset(read_msg, 0, 4096);
                        printf("Private chat \n");
                        valread = recv(i, buffer, 1024, 0);
                        printf("Val read : %d \n", valread);
                        
                        if (valread > 0) {
                            printf("Get client sock : %d \n", get_client_sock);

                            for (int j = 0; j < client_no; j++) {
                                if (i == global_client[j]->socket) {
                                    sprintf(read_msg, "%s : %s", global_client[j]->name, buffer);
                                }
                            }
                            
                            send_msg(get_client_sock, read_msg, 1);
                        }
                        else 
                        {
                            FD_CLR(i, &master_set);
                            close(i);
                            client_no--;
                            global_client[i] = NULL;
                            printf("User disconnected \n");
                        }
                    }
                }
            }
        }
    }
}



void run_server()
{
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
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
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