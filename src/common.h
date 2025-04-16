// src/common.h

#ifndef COMMON_H
#define COMMON_H

#define PIPE_CLIENT_TO_SERVER "tmp/client_to_server"
#define PIPE_SERVER_TO_CLIENT_PREFIX "tmp/server_to_client_"

#define MAX_MESSAGE 256

typedef enum {
    OP_PING = 0,  // Apenas para testar comunicação
    OP_ADD,
    OP_QUERY,
    OP_DELETE
} Operation;

typedef struct {
    pid_t client_pid;
    Operation op;
    char data[MAX_MESSAGE];
} Request;

typedef struct {
    char response[MAX_MESSAGE];
} Response;

typedef struct {
    int id;
    char title[200];
    char authors[200];
    int year;
    char path[64];
} Document;


#endif

