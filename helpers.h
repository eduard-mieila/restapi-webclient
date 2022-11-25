// Credits: Lab 10 - PCom

#ifndef _HELPERS_
#define _HELPERS_


// Src: https://ocw.cs.pub.ro/courses/so/laboratoare/resurse/die
#include <errno.h>
#define DIE(assertion, call_description)				\
	do {								                \
		if (assertion) {					            \
			fprintf(stderr, "(%s, %d): ",			    \
					__FILE__, __LINE__);		        \
			perror(call_description);			        \
			exit(errno);					            \
		}							                    \
	} while (0)


// Dimensiuni buffere
#define BUFLEN 4096
#define LINELEN 1000
#define USERLEN 100
#define MSG_LEN 8192
#define STR_LEN 100

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

#endif
