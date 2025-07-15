#include "ubus_wrapper.h"
#include "utils.h"
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h>

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	__MEMORY_MAX,
};

enum {
	MEMORY_DATA,
	__INFO_MAX,
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY]  = { .name = "free", .type = BLOBMSG_TYPE_INT64 }
};

static void memory_stats_callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
        (void) type; // Unused variable
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

        data->total    = blobmsg_get_u64(memory[TOTAL_MEMORY]);
	data->free     = blobmsg_get_u64(memory[FREE_MEMORY]);
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