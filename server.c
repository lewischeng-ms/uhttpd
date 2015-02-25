/*
 * file: server.c
 * description: server and worker implementation
 * author: Lewis Cheng
 * date: 2011.09.14
 */

#include "uhttpd.h"

in_addr_t server_address = INADDR_ANY;
uint16_t server_port = 80;
char *server_name = "uHttpd";
char *default_html_name = "default.html";
char *document_root = "./";

static void stop_process(int sig) {
	report_fatal("stop_process()", "Signal '%s' caught, process(pid = %u) stopping", strsignal(sig), getpid());
}

static void show_xmval(int sig) {
	report_info("show_xmval()", "\nIf the worker is suspended, the following value should be 2\n#xmalloc() - #xfree() = %d", get_xmval());
}

static void worker(int client_sock, int server_sock) {
	report_info("worker()", "Worker forked with pid = %u", getpid());
	close(server_sock);
			
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR ||
		signal(SIGINT, stop_process) == SIG_ERR ||
		signal(SIGTERM, stop_process) == SIG_ERR ||
		signal(SIGTSTP, show_xmval) == SIG_ERR)
        report_fatal("worker()", "Fail to setup signals");

	while (1) {
		// read packet
		packet_type *packet = read_packet(client_sock);
		if (packet == NULL)
			report_fatal("worker()", "Connection closed");
		
		// parse request
		http_request_type *request = create_request_from_packet(packet);
		free_packet(packet);
		
		// handle request
		http_response_type *response = handle_request(request);
		
		// create packet
		packet = create_packet_from_response(response);
		
		// write packet
		if (write_packet(client_sock, packet) == -1)
			report_fatal("worker()", "Connection closed");
		
		// keep-alive
		if (request->connection != NULL && !xstrcmp(request->connection, "close", MAX_CONNECTION_LENGTH))
			report_fatal("worker()", "Connection closed");
		
		free_packet(packet);
		free_request(request);
		free_response(response);
	}
}

void start_server() {
	// Setup signals.
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR ||
		signal(SIGINT, stop_process) == SIG_ERR ||
		signal(SIGTERM, stop_process) == SIG_ERR ||
		signal(SIGTSTP, SIG_IGN) == SIG_ERR)
        report_fatal("start_server()", "Fail to setup signals");

	// Create server socket.
	int server_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sock == -1)
		report_fatal("start_server()", "Fail to create server socket");
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = server_address;
	memset(server_addr.sin_zero, 0, 8);
	
	// Bind server socket to server_address.
	if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1)
		report_fatal("start_server()", "Fail to bind server socket");
	
	// Listen on server_port.
	if (listen(server_sock, MAX_PENDING_CONNECTIONS) == -1)
		report_fatal("start_server()", "Fail to listen server socket");

	// Main loop of server.
	while (1) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len;
		
		// Accept a client connection.
		int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_sock == -1) {
			report_error("start_server()", "Client socket is invalid");
			continue;
		}
		
		if (fork() == 0) {
			// Call worker to do succeeding work.
			worker(client_sock, server_sock);
		} else {
			// Server doesn't need talk to client.
			close(client_sock);
		}
	}
}
