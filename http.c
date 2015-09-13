/*
 * file: http.c
 * description: deal everything with HTTP protocol
 * author: Lewis Cheng
 * date: 2011.09.14
 */
 
#include "uhttpd.h"

void free_packet(packet_type *packet) {
	assert(packet != NULL);

	xfree(packet->buf);
	xfree(packet);
}

packet_type *read_packet(int client_sock) {
	packet_type *packet = xmalloc(sizeof(packet_type));
	packet->buf = xmalloc(MAX_PACKET_SIZE);
	packet->size = xread(client_sock, packet->buf, MAX_PACKET_SIZE);
	if (packet->size <= 0)
		return NULL;
	return packet;
}

int write_packet(int client_sock, packet_type *packet) {
	assert(packet != NULL);
	
	ssize_t bytes_write = xwrite(client_sock, packet->buf, packet->size);
	if (bytes_write == packet->size)
		return 0;
	else
		return -1;
}

http_request_type *create_request_from_packet(const packet_type *packet) {
	assert(packet != NULL);
	
	http_request_type *request = xmalloc(sizeof(http_request_type));
	request->method = NULL;
	request->url = NULL;
	request->version = NULL;
	request->connection = NULL;
	request->content_type = NULL;
	request->content_length = 0;
	request->content = NULL;
	
	const char *ptr = packet->buf;
	// Parse method.
	request->method = xstralloc(MAX_METHOD_LENGTH);
	ptr = copy_to_stop_char(request->method, 0, ptr, ' ', MAX_METHOD_LENGTH);
	++ptr; // Skip the space.

	// Parse URL.
	request->url = xstralloc(MAX_URL_LENGTH);
	ptr = copy_to_stop_char(request->url, 0, ptr, ' ', MAX_URL_LENGTH);
	++ptr; // Skip the space.

	// Parse version.
	request->version = xstralloc(MAX_VERSION_LENGTH);
	ptr = copy_to_stop_char(request->version, 0, ptr, '\r', MAX_VERSION_LENGTH);
	ptr += 2; // Skip '\r\n'.
	
	// Parse header fields.
	char *field = xstralloc(MAX_HEADER_FIELD_LENGTH);
	while (*ptr != '\r') {
		ptr = copy_to_stop_char(field, 0, ptr, ':', MAX_HEADER_FIELD_LENGTH);
		ptr += 2; // Skip ": "

		char *value = xstralloc(MAX_HEADER_VALUE_LENGTH);
		ptr = copy_to_stop_char(value, 0, ptr, '\r', MAX_HEADER_VALUE_LENGTH);
		ptr += 2; // Skip "\r\n"

		if (!xstrcmp(field, "Content-Type", MAX_HEADER_FIELD_LENGTH)) {
			request->content_type = value;
		} else if (!xstrcmp(field, "Content-Length", MAX_HEADER_FIELD_LENGTH)) {
			request->content_length = atoi(value);
			xfree(value);
			request->content = xmalloc(request->content_length);
		} else if (!xstrcmp(field, "Connection", MAX_HEADER_FIELD_LENGTH)) {
			request->connection = value;
		} else {
			xfree(value);
		}
	}
	xfree(field);
	
	ptr += 2; // Skip the last "\r\n" of the header.
	
	memcpy(request->content, ptr, request->content_length);
	
	report_info("create_request_from_packet()",
		"\nMethod = %s\nURL = %s\nVersion = %s\nConnection = %s\nContent-Type = %s\nContent-Length = %u",
		request->method,
		request->url,
		request->version,
		request->connection,
		request->content_type,
		request->content_length);
		
	return request;
}

static void set_http_abnormal_status_response(http_request_type *request, http_response_type *response, int status_code) {
	response->version = "HTTP/1.1";
	response->connection = "close";
	response->content_type = "text/html";
	response->content = NULL;
	
	switch (status_code) {
	case 400:
		response->code_plus_desc = "400 Bad Request";
		response->content = xmalloc(MAX_INTERNAL_HTML_SIZE);
		snprintf(response->content, MAX_INTERNAL_HTML_SIZE,
			"<html>\r\n"
			"\t<body>\r\n"
			"\t\t<h1>%s</h1>\r\n"
			"\t\t<p>The URL '%s' you requested is not understandable by this server.</p>\r\n"
			"\t\t<hr>\r\n"
			"\t\t<p>If you have any questions, please contact the website administrator.</p>\r\n"
			"\t</body>\r\n"
			"</html>\r\n",
			response->code_plus_desc,
			request->url);
		break;
	case 404:
		response->code_plus_desc = "404 Not Found";
		response->content = xmalloc(MAX_INTERNAL_HTML_SIZE);
		snprintf(response->content, MAX_INTERNAL_HTML_SIZE,
			"<html>\r\n"
			"\t<body>\r\n"
			"\t\t<h1>%s</h1>\r\n"
			"\t\t<p>The URL '%s' you requested doesn't exist on  this server.</p>\r\n"
			"\t\t<hr>\r\n"
			"\t\t<p>If you have any questions, please contact the website administrator.</p>\r\n"
			"\t</body>\r\n"
			"</html>\r\n",
			response->code_plus_desc,
			request->url);
		break;
	case 500:
		response->code_plus_desc = "500 Internal Server Error";
		response->content = xmalloc(MAX_INTERNAL_HTML_SIZE);
		snprintf(response->content, MAX_INTERNAL_HTML_SIZE,
			"<html>\r\n"
			"\t<body>\r\n"
			"\t\t<h1>%s</h1>\r\n"
			"\t\t<p>This server has encountered an error internally, please try it later.</p>\r\n"
			"\t\t<hr>\r\n"
			"\t\t<p>If you have any questions, please contact the website administrator.</p>\r\n"
			"\t</body>\r\n"
			"</html>\r\n",
			response->code_plus_desc);
		break;
	case 501:
		response->code_plus_desc = "501 Not Implemented";
		response->content = xmalloc(MAX_INTERNAL_HTML_SIZE);
		snprintf(response->content, MAX_INTERNAL_HTML_SIZE,
			"<html>\r\n"
			"\t<body>\r\n"
			"\t\t<h1>%s</h1>\r\n"
			"\t\t<p>The method '%s' isn't supported by this server.</p>\r\n"
			"\t\t<hr>\r\n"
			"\t\t<p>If you have any questions, please contact the website administrator.</p>\r\n"
			"\t</body>\r\n"
			"</html>\r\n",
			response->code_plus_desc,
			request->method);
		break;
	case 505:
		response->code_plus_desc = "505 HTTP Version Not Supported";
		response->content = xmalloc(MAX_INTERNAL_HTML_SIZE);
		snprintf(response->content, MAX_INTERNAL_HTML_SIZE,
			"<html>\r\n"
			"\t<body>\r\n"
			"\t\t<h1>%s</h1>\r\n"
			"\t\t<p>This server only supports HTTP 1.0/1.1, but %s requested.</p>\r\n"
			"\t\t<hr>\r\n"
			"\t\t<p>If you have any questions, please contact the website administrator.</p>\r\n"
			"\t</body>\r\n"
			"</html>\r\n",
			response->code_plus_desc,
			request->version);
		break;
	}
	
	response->content_length = xstrlen(response->content, MAX_INTERNAL_HTML_SIZE);
}

static void handle_get_post(http_response_type *response, http_request_type *request) {
	char *path_buf = xstralloc(MAX_PATH_LENGTH);
	char *arg_str_buf = xstralloc(MAX_ARGUMENT_STRING_LENGTH);
	char *suffix_buf = xstralloc(MAX_SUFFIX_LENGTH);
	
	int status_code = 200;
	
	if (parse_url(request->url, path_buf, arg_str_buf, suffix_buf) == -1) {
		status_code = 400;
		goto finalize;
	}
	
	if (!xstrcmp(suffix_buf, "so", MAX_SUFFIX_LENGTH)) {
		// .so file.
		
		// Open module.
		handler_type handler = open_module(path_buf);
		if (handler == NULL) {
			status_code = 404;
			goto finalize;
		}
		
		// Check method to decide the source of the arguments.
		if (!xstrcmp(request->method, "GET", MAX_METHOD_LENGTH)) {
			// Arguments come from the URL.
		} else if (!xstrcmp(request->method, "POST", MAX_METHOD_LENGTH)) {
			xstrcpy(arg_str_buf, request->content, request->content_length);
		} else {
			status_code = 501;
			goto finalize;
		}
		
		// Create argument list.
		arg_list_type list = create_arg_list(arg_str_buf);
		if (list == NULL) {
			status_code = 400;
			goto finalize;
		}
		
		// Call .so
		response->content = xmalloc(MAX_INTERNAL_HTML_SIZE);
		ssize_t count = (*handler)(response->content, list);
		free_arg_list(list);
		
		// Check return and set content-length.
		if (count == -1) {
			status_code = 500;
			goto finalize;
		}
		response->content_length = count;
	
		// Fill other fields.
		response->code_plus_desc = "200 OK";
		response->version = "HTTP/1.1";
		response->connection = xstrcmp(request->connection, "close", MAX_VERSION_LENGTH) ? "Keep-Alive" : "close";
		response->content_type = get_mime_type("html");
	} else {
		// Request regular file.
		
		// Check if the requested file exists.
		FILE *strm = fopen(path_buf, "rb");
		if (strm == NULL) {
			status_code = 404;
			goto finalize;
		}
		
		// Content-Length.
		fseek(strm, 0, SEEK_END);
		response->content_length = ftell(strm);
		response->content = xmalloc(response->content_length);
		
		// Content.
		fseek(strm, 0, SEEK_SET);
		fread(response->content, response->content_length, 1, strm);
		
		// Other fields.
		response->code_plus_desc = "200 OK";
		response->version = "HTTP/1.1";
		response->connection = xstrcmp(request->connection, "close", MAX_CONNECTION_LENGTH) ? "Keep-Alive" : "close";
		response->content_type = get_mime_type(suffix_buf);
	}

finalize:
	if (status_code != 200)
		set_http_abnormal_status_response(request, response, status_code);
	xfree(path_buf);
	xfree(arg_str_buf);
	xfree(suffix_buf);
}

http_response_type *handle_request(http_request_type *request) {
	http_response_type *response = xmalloc(sizeof(http_response_type));

	// Check HTTP version.
	if (xstrcmp(request->version, "HTTP/1.1", MAX_VERSION_LENGTH) && xstrcmp(request->version, "HTTP/1.0", MAX_VERSION_LENGTH)) {
		// The opposite side will not use the right version of HTTP so exit.
		set_http_abnormal_status_response(request, response, 505);
		return response;
	}
		
	// Handle method.
	handle_get_post(response, request);
	
	return response;
}

packet_type *create_packet_from_response(http_response_type *response) {
	packet_type *packet = xmalloc(sizeof(packet_type));
	packet->buf = xmalloc(MAX_PACKET_SIZE);

	packet->size = snprintf(packet->buf, MAX_PACKET_SIZE,
		"%s %s\r\n"
		"Server: %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %lu\r\n"
		"Connection: %s\r\n\r\n",
		response->version,
		response->code_plus_desc,
		server_name,
		response->content_type,
		response->content_length,
		response->connection);
		
	if (packet->size + response->content_length > MAX_PACKET_SIZE) {
		report_error("create_packet_from_response()", "Packet size will be truncated to MAX_PACKET_SIZE");
		response->content_length = MAX_PACKET_SIZE - packet->size;
	}
	
	memcpy(packet->buf + packet->size, response->content, response->content_length);
	packet->size += response->content_length;
	
	report_info("create_packet_from_response()",
		"\nServer = %s\nResponse = %s\nVersion = %s\nConnection = %s\nContent-Type = %s\nContent-Length = %u",
		server_name,
		response->code_plus_desc,
		response->version,
		response->connection,
		response->content_type,
		response->content_length);
	
	return packet;
}

void free_request(http_request_type *request) {
	xfree(request->method);
	xfree(request->url);
	xfree(request->version);
	xfree(request->connection);
	xfree(request->content_type);
	xfree(request->content);
	xfree(request);
}

void free_response(http_response_type *response) {
	xfree(response->content);
	xfree(response);
}

