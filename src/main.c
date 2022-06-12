#include "init.h"
#include <stdio.h>
int main (void)
{
        printf("Main function");
        start_server();
        run_client("127.0.0.1", "1234");
        return 0;
}