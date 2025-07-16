#ifndef TUYAREPORTER_NET_INTERFACES
#define TUYAREPORTER_NET_INTERFACES

struct if_list {
	char *name;
        char *ipv4_addr;
        int netmask;
	struct if_list *next;
};

int get_network_interfaces(struct if_list **list);
void delete_interface_list(struct if_list **list);

#endif