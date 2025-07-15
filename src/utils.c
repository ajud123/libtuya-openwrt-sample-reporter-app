#include "utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <cJSON.h>

/* 
 * Generates a JSON string from the given memory statistics struct
 */
char *generate_memory_json_string(struct memory_stats *stats) {
        cJSON *root = cJSON_CreateObject();

        cJSON *totalRam = cJSON_CreateObject();
        cJSON_AddNumberToObject(totalRam, "value", stats->total);
        cJSON_AddItemToObjectCS(root, "totalRAM", totalRam);

        cJSON *freeRam = cJSON_CreateObject();
        cJSON_AddNumberToObject(freeRam, "value", stats->free);
        cJSON_AddItemToObjectCS(root, "freeRAM", freeRam);

        return cJSON_PrintUnformatted(root);
}

/*
 * Turns the process into a daemon
 * Returns -1 on failure
 * Returns 1 if the process should exit
 * Returns 0 if the process became a daemon
 */
int become_daemon()
{
	switch (fork()) {
	case -1:
		return -1;
	case 0:
		break;
	default:
		return 1;
	}

	if (setsid() == -1)
		return -1;

	switch (fork()) {
	case -1:
		return -1;
	case 0:
		break;
	default:
		return 1;
	}

	umask(0);
	return 0;
}