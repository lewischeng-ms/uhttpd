/*
 * file: main.c
 * description: application interface(parse args, show help and so on)
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

static char *help_content =
	"uHttpd Version 1.00 Powered by Lewis Cheng (2011.09)\n\n"
	"Usage: %s [-a | --address IP] [-p | --port Port]\n\n"
	"Spec:\n"
	"IP:\t\t\tDefault value is ANY.\n"
	"Port:\t\t\tDefault value is 80.\n"
	"Hotkey:\n"
	"Ctrl-C:\t\t\tStop the server and all workers.\n"
	"Ctrl-Z:\t\t\tShow memory allocation statistics.\n";

static void parse_arg_list(int, char *[]);

int main(int argc, char *argv[]) {
	parse_arg_list(argc, argv);
	open_logfile(NULL);
	report_info("main()",
		"About Server:\nName: %s\nPID: %d\nAddress: %s\nPort: %d",
		server_name, getpid(), inet_ntoa(*(struct in_addr *)&server_address), server_port);
	
	start_server();
	return 0;
}

static void print_help_exit(char *name) {
	printf(help_content, strrchr(name, '/') + 1);
	exit(0);
}

static void parse_arg_list(int argc, char *argv[]) {
	int i;
	for (i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			print_help_exit(argv[0]);
		} else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--address"))	{	
			if (++i == argc) {
				fprintf(stderr, "IP is missing.\n");
				print_help_exit(argv[0]);
			}
			if (strcmp(argv[i], "ANY")) {
				server_address = inet_addr(argv[i]);
				if (server_address == INADDR_NONE) {
					fprintf(stderr, "IP is wrong.\n");
					print_help_exit(argv[0]);
				}
			}
		} else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) {
			if (++i == argc) {
				fprintf(stderr, "Port is missing.\n");
				print_help_exit(argv[0]);
			}
			server_port = atoi(argv[i]);
			if (server_port < 1024 && server_port != 80) {
				fprintf(stderr, "Port is out of valid range(=80 or >1023).\n");
				print_help_exit(argv[0]);
			}
		} else {
			fprintf(stderr, "Unknown argument: %s.\n", argv[i]);
			print_help_exit(argv[0]);
		}
	}
}
