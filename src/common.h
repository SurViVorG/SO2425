// src/common.h

#ifndef COMMON_H
#define COMMON_H

#define PIPE_CLIENT_TO_SERVER "tmp/client_to_server"
#define PIPE_SERVER_TO_CLIENT_PREFIX "tmp/server_to_client_"

#define MAX_MESSAGE 256

// Comandos suportados pelo cliente
#define CMD_ADD "-a"      // Adicionar documento
#define CMD_QUERY "-c"    // Consultar documento
#define CMD_DELETE "-d"   // Remover documento
#define CMD_COUNT "-l"    // Contar linhas com palavra-chave
#define CMD_SEARCH "-s"   // Pesquisar documentos com palavra-chave
#define CMD_SHUTDOWN "-f" // Encerrar servidor

typedef enum {
    OP_PING = 0,  // Apenas para testar comunicação
    OP_ADD,       // Adicionar documento
    OP_QUERY,     // Consultar documento
    OP_DELETE,    // Remover documento
    OP_COUNT,     // Contar linhas com palavra-chave
    OP_SEARCH,    // Pesquisar documentos com palavra-chave
    OP_SHUTDOWN   // Encerrar servidor
} Operation;

typedef struct {
    pid_t client_pid;         // PID do cliente
    Operation op;             // Operação solicitada
    char data[MAX_MESSAGE];   // Dados do pedido
} Request;

typedef struct {
    char response[MAX_MESSAGE]; // Resposta do servidor
} Response;

typedef struct {
    int id;                   // Identificador único do documento
    char title[200];          // Título do documento
    char authors[200];        // Autores do documento
    int year;                 // Ano do documento
    char path[64];            // Caminho relativo do documento
} Document;

#endif
