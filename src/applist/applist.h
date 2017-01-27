#ifndef APPLIST_H
#define APPLIST_H

#include <stdint.h>

typedef struct application {
    char name[256];
    char id[16];
} application_t;

typedef struct applist_t {
    uint32_t size;
    application_t *items;
} applist_t;

int applist_load(applist_t *list);

#endif

