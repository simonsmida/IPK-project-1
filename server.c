#include <stdio.h>
#include <stdlib.h> // atoi
#include <string.h> // strlen
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // write

/**
 * @brief print program usage
 */
void usage(char **argv)
{
    printf("Usage: %s PORT\n", argv[0]);
}

/**
 * @brief create server using the port_number
 */
struct sockaddr_in create_server(int port_number)
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);
    return server;
}

/**
 * @brief Initiate the communication - server will be alive
 */
int initiate_live_server(int socket_dr)
{
    char *message;
    int new_socket, c;
    struct sockaddr_in client;
    
    c = sizeof(struct sockaddr_in);
    
    printf("Waiting for incoming connections...\n");
    while ((new_socket = accept(socket_dr, (struct sockaddr *)&client, (socklen_t *)&c))) {
        if (new_socket < 0) {
            perror("Error: accept failed.\n");
            return -1;
        } 
        printf("Connection accepted!\n");
        // Reply to the client
        message = "Hello client, we are connected, but I gotta go now\n";
        write(new_socket, message, strlen(message));
    }
    return 0;
}

int main(int argc, char *argv[]) 
{
    // Check argument validity
    if (argc < 2) {
        fprintf(stderr, "Error: invalid number of arguments.\n");
        usage(argv);
        return -1;
    }

    // Get the local port number
    const int port_number = atoi(argv[1]);
    printf("Port number: %d\n", port_number);

    // Create socket
    int socket_dr = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_dr == -1) {
        fprintf(stderr, "Error: could not create socket.\n");
        return -1;
    }
    
    // Create the server
    struct sockaddr_in server = create_server(port_number);
    printf("Port number: %d\n", server.sin_port);

    // Bind - bind a socket to a particular "address and port"
    if (bind(socket_dr, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Error: bind failed.\n");
        return -1;
    }

    printf("Bind done!\n");

    // Listen on the port_number
    listen(socket_dr, 3);

    // Accept an incomming connection
    if (initiate_live_server(socket_dr) == -1) {
        return -1;
    }
}

