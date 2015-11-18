#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

void *process(void *arg)
{
    int client_socket = (int)arg;
    char buffer[100];

    pthread_detach(pthread_self());

    if (recv(client_socket, buffer, 100, 0) != -1)
    {
        printf("client: %s", buffer);
        send(client_socket, "hello\n", 6, 0);
        close(client_socket);
    }

    return NULL;
}

int main()
{
    int port = 10000;
    int server_socket, client_socket;    
    struct sockaddr_in server_address, client_address;
    socklen_t address_size;    
    int yes = 1;
    pthread_t thread;

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