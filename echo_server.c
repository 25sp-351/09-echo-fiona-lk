#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 2345
#define BUFFER_SIZE 1024

int verbose = 0;

void* handleConnection(void* sock_fd_ptr) {
    int sock_fd = *(int*)sock_fd_ptr;
    free(sock_fd_ptr);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Handling connection on %d\n", sock_fd);

    // revieve echo loop
    while ((bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (verbose)
            printf("Received: %s", buffer);
        send(sock_fd, buffer, bytes_read, 0);
    }

    // cleaning connection
    printf("Done with connection %d\n", sock_fd);
    close(sock_fd);
    return NULL;
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    // parsing command-line
    for (int i = 1; i < argc; ++i)
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);
        else if (strcmp(argv[i], "-v") == 0)
            verbose = 1;

    // create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr = {.sin_family      = AF_INET,
                                      .sin_port        = htons(port),
                                      .sin_addr.s_addr = INADDR_ANY};

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
        0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    // listening for connection
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Listening on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int* client_fd       = malloc(sizeof(int));
        *client_fd =
            accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        printf("Accepted connection on %d\n", *client_fd);

        pthread_t thread;
        pthread_create(&thread, NULL, handleConnection, (void*)client_fd);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
