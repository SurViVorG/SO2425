// src/dserver.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

int main() {
    unlink(PIPE_CLIENT_TO_SERVER);
    mkfifo(PIPE_CLIENT_TO_SERVER, 0666);

    printf("Servidor iniciado.\n");

    while (1) {
        int fd = open(PIPE_CLIENT_TO_SERVER, O_RDONLY);
        if (fd < 0) {
            perror("open client_to_server");
            exit(1);
        }

        Request req;
        read(fd, &req, sizeof(Request));
        close(fd);

        printf("Recebido pedido do cliente %d: %s\n", req.client_pid, req.data);

        // Responder
        char client_pipe[128];
        snprintf(client_pipe, sizeof(client_pipe), "%s%d", PIPE_SERVER_TO_CLIENT_PREFIX, req.client_pid);

        int client_fd = open(client_pipe, O_WRONLY);
        if (client_fd < 0) {
            perror("open server_to_client");
            continue;
        }

        Response res;
        snprintf(res.response, MAX_MESSAGE, "Olá do servidor! Recebi: %s", req.data);
        write(client_fd, &res, sizeof(Response));
        close(client_fd);
    }

    return 0;
}
