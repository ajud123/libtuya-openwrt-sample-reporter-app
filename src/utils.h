#ifndef TUYAREPORTER_UTILS
#define TUYAREPORTER_UTILS

#include <tuyalink_core.h>

struct memory_stats {
        unsigned long total;
        unsigned long free;
};

char *generate_memory_json_string(struct memory_stats *stats);
int tuyalink_report_action_rsp(tuya_mqtt_context_t* context, char *data, int errcode);
int become_daemon();
char *escape_quotes(char *input);
#endif