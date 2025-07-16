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

int populate_json_array_with_interfaces(cJSON *array, struct if_list **interfaces){
        struct if_list *current = *interfaces;
        while(current != NULL){
                struct cJSON *root = cJSON_CreateObject();
                if(root == NULL){
                        syslog(LOG_ERR, "Failed to create a JSON object for serializing network interfaces");
                        return -1;
                }
                cJSON_AddItemToObjectCS(root, "name", cJSON_CreateString(current->name));
                if(current->ipv4_addr != NULL){
                        cJSON_AddItemToObjectCS(root, "address", cJSON_CreateString(current->ipv4_addr));
                        cJSON_AddItemToObjectCS(root, "netmask", cJSON_CreateNumber(current->netmask));
                }
                char *output = cJSON_PrintUnformatted(root);
                cJSON_AddItemToArray(array, cJSON_CreateString(output));
                cJSON_Delete(root);
                free(output);
                current = current->next;
        }
        return 0;
}

char *generate_network_json_string(struct if_list **list)
{
	struct cJSON *root = cJSON_CreateObject();

	struct cJSON *ifobj = cJSON_CreateObject();
        struct cJSON *values = cJSON_CreateArray();
        if(populate_json_array_with_interfaces(values, list) == 0){
                cJSON_AddItemToObjectCS(ifobj, "value", values);
                cJSON_AddItemToObjectCS(root, "networkInterfaces", ifobj);
                char *output = cJSON_PrintUnformatted(root);
                cJSON_Delete(root);
                return output;
        }
        return NULL;
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