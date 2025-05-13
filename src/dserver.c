// src/dserver.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"
#include "persistence.h"

Document documents[MAX_DOCS]; // Array para armazenar documentos
int document_count = 0;       // Contador de documentos

void handle_add_request(Request *req, Response *res) {
    Document doc;
    char title[200], authors[200], path[64];
    int year;

    // Extrair dados do pedido
    sscanf(req->data, "%199[^|]|%199[^|]|%d|%63s", title, authors, &year, path);

    // Preencher a estrutura do documento
    doc.id = document_count + 1; // Gerar ID único
    strncpy(doc.title, title, sizeof(doc.title));
    strncpy(doc.authors, authors, sizeof(doc.authors));
    doc.year = year;
    strncpy(doc.path, path, sizeof(doc.path));

    // Armazenar em memória
    documents[document_count++] = doc;

    // Persistir em disco
    if (!save_documents(documents, document_count)) {
        snprintf(res->response, MAX_MESSAGE, "Erro ao gravar documento no disco");
        return;
    }

    // Preparar resposta
    snprintf(res->response, MAX_MESSAGE, "Document %d indexed", doc.id);
}

void handle_query_request(Request *req, Response *res) {
    int id;
    sscanf(req->data, "%d", &id); // Extrair o ID do pedido

    // Procurar o documento pelo ID
    for (int i = 0; i < document_count; i++) {
        if (documents[i].id == id) {
            // Construir a resposta
            snprintf(res->response, MAX_MESSAGE, "Title: %s\nAuthors: %s\nYear: %d\nPath: %s",
                     documents[i].title, documents[i].authors, documents[i].year, documents[i].path);
            return;
        }
    }

    // Documento não encontrado
    snprintf(res->response, MAX_MESSAGE, "Erro: Documento com ID %d não encontrado", id);
}

void handle_delete_request(Request *req, Response *res) {
    int id;
    sscanf(req->data, "%d", &id); // Extrair o ID do pedido

    // Procurar o documento pelo ID
    for (int i = 0; i < document_count; i++) {
        if (documents[i].id == id) {
            // Remover o documento deslocando os elementos seguintes
            for (int j = i; j < document_count - 1; j++) {
                documents[j] = documents[j + 1];
            }
            document_count--;

            // Atualizar o disco
            if (!save_documents(documents, document_count)) {
                snprintf(res->response, MAX_MESSAGE, "Erro ao atualizar o disco");
                return;
            }

            // Preparar resposta
            snprintf(res->response, MAX_MESSAGE, "Documento com ID %d removido", id);
            return;
        }
    }

    // Documento não encontrado
    snprintf(res->response, MAX_MESSAGE, "Erro: Documento com ID %d não encontrado", id);
}

void handle_count_request(Request *req, Response *res) {
    int id;
    char keyword[MAX_MESSAGE];
    sscanf(req->data, "%d|%s", &id, keyword); // Extrair o ID e a palavra-chave

    // Procurar o documento pelo ID
    for (int i = 0; i < document_count; i++) {
        if (documents[i].id == id) {
            char command[512];
            snprintf(command, sizeof(command), "grep -c '%s' %s", keyword, documents[i].path);

            // Executar o comando e capturar o resultado
            FILE *fp = popen(command, "r");
            if (!fp) {
                snprintf(res->response, MAX_MESSAGE, "Erro ao executar comando grep");
                return;
            }

            int count;
            fscanf(fp, "%d", &count);
            pclose(fp);

            // Preparar resposta
            snprintf(res->response, MAX_MESSAGE, "O documento %d contém %d linhas com a palavra '%s'", id, count, keyword);
            return;
        }
    }

    // Documento não encontrado
    snprintf(res->response, MAX_MESSAGE, "Erro: Documento com ID %d não encontrado", id);
}

void handle_search_request(Request *req, Response *res) {
    char keyword[MAX_MESSAGE];
    sscanf(req->data, "%s", keyword); // Extrair a palavra-chave

    char result[MAX_MESSAGE] = "[";
    int found = 0;

    // Procurar a palavra-chave em cada documento
    for (int i = 0; i < document_count; i++) {
        char command[512];
        snprintf(command, sizeof(command), "grep -q '%s' %s", keyword, documents[i].path);

        // Executar o comando
        if (system(command) == 0) {
            // Adicionar o ID do documento ao resultado
            if (found > 0) {
                strncat(result, ", ", sizeof(result) - strlen(result) - 1);
            }
            char id_str[16];
            snprintf(id_str, sizeof(id_str), "%d", documents[i].id);
            strncat(result, id_str, sizeof(result) - strlen(result) - 1);
            found++;
        }
    }

    strncat(result, "]", sizeof(result) - strlen(result) - 1);

    // Preparar resposta
    if (found > 0) {
        snprintf(res->response, MAX_MESSAGE, "Documentos encontrados: %s", result);
    } else {
        snprintf(res->response, MAX_MESSAGE, "Nenhum documento contém a palavra '%s'", keyword);
    }
}

void handle_shutdown_request(Request *req, Response *res) {
    // Preparar resposta
    snprintf(res->response, MAX_MESSAGE, "Servidor encerrado");
}

int main() {
    int running = 1;
    // Carregar documentos do disco ao iniciar
    if (!load_documents(documents, &document_count)) {
        printf("Nenhum documento encontrado no disco.\n");
    } else {
        printf("Carregados %d documentos do disco.\n", document_count);
    }

    // Criar pipe nomeado para comunicação
    unlink(PIPE_CLIENT_TO_SERVER);
    mkfifo(PIPE_CLIENT_TO_SERVER, 0666);

    printf("Servidor iniciado.\n");

    while (running) {
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

        Response res = { .response = "" };

        // Processar pedido
        switch (req.op) {
            case OP_ADD:
                handle_add_request(&req, &res);
                break;
            case OP_QUERY:
                handle_query_request(&req, &res);
                break;
            case OP_DELETE:
                handle_delete_request(&req, &res);
                break;
            case OP_COUNT:
                handle_count_request(&req, &res);
                break;
            case OP_SEARCH:
                handle_search_request(&req, &res);
                break;
            case OP_SHUTDOWN:
                handle_shutdown_request(&req, &res);
                running = 0;
                break;
            default:
                snprintf(res.response, MAX_MESSAGE, "Operação não suportada");
                break;
        }

        // Enviar resposta ao cliente
        write(client_fd, &res, sizeof(Response));
        close(client_fd);
    }

    return 0;
}
