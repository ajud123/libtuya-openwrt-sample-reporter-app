#ifndef TUYAREPORTER_UTILS
#define TUYAREPORTER_UTILS

#include "net_interfaces.h"

struct memory_stats {
        unsigned long total;
        unsigned long free;
};

char *generate_memory_json_string(struct memory_stats *stats);
char *generate_network_json_string(struct if_list **list);

int become_daemon();

#endif