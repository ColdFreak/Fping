#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <strings.h> /* bzero() */
#include <string.h>
#include <arpa/inet.h> /* inet_ntop()*/
#include <netinet/ip_icmp.h> /* struct icmp */
#include <sys/time.h>
#include <netinet/in.h> /* struct sockaddr_in */
#include <unistd.h>

#define MIN_INTERVAL	10
typedef struct host_entry {


}HOST_ENTRY;

/* global */
HOST_ENTRY **table = NULL; /*array of pointers to items in the list */

/* global stats*/
int num_hosts; /* total number of hosts */
/* -i and -g options */
unsigned int interval = 25 * 100;
int generate_list = 0;

void usage(void);
void add_cidr(char *addr);
void add_name(char *name);

int main(int argc, char **argv) {
	int c;
	while ((c = getopt(argc, argv, "gi:")) != EOF) {
		switch(c) {
			case 'i':
				if( !(interval = (unsigned int)atoi(optarg)*100))
				   usage();
				break;
			case 'g':
				generate_list = 1;
				break;
			default:
				usage();
				break;
		}
	}

	/* if getuid() => 1000(normal user), dangerous!! 
	 * if getuid() => 0( root), OK!!
	 */
	if(interval < MIN_INTERVAL && getuid()) {
		fprintf(stderr, "Thest options are too risky for mere mortals.\n");
		fprintf(stderr, "You nedd i>= 10\n");
		usage();
	}

	argv = &argv[optind];
	argc = argc - optind;

	if ( *argv && generate_list ) {
		if(argc == 1)
			add_cidr(argv[0]);
//		if(argc == 2) {
//			add_range(argv0, argv[1]);
		else
			usage();
	}

	table = (HOST_ENTRY **)malloc(sizeof(HOST_ENTRY *) * num_hosts);
	if(! table) {
	   perror("Can't malloc array of pointers in the list");
	   exit (1);
	}





}

void usage(void) {
	fprintf(stderr, "Usage: ./a.out [options] [targets...]\n");
	fprintf(stderr, "-i n\t interval between sending ping packets(in millisec), (default 25)\n");
	fprintf(stderr, "-g\tgenerate target list\n");
	exit (1);
}



void add_cidr(char *addr) {
	char *addr_end;
	char *mask_str;
	unsigned long mask;

	int ret;
	struct addrinfo addr_hints;
	struct addrinfo *addr_res;

	unsigned long net_addr;
	unsigned long net_last;
	unsigned long long bitmask;
	/* find the '/' symbol in 192.168.1.0/24 */
	addr_end = strchr(addr, '/');
	if(addr_end == NULL)
		usage();
	/* 192.168.1.0/24 became 192.168.1.0 */
	*addr_end = '\0';
	mask_str = addr_end+1;
	mask = atoi(mask_str);
	if(mask < 1 || mask > 30 ){
		fprintf(stderr, "Error, netmask must be between 1 and 30(is: %s)\n",mask_str);
		exit (1);
	}
	
	/* clear addr_hints */
	memset(&addr_hints, 0, sizeof(struct addrinfo));
	addr_hints.ai_family = AF_UNSPEC;
	addr_hints.ai_flags = AI_NUMERICHOST;
	ret = getaddrinfo(addr, NULL, &addr_hints, &addr_res);
	if(ret) {
		fprintf(stderr, "Error: can't parse address %s: %s\n",addr, gai_strerror(ret));
		exit (1);
	}
	
	/* uint32_t ntohl (uint32_t netlong); */
	net_addr = ntohl(((struct sockaddr_in *)addr_res->ai_addr)->sin_addr.s_addr);
	bitmask = 0xFFFFFFFF << (32 - mask);
	net_addr &= bitmask;
	net_last = net_addr + ((unsigned long)0x1 << (32-mask)) -1;
	while(++net_addr < net_last) {
		struct in_addr in_addr_tmp;
		/* fill in with 192.168.1.24 */
		char buffer[20];
		/*uint32_t htonl(uint32_t hostlong);
		 * struct in_addr {
		 *		unsigned long s_addr;
		 * }
		 */
		in_addr_tmp.s_addr = htonl(net_addr);
		inet_ntop(AF_INET, &in_addr_tmp, buffer, sizeof(buffer));
		addr_name(buffer);
	}
	freeaddrinfo(addr_res);
}

