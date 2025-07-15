#ifndef TUYAREPORTER_UBUS_WRAPPER
#define TUYAREPORTER_UBUS_WRAPPER
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "utils.h"

int get_memory_statistics(struct ubus_context *ctx, struct memory_stats *out);

#endif