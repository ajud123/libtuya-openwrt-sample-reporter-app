#include "net_interfaces.h"
#include <uci.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void add_interface(struct if_list **list, struct if_list *interface)
{
	struct if_list *current = *list;
	if (current == NULL) {
		*list = interface;
		return;
	}

	struct if_list *last = current;

	for (; current != NULL; current = current->next) {
		if (current->name == interface->name)
			return;
		last = current;
	}
	if (last != NULL)
		last->next = interface;
}

void report_error(struct uci_context *ctx, char *prefix)
{
	char *msg;
	uci_get_errorstr(ctx, &msg, prefix);
	syslog(LOG_ERR, "%s", msg);
	puts(msg);
	free(msg);
}

static void iterate_net_sections(struct uci_package *p, struct if_list **list)
{
	struct uci_element *e;

	uci_foreach_element (&p->sections, e) {
		struct uci_section *s = uci_to_section(e);
		if (strcmp(s->type, "interface") == 0) {
			if (strstr(e->name, "6") == NULL && strcmp("loopback", e->name) != 0) {
				struct if_list *new = malloc(sizeof(struct if_list));
				new->name	    = strdup(e->name);
				new->ipv4_addr	    = NULL;
				new->netmask	    = 0;
				new->next	    = NULL;
				add_interface(list, new);
			}
		}
	}
}

int get_network_interfaces(struct if_list **list)
{
	struct uci_context *ctx = uci_alloc_context();
	if (ctx == NULL) {
		report_error(ctx, "UCI failed to allocate context: ");
		return 1;
	}
	struct uci_package *netpkg = NULL;
	if (uci_load(ctx, "network", &netpkg)) {
		report_error(ctx, "UCI failed to load network package: ");
		return 2;
	}

	iterate_net_sections(netpkg, list);

	uci_free_context(ctx);
	return 0;
}

void delete_interface_list(struct if_list **list)
{
	struct if_list *next	= NULL;
	struct if_list *current = *list;
	while (current != NULL) {
		next = current->next;
		if (current->name != NULL)
			free(current->name);
		if (current->ipv4_addr != NULL)
			free(current->ipv4_addr);
		free(current);
		current = next;
	}
	*list = NULL;
}
