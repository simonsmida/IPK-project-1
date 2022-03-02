#define _DEFAULT_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h> // atoi
#include <string.h> // strlen
#include <limits.h> // USHRT_MAX
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // write

#define QUEUE 2  // number of pending connections
#define MAX_BUFFER 1024
#define SOCKET int
typedef unsigned long UL;

/**
 * @brief Print program usage
 */
void usage(char **argv)
{
    printf("Usage: %s <port>\n", argv[0]);
}


int get_cpu_data(char *what_data)
{
    char *awk_field;
    if (!strcmp(what_data, "idle")) {
        awk_field = "{print $5}";
    } else if (!strcmp(what_data, "iowait")) {
        awk_field = "{print $6}";
    } else if (!strcmp(what_data, "user")) {
        awk_field = "{print $2}";
    } else if (!strcmp(what_data, "nice")) {
        awk_field = "{print $3}";
    } else if (!strcmp(what_data, "system")) {
        awk_field = "{print $4}";
    } else if (!strcmp(what_data, "irq")) {
        awk_field = "{print $7}";
    } else if (!strcmp(what_data, "softirq")) {
        awk_field = "{print $8}";
    } else if (!strcmp(what_data, "steal")) {
        awk_field = "{print $9}";
    }
#define MAX_READ 100
    char command[MAX_READ];
    sprintf(command, "cat /proc/stat | head -n 1 | awk -F ' ' '%s'", awk_field);
    FILE *fp = popen(command, "r");
    char data[MAX_READ];
    
    if (fgets(data, MAX_READ, fp) == NULL) {
        pclose(fp);
        fprintf(stderr, "Error: could not get cpu data, fgets() failed\n");
        exit(EXIT_FAILURE);
    }
    pclose(fp); 
    return atoi(data);
}

float get_cpu_load()
{
    int prev_idle_i   = get_cpu_data("idle");
    int prev_iowait_i = get_cpu_data("iowait");
    int prev_user     = get_cpu_data("user"); 
    int prev_nice     = get_cpu_data("nice"); 
    int prev_system   = get_cpu_data("system"); 
    int prev_irq      = get_cpu_data("irq"); 
    int prev_softirq  = get_cpu_data("softirq"); 
    int prev_steal    = get_cpu_data("steal"); 

    sleep(1);

    int idle_i   = get_cpu_data("idle"); 
    int iowait_i = get_cpu_data("iowait"); 
    int user     = get_cpu_data("user"); 
    int nice     = get_cpu_data("nice"); 
    int system   = get_cpu_data("system"); 
    int irq      = get_cpu_data("irq"); 
    int softirq  = get_cpu_data("softirq"); 
    int steal    = get_cpu_data("steal"); 
    
    // First part
    UL prev_idle = prev_idle_i + prev_iowait_i;
    UL idle = idle_i + iowait_i; 
    
    // Second part
    UL prev_non_idle = prev_user + prev_nice + prev_system + 
                           prev_irq + prev_softirq + prev_steal;
    UL non_idle = user + nice + system + irq + softirq + steal;

    UL prev_total = prev_idle + prev_non_idle;
    UL total = idle + non_idle;

    // Differentiate: actual value minus the previous one
    UL total_ld = total - prev_total;
    UL idle_ld = idle - prev_idle;

    // TODO handle menovatel = 0
    return (float)(total_ld - idle_ld) / (float)total_ld;
}

void send_400(SOCKET client)
{
    const char *e400 = "HTTP/1.1 400 Bad Request\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 11\r\n\r\n"
                       "Bad Request";
    send(client, e400, strlen(e400), 0);
    close(client);
}

void send_404(SOCKET client)
{
    const char *e404 = "HTTP/1.1 404 Not Found\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 9\r\n\r\n"
                       "Not Found";
    send(client, e404, strlen(e404), 0);
    close(client);
}

void get_hostname(char *content, unsigned *content_length)
{
    gethostname(content, MAX_BUFFER);
    *content_length = strlen(content);
}

void get_cpuname(char *content, unsigned *content_length)
{
    FILE *fd = popen(
            "cat /proc/cpuinfo | "
            "grep 'model name' | "
            "head -n 1 | awk -F ':' '{print $2}'", "r"
    );
    
    if (fgets(content, MAX_BUFFER, fd) == NULL) {
        pclose(fd);
        fprintf(stderr, "Error: could not get cpu name, fgets() failed\n");
        exit(EXIT_FAILURE);
    }
    *content_length = strlen(content);
    pclose(fd);
}

void get_load(char *content, unsigned *content_length) 
{
    float cpu_load = get_cpu_load();
    int cpu_load_percent = (int)(cpu_load*100);
    sprintf(content, "%d%%", cpu_load_percent);
    *content_length = strlen(content); 
}

void serve_resource(SOCKET client, char *path)
{
    char response[MAX_BUFFER];
    sprintf(response, "HTTP/1.1 200 OK\r\n");
    send(client, response, strlen(response), 0);

    sprintf(response, "Connection: close\r\n");
    send(client, response, strlen(response), 0);
    
    unsigned cl; // content length
    char content[MAX_BUFFER];

    if (!strcmp(path, "/hostname")) {
        get_hostname(content, &cl);
    } else if (!strcmp(path, "/cpu-name")) {
        get_cpuname(content, &cl);
    } else if (!strcmp(path, "/load")) {
        get_load(content, &cl);
    } else {
        // TODo check if there is nothing else to do about the server
        // because we are basically just exiting from the whole program
        // idk if we should close something before other than client's socket
        send_404(client);
        exit(EXIT_FAILURE);
    }
    
    sprintf(response, "Content-length: %u\r\n", cl);
    send(client, response, strlen(response), 0);
    
    sprintf(response, "Content-type: text/plain\r\n");
    send(client, response, strlen(response), 0);
    
    sprintf(response, "\r\n");
    send(client, response, strlen(response), 0);
    
    sprintf(response, "%s", content);
    send(client, response, strlen(response), 0);

    close(client);
}

/**
 * @brief Initiate the communication - server will be waiting for connections
 */
void initiate_live_server(int socket_listen)
{
    //char *message;
    int socket_client;
    struct sockaddr_in client_addr; 
    
    while (42) {
        // TODO get rid of me!
        printf("Waiting for incoming connections...\n");

        // Create new socket for current connection
        // socket_listen(sockfd) ... listening server socket
        // client_addr(addr) ... socket address of the client
        // client_len(addrlen) ... size of structure pointed to by client_addr
        socklen_t client_len = sizeof(struct sockaddr_in);
        socket_client = accept(
                socket_listen, 
                (struct sockaddr *)&client_addr, 
                (socklen_t *)&client_len
        );
        if (socket_client < 0) {
            perror("accept()");
            exit(EXIT_FAILURE);
        } 
        // TODO get rid of me!
        printf("Connection accepted!\n");
        char read_bytes[MAX_BUFFER];
        int bytes_received = recv(socket_client, read_bytes, MAX_BUFFER, 0);
        read_bytes[bytes_received] = '\0';
        if (strstr(read_bytes, "\r\n\r\n")) {
            if (strncmp("GET /", read_bytes, 5)) {
                send_400(socket_client);
            } else {
                char *path = read_bytes + 4;
                char *end_path = strstr(path, " ");
                if (!end_path) {
                    send_400(socket_client);
                } else {
                    *end_path = '\0';
                    printf("Path: %s\n", path);
                    serve_resource(socket_client, path);
                }
            }
        }
        // TODO handle 3 request options (/hostname, /cpu-name, /load)
        // TODO reply to the client
        // close(socket_client);
    }
}

/**
 * @brief Create the server socket structure using the port number
 * @param port port number
 * @return server socket structure
 */
//struct sockaddr_in create_socket_struct(const long port)
struct sockaddr_in create_socket_struct(const int port)
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;         // IPv4 protocol
    server_addr.sin_addr.s_addr = INADDR_ANY; // listening on every connected network interface
    server_addr.sin_port = htons(port);       // host byte to network order
    return server_addr;
}

SOCKET create_socket(const int port)
{
    // Create socket for listening
    // AF_INET(domain)   ... IPv4 internet protocols
    // SOCK_STREAM(type) ... provides sequenced, reliable, two-way byte streams
    // 0 ... supporting only 1 protocol for this socket
    SOCKET socket_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_listen == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    
    // Create the server socket structure 
    struct sockaddr_in server_addr = create_socket_struct(port);
    // Enable reusing the same port as before it was closed
    int optval = 1;
    setsockopt(
            socket_listen, 
            SOL_SOCKET, 
            SO_REUSEADDR | SO_REUSEPORT, 
            (const char *)&optval, sizeof(int)
    );

    // Binding - bind a name to a socket socket to a particular "address and port"
    // socket_listen(sockfd) ... socket to be assigned the address to
    // server_addr(addr) ... address to be assigned to the socket_listen socket
    // (addrlen) ... specifies the size [B] of the address structure
    if (bind(socket_listen, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    // Listen on the port
    if (listen(socket_listen, QUEUE) < 0) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }
    
    return socket_listen;
}

/**
 * @brief Main function serving the clients
 * @param port port number
 */
void serve_clients(const int port)
{
    SOCKET socket_listen = create_socket(port);
    initiate_live_server(socket_listen);
}

/**
 * @brief Convert string to long integer
 * @param argc argument count
 * @param argv argument vector
 * @return port number on success
 */
int get_port_number(int argc, char **argv)
{
    // Check argument validity
    if (argc < 2) {
        fprintf(stderr, "Error: missing argument 'port'.\n");
        usage(argv);
        exit(EXIT_FAILURE);
    }
    
    // Parse input number
    char *endptr;
    long port = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || port < 0) {
        fprintf(stderr, "Error: invalid port number, please enter positive integer.\n");
        usage(argv);
        exit(EXIT_FAILURE);
    }

    if (port > USHRT_MAX) {
        fprintf(stderr, "Error: invalid port number, must be in range [0, 65535].\n");
        usage(argv);
        exit(EXIT_FAILURE);
    }
    
    return (int)port;
}

int main(int argc, char *argv[]) 
{
    const int port = get_port_number(argc, argv);
    serve_clients(port);
}
