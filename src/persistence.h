// src/persistence.h

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "common.h"

#define META_FILE "tmp/meta.txt"
#define MAX_DOCS 1000

int load_documents(Document docs[], int *count);
int save_documents(Document docs[], int count);

#endif
