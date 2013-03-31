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
typedef struct HOST {

};

unsigned int interval = 25 * 100;
int generate_list = 0;

void usage(void);

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



}

void usage(void) {
	fprintf(stderr, "Usage: ./a.out [options] [targets...]\n");
	fprintf(stderr, "-i n\t interval between sending ping packets(in millisec), (default 25)\n");
	fprintf(stderr, "-g\tgenerate target list\n");
	exit (1);
}
