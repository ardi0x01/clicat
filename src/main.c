#include "init.h"
int main (void)
{
        start_server();
        run_client("127.0.0.1", "1234");
        return 0;
}