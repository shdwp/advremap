#include "applist.h"

#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

#define APP_DB "ur0:shell/db/app.db"

static int get_applist_callback(void *data, int argc, char **argv, char **cols) {
    applist_t *list = (applist_t*)data;

    int i = list->size++;
    list->items = realloc(list->items, sizeof(application_t) * list->size);

    strcpy(list->items[i].name, argv[2]);
    strcpy(list->items[i].id, argv[1]);

    for (int i = 0; i < 256; i++) {
        if (list->items[i].name[i] == '\n') {
            list->items[i].name[i] = ' ';
        }
    }
    return 0;
}

int applist_load(applist_t *list) {
    char *query = "select a.titleid, b.realid, c.title, d.ebootbin,"
                  "       rtrim(substr(d.ebootbin, 0, 5), ':') as dev"
                  "  from (select titleid"
                  "          from tbl_appinfo"
                  "         where key = 566916785"
                  "           and titleid like 'PCS%'"
                  "         order by titleid) a,"
                  "       (select titleid, val as realid"
                  "          from tbl_appinfo"
                  "         where key = 278217076) b,"
                  "       tbl_appinfo_icon c,"
                  "       (select titleid, val as ebootbin"
                  "          from tbl_appinfo"
                  "         where key = 3022202214) d"
                  " where a.titleid = b.titleid"
                  "   and a.titleid = c.titleid"
                  "   and a.titleid = d.titleid";

    sqlite3 *db;
    int ret = sqlite3_open(APP_DB, &db);
    if (ret) {
        return -1;
    }
    char *errMsg;
    ret = sqlite3_exec(db, query, get_applist_callback, (void *)list, &errMsg);
    if (ret != SQLITE_OK) {
        sqlite3_close(db);
        return -2;
    }
    sqlite3_close(db);

    if (list->size <= 0) {
        return -3;
    }

    return 0;
}
