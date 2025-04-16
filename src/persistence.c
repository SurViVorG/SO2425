// src/persistence.c

#include <stdio.h>
#include <string.h>
#include "persistence.h"

int load_documents(Document docs[], int *count) {
    FILE *file = fopen(META_FILE, "r");
    if (!file) {
        *count = 0;
        return 0; // Não existe ainda, não é erro
    }

    int i = 0;
    while (fscanf(file, "%d|%199[^|]|%199[^|]|%d|%63[^\n]\n",
                  &docs[i].id, docs[i].title, docs[i].authors,
                  &docs[i].year, docs[i].path) == 5) {
        i++;
        if (i >= MAX_DOCS) break;
    }

    *count = i;
    fclose(file);
    return 1;
}

int save_documents(Document docs[], int count) {
    FILE *file = fopen(META_FILE, "w");
    if (!file) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d|%s|%s|%d|%s\n",
                docs[i].id, docs[i].title, docs[i].authors,
                docs[i].year, docs[i].path);
    }

    fclose(file);
    return 1;
}
