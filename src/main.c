#include "init.h"
int main (void)
{
        int question;
        printf("Start server/client ? (1) for server (2) for client \n");
        scanf("%d", &question);

        if(question == 1)
                start_server();
        else if(question == 2)
                run_client("127.0.0.1", "1234");
        return 0;
}