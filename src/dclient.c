// src/dclient.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <mensagem>\n", argv[0]);
        return 1;
    }

    pid_t pid = getpid();

    char client_pipe[128];
    snprintf(client_pipe, sizeof(client_pipe), "%s%d", PIPE_SERVER_TO_CLIENT_PREFIX, pid);
    mkfifo(client_pipe, 0666);

    Request req;
    req.client_pid = pid;
    req.op = OP_PING;
    snprintf(req.data, MAX_MESSAGE, "%s", argv[1]);

    int fd = open(PIPE_CLIENT_TO_SERVER, O_WRONLY);
    if (fd < 0) {
        perror("open client_to_server");
        return 1;
    }

    write(fd, &req, sizeof(Request));
    close(fd);

    // Espera resposta
    int res_fd = open(client_pipe, O_RDONLY);
    if (res_fd < 0) {
        perror("open server_to_client");
        return 1;
    }

    Response res;
    read(res_fd, &res, sizeof(Response));
    close(res_fd);

    printf("Resposta do servidor: %s\n", res.response);

    unlink(client_pipe);
    return 0;
}
