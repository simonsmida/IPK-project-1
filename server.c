#include <stdio.h>
#include <sys/socket.h>


int main(int argc, char *argv[]) 
{
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Error: could not create a socket\n");
        return -1;
    }
    return 0;
}
