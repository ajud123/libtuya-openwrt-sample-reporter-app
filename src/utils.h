#ifndef TUYAREPORTER_UTILS
#define TUYAREPORTER_UTILS

struct memory_stats {
        unsigned long total;
        unsigned long free;
};

char *generate_memory_json_string(struct memory_stats *stats);

int become_daemon();

#endif