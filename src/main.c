
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/****************************************Defines***********************************************/
#define PING_SUCCESS 1
#define PING_FAILURE 0

#define try_send_command(cmd) \
res = send(sock, cmd, sizeof(cmd), 0);\
if (print)\
printf("client: %s", cmd);\
res = recv(sock, reply_packet, sizeof(reply_packet), 0);\
if (res < 0) {\
	printf("no reply!\n");\
	return PING_FAILURE;\
}\
if (print)\
printf("server: %s", reply_packet);
#define try_send_commandsz(cmd, size) \
res = send(sock, cmd, size, 0);\
if (print)\
printf("client: %s", cmd);\
res = recv(sock, reply_packet, sizeof(reply_packet), 0);\
if (res < 0) {\
	printf("no reply!\n");\
	return PING_FAILURE;\
}\
if (print)\
printf("server: %s", reply_packet);
/*********************************************************************************************/



/***********************************Global Variables******************************************/
SOCKET sock = 0;
char reply_packet[0xFFFF];
struct addrinfo hints;
/********************************************************************************************/



/****************************************Functions*********************************************/
/// <summary>
/// ping a host
/// </summary>
/// <param name="addr">>address of the host to ping</param>
/// <param name="ttl">time-to-live value</param>
/// <param name="timeout">timeout time in miliseconds</param>
/// <param name="maxpackets">maximum number of packets to send</param>
/// <param name="print"> ==1 print sent messages and replies, !=1 do not print sent messages and replies</param>
/// <returns>ping result</returns>
int ping(const char* addr, int ttl, int timeout, int print);
void initialize();
void dump();
/*********************************************************************************************/



/*******************************************Main Function****************************************/
int main(int argc, char* argv[])
{
	initialize();

	if (argc < 2)
	{
		printf("usage: ping-smtp [print] <servers>\n\n\tspecify 'print' to print sent and received packets.\n");
		exit(-1);
	}

	int args_count = argc - 1;
	int server_count;
	int print;
	char** servers;
	
	if (strcmp(argv[1], "print") == 0)
	{
		server_count = args_count - 1;

		if (!server_count)
		{
			printf("no servers specified!\n");
			exit(-1);
		}

		print = 1;
		servers = argv + 2;
	}
	else
	{
		server_count= args_count;
		print = 0;
		servers = argv + 1;
	}

	// create an array for ping results
	int* results = malloc(server_count * sizeof(int));

	// ping every server listed in the command line
	for (int i = 0; i < server_count; i++)
	{
		// we want to ping our server with 64 ttl, 4s timeout time and four packets
		// you can change these values if you want
		results[i] = ping(servers[i], 64, 4000, print);
	}

	// evaluate the results
	printf("\nping results:\n");

	int num_successes = 0;

	for (int i = 0; i < server_count; i++)
	{
		if (results[i] == PING_SUCCESS)
		{
			printf("%s passed.(+)\n", servers[i]);
			num_successes++;
		}
		else
			printf("%s failed.(-)\n", servers[i]);
	}
	
	printf("\nopenrelay SMTP servers found: %d out of %d\n", num_successes, server_count);

	dump();
}
/************************************************************************************************/



void initialize()
{
	// initialize Winsock with version 2.2
	// this will allow us to use the windows socket functions
	WSADATA wsaData = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// initialize the hints for resolving hosts
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}

void dump()
{
	// uninitialize 
	WSACleanup();
}

int ping(const char* addr, int ttl, int timeout, int print)
{
	int res = 0, helo_size = 0;
	u_long nonblock_mode = 1;
	char helo[64];
	struct addrinfo* host = NULL;
	struct timeval timeout_eval;

	// "setsockopt" processes tv_sec in miliseconds
	// "select" processes tv_sec in seconds
	timeout_eval.tv_sec = timeout;
	timeout_eval.tv_usec = 0;

	// format our helo command
	helo_size = sprintf(helo, "HELO %s\r\n", addr) + 1;

	getaddrinfo(addr, "smtp", &hints, &host);

	if (host)
	{
		// check if addr is a domain
		if (isalpha(addr[0]))
			printf("\npinging %s [%s]:\n", addr, inet_ntoa(((struct sockaddr_in*)(host->ai_addr))->sin_addr));
		else
			printf("\npinging %s:\n", addr);

		// create our socket
		sock = socket(host->ai_family, host->ai_socktype, host->ai_protocol);

		// set ttl value
		setsockopt(sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl));
		// set timeout time
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_eval, sizeof(timeout_eval));
		// disable blocking mode for connection timeout
		ioctlsocket(sock, FIONBIO, &nonblock_mode);

		// connect to our server
		res = connect(sock, host->ai_addr, host->ai_addrlen);

		// check if connection failed or we are still trying to connect
		if (res < 0)
		{
			res = WSAGetLastError();

			// we are still trying to connect
			if (res == WSAEINPROGRESS || res == WSAEWOULDBLOCK) {

				fd_set wait_set;
				FD_ZERO(&wait_set);
				FD_SET(sock, &wait_set);

				// "select" proccesses tv_sec in seconds
				timeout_eval.tv_sec = timeout / 1000;

				// set connection timeout time
				res = select(sock + 1, 0, &wait_set, 0, &timeout_eval);

				// if select return 0 that means connection has timed out
				if (res == 0)
				{
					printf("connection timeout!\n");
					closesocket(sock);
					return PING_FAILURE;
				}
			}
			else
			{
				// connection failed
				printf("connection failed!\n");
				closesocket(sock);
				return PING_FAILURE;
			}
		}

		// reenable blocking mode
		nonblock_mode = 0;
		ioctlsocket(sock, FIONBIO, &nonblock_mode);

		res = recv(sock, reply_packet, sizeof(reply_packet), 0);

		if (res > 0)
		{
			if (print)
				printf("server: %s", reply_packet);
		}

		freeaddrinfo(host);
	}
	else
	{
		printf("\ncould not resolve %s!\n", addr);
		return PING_FAILURE;
	}

	// send helo
	try_send_commandsz(helo, helo_size);
	// send email
	try_send_command("MAIL FROM:<test@example.com>\r\n");

	// check if server doesn't proceed
	if (memcmp(reply_packet, "250", 3) != 0)
	{
		printf("server requires authentication!\n");
		return PING_FAILURE;
	}

	printf("success!\n");

	// diconnect
	closesocket(sock);

	return PING_SUCCESS;
}
