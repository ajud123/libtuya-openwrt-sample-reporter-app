#include "ubus_wrapper.h"
#include "utils.h"
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h>

// Base structure of an interface
enum {
	IPV4_ADDR_ARR,
	__INTERFACE_MAX,
};

const struct blobmsg_policy interface_policy[__INTERFACE_MAX] = {
	[IPV4_ADDR_ARR] = { .name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY }, // ipv4 address list
};

// Base structure of an ipv4 address list object
enum {
	ADDR,
	MASK,
	__ADDR_OBJ_MAX,
};

const struct blobmsg_policy address_policy[__ADDR_OBJ_MAX] = {
	[ADDR] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
	[MASK] = { .name = "mask", .type = BLOBMSG_TYPE_INT32 },
};

// Base structure of the system info message

enum {
	MEMORY_DATA,
	__INFO_MAX,
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

// Base structure of the memory table of the system info message

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	__MEMORY_MAX,
};

const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY]  = { .name = "free", .type = BLOBMSG_TYPE_INT64 }
};

static void memory_stats_callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	(void)type; // Unused variable
	struct memory_stats *data = (struct memory_stats *)req->priv;

	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (tb == NULL) {
		syslog(LOG_ERR, "Didn't receive any system status data.");
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]),
		      blobmsg_data_len(tb[MEMORY_DATA]));

	if (memory == NULL) {
		syslog(LOG_ERR, "Didn't receive any memory data.");
		return;
	}

	data->total = blobmsg_get_u64(memory[TOTAL_MEMORY]);
	data->free  = blobmsg_get_u64(memory[FREE_MEMORY]);
}

static void interface_status_callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	syslog(LOG_ALERT, "Received response");
	(void)type; // Unused variable
	struct if_list *data = (struct if_list *)req->priv;

	struct blob_attr *addrList[__INTERFACE_MAX];
	int rem;
	syslog(LOG_ALERT, "Parsing using the interface policy");
	blobmsg_parse(interface_policy, __INTERFACE_MAX, addrList, blob_data(msg), blob_len(msg));

        
	if (addrList[IPV4_ADDR_ARR]) {
                struct blob_attr *cur;
		blobmsg_for_each_attr (cur, addrList[IPV4_ADDR_ARR], rem) {
			struct blob_attr *address[__ADDR_OBJ_MAX];
			if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE)
				continue;
			syslog(LOG_ALERT, "Parrsing array objects...");
			blobmsg_parse(address_policy, __ADDR_OBJ_MAX, address, blobmsg_data(cur),
				      blobmsg_len(cur));
			if (address[ADDR] && address[MASK]) {
                                data->ipv4_addr = strdup(blobmsg_get_string(address[ADDR]));
				uint32_t bits = blobmsg_get_u32(address[MASK]);
                                data->netmask = bits;
			}

		}
	} else {
        	syslog(LOG_ALERT, "Given interface does not have an ipv4-address array");
        }
}

/*
 * Polls the ubus 'system' object for memory information
 * Returns 0 if successful
 * Returns 1 if *out is NULL
 * Returns a ubus return code otherwise
 */
int get_memory_statistics(struct ubus_context *ctx, struct memory_stats *out)
{
	syslog(LOG_INFO, "Querying memory statistics via Ubus...");
	if (out == NULL) {
		syslog(LOG_ERR, "Argument for memory statistics was passed as NULL.");
		return 1;
	}
	uint32_t id = 0;
	int rc	    = ubus_lookup_id(ctx, "system", &id);
	if (rc) {
		syslog(LOG_ERR, "Could not find object 'system'\n");
		return rc;
	}
	rc = ubus_invoke(ctx, id, "info", NULL, memory_stats_callback, out, 3000);
	return rc;
}

/*
 * Polls the ubus 'network.interface' object for network interface information
 * Returns 0 if successful
 * Returns 1 if *out is NULL
 * Returns a ubus return code otherwise
 */

int get_network_statistics(struct ubus_context *ctx, struct if_list *out)
{
	syslog(LOG_INFO, "Querying network interfaces via Ubus...");
	if (out == NULL) {
		syslog(LOG_ERR, "Argument for network statistics was passed as NULL.");
		return 1;
	}
	uint32_t id    = 0;
	char *queryStr = malloc(512 * sizeof(char));
	sprintf(queryStr, "network.interface.%s", out->name);
	int rc = ubus_lookup_id(ctx, queryStr, &id);
	if (rc) {
		syslog(LOG_ERR, "Could not find object '%s'\n", queryStr);
		return rc;
	}
	syslog(LOG_ALERT, "Invoking big command");
	rc = ubus_invoke(ctx, id, "status", NULL, interface_status_callback, out, 3000);

	return rc;
}
