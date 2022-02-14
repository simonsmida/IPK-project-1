#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h> // atoi
#include <string.h> // strlen
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // write

/**
 * @brief Print program usage
 */
void usage(char **argv)
{
    printf("Usage: %s PORT\n", argv[0]);
}

/**
 * @brief Create server using the PORT
 */
struct sockaddr_in create_server_address(int PORT)
{
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    return server_address;
}



int get_idle(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $5}'", "r");
    char idle[100];
    fgets(idle, 100, fp);
    pclose(fp); 
    
    return atoi(idle);
}

int get_iowait(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $6}'", "r");
    char iowait[100];
    fgets(iowait, 100, fp);
    pclose(fp); 

    return atoi(iowait);
}

int get_user(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $2}'", "r");
    char user[100];
    fgets(user, 100, fp);
    pclose(fp); 

    return atoi(user);
}

int get_nice(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $3}'", "r");
    char nice[100];
    fgets(nice, 100, fp);
    pclose(fp); 

    return atoi(nice);
}

int get_system(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $4}'", "r");
    char system[100];
    fgets(system, 100, fp);
    pclose(fp); 

    return atoi(system);
}

int get_irq(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $7}'", "r");
    char irq[100];
    fgets(irq, 100, fp);
    pclose(fp); 

    return atoi(irq);
}

int get_softirq(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $8}'", "r");
    char softirq[100];
    fgets(softirq, 100, fp);
    pclose(fp); 

    return atoi(softirq);
}

int get_steal(void)
{
    FILE *fp = popen("cat /proc/stat | head -n 1 | awk -F ' ' '{print $9}'", "r");
    char steal[100];
    fgets(steal, 100, fp);
    pclose(fp); 

    return atoi(steal);
}
float get_cpu_load()
{
    int prev_idle_i = get_idle();
    int prev_iowait_i = get_iowait();
    int prev_user = get_user();
    int prev_nice = get_nice();
    int prev_system = get_system();
    int prev_irq = get_irq();
    int prev_softirq = get_softirq();
    int prev_steal = get_steal();

    sleep(1);

    int idle_i = get_idle();
    int iowait_i = get_iowait();
    int user = get_user();
    int nice = get_nice();
    int system = get_system();
    int irq = get_irq();
    int softirq = get_softirq();
    int steal = get_steal();
    
    // First part
    unsigned long int prev_idle = prev_idle_i + prev_iowait_i;
    unsigned long int idle = idle_i + iowait_i; 

    // Second part
    unsigned long int prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long int non_idle = user + nice + system + irq + softirq + steal;

    unsigned long prev_total = prev_idle + prev_non_idle;
    unsigned long total = idle + non_idle;

    // Differentiate: actual value minus the previous one
    unsigned long int total_ld = total - prev_total;
    unsigned long int idle_ld = idle - prev_idle;

    return (float)(total_ld - idle_ld) / (float)total_ld;
}

/**
 * @brief Initiate the communication - server will be alive
 */
void initiate_live_server(int server_fd)
{
    //char *message;
    int new_socket, c;
    struct sockaddr_in client;
    
    c = sizeof(struct sockaddr_in);
    
    while (42) {
        printf("Waiting for incoming connections...\n");
        // Create new socket for current connection
        new_socket = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&c);
        if (new_socket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        } 
        printf("Connection accepted!\n");
        char buffer[20000];
        read(new_socket, buffer, 20000);
        //printf("%s\n", buffer);
        
        char request[100];
        if (!strncmp(buffer, "GET", 3)) {
            int i = 0;
            while (buffer[i+4] != '\r') {
                request[i] = buffer[i+4];
                i++;
            }
            request[i] = '\0';
        }
        char message[1000];
        if (!strcmp(request, "/hostname HTTP/1.1")) {
            char host[101];
            gethostname(host, 101);
            snprintf(message, 1000, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\nHostname: %s\n", strlen(host)+strlen("Hostname: "), host);
        } else if (!strcmp(request, "/cpu-name HTTP/1.1")) {
            FILE *x = popen("cat /proc/cpuinfo | grep 'model name' | head -n 1 | awk -F ':' '{print $2}'", "r");
            char cpu_name[100];
            if (fgets(cpu_name, 100, x) != NULL) {

            snprintf(message, 1000, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\nCpu-name: %s\n", strlen(cpu_name)+strlen("Cpu-name: "), cpu_name);
            }
            pclose(x);
        } else if (!strcmp(request, "/load HTTP/1.1")) {
            int cpu_load = get_cpu_load() * 100;
            snprintf(message, 1000, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\nCpu-load: %d%%\n", sizeof(cpu_load) + strlen("Cpu-load: "), cpu_load);
        } else {
            ;
        }
        write(new_socket, message, strlen(message));
        //printf("\n\nRequest: **%s**\n\n", request);

        // SERVER: Reply to the client
        close(new_socket);
    }
}

/**
 * @brief Main function serving the clients
 */
void serve_clients(int PORT)
{
    // Create socket for listening
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Create the server
    struct sockaddr_in server_address = create_server_address(PORT);

    // Binding - bind a socket to a particular "address and port"
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("Binding done!\n");

    // Listen on the PORT
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept incomming connections
    initiate_live_server(server_fd);
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
    const int PORT = atoi(argv[1]);
    serve_clients(PORT);
}

