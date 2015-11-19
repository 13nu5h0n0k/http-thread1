#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

char *root;

void *process(void *arg)
{
    int client_socket = (int)arg;
    FILE *client = fdopen(client_socket, "r");
    char buffer[500];
    char address[500];
    int address_length;
    int file;
    int type;
    int file_size;
    struct stat file_stat;
    char *file_in_memory;

    pthread_detach(pthread_self());

    if (fgets(buffer, 100, client) != NULL)
    {
        if (strncmp(buffer, "GET ", 4) == 0)
        {
            type = 0;
            address_length = strchr(buffer + 4, ' ') - (buffer + 4);
            strcpy(address, root);
            strncpy(address + strlen(root), buffer + 4, address_length);
            
        }

        else if (strncmp(buffer, "HEAD ", 5) == 0)
        {
            type = 1;
            address_length = strchr(buffer + 5, ' ') - (buffer + 5);
            strcpy(address, root);
            strncpy(address + strlen(root), buffer + 5, address_length);
        }

        else
        {
            printf("error\n");
            send(client_socket, "error\n", 6, 0);
            close(client_socket);
            return NULL;
        }

        address_length = address_length + strlen(root);
        if (address[address_length - 1] == '/')
        {
            strcpy(address + address_length, "index.html");
            address_length = address_length + 10;
        }
        address[address_length] = '\0';

        file = open(address, O_RDONLY, 0);
        if (file != -1)
        {
            send(client_socket, "HTTP/1.0 200 OK\n", 16, 0);
            send(client_socket, "Server: http thread1\n\n", 22, 0);

            if (type == 0)
            {                
                fstat(file, &file_stat);
                file_size = (int)file_stat.st_size;
                file_in_memory = (char *)malloc(file_size);
                read(file, file_in_memory, file_size);
                
                send(client_socket, file_in_memory, file_size, 0);

                free(file_in_memory);
            }

        }

        else 
        {
            send(client_socket, "HTTP/1.0 404 Not Found\n\n", 25, 0);
        }
        
        close(client_socket);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    int port = 10000;
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t address_size;    
    int yes = 1;
    pthread_t thread;
    
    if (argc < 2)
        return -1;

    root = malloc(strlen(argv[1])+1);
    strncpy(root, argv[1], strlen(argv[1]));

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = 0;
        
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr));

    listen(server_socket, 10);

    address_size = sizeof(struct sockaddr_in);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &address_size);
        pthread_create(&thread, NULL, process, (void *)client_socket);        
    }
}