// src/dclient.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

void send_request(Request *req, Response *res) {
    // Criar pipe nomeado para resposta
    char client_pipe[128];
    snprintf(client_pipe, sizeof(client_pipe), "%s%d", PIPE_SERVER_TO_CLIENT_PREFIX, getpid());
    unlink(client_pipe);
    mkfifo(client_pipe, 0666);

    // Enviar pedido ao servidor
    int server_fd = open(PIPE_CLIENT_TO_SERVER, O_WRONLY);
    if (server_fd < 0) {
        perror("Erro ao abrir pipe do servidor");
        exit(1);
    }
    write(server_fd, req, sizeof(Request));
    close(server_fd);

    // Ler resposta do servidor
    int client_fd = open(client_pipe, O_RDONLY);
    if (client_fd < 0) {
        perror("Erro ao abrir pipe do cliente");
        exit(1);
    }
    read(client_fd, res, sizeof(Response));
    close(client_fd);

    // Remover pipe do cliente
    unlink(client_pipe);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s [comando] [argumentos]\n", argv[0]);
        return 1;
    }

    Request req = { .client_pid = getpid(), .op = OP_PING };
    Response res = { .response = "" };

    if (strcmp(argv[1], CMD_ADD) == 0 && argc == 6) {
        // Indexação
        req.op = OP_ADD;
        snprintf(req.data, MAX_MESSAGE, "%s|%s|%s|%s", argv[2], argv[3], argv[4], argv[5]);
    } else if (strcmp(argv[1], CMD_QUERY) == 0 && argc == 3) {
        // Consulta
        req.op = OP_QUERY;
        snprintf(req.data, MAX_MESSAGE, "%s", argv[2]);
    } else if (strcmp(argv[1], CMD_DELETE) == 0 && argc == 3) {
        // Remoção
        req.op = OP_DELETE;
        snprintf(req.data, MAX_MESSAGE, "%s", argv[2]);
    } else if (strcmp(argv[1], CMD_COUNT) == 0 && argc == 4) {
        // Contagem de linhas
        req.op = OP_COUNT;
        snprintf(req.data, MAX_MESSAGE, "%s|%s", argv[2], argv[3]);
    } else if (strcmp(argv[1], CMD_SEARCH) == 0 && argc == 3) {
        // Pesquisa
        req.op = OP_SEARCH;
        snprintf(req.data, MAX_MESSAGE, "%s", argv[2]);
    } else if (strcmp(argv[1], CMD_SHUTDOWN) == 0) {
        // Encerramento
        req.op = OP_SHUTDOWN;
    } else {
        printf("Comando inválido.\n");
        return 1;
    }

    // Enviar pedido e exibir resposta
    send_request(&req, &res);
    printf("%s\n", res.response);

    return 0;
}
