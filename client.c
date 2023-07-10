/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFF_SIZE 1024

// Check input is vaild ip or not
int checkValidIp(const char *ip_str)
{
	struct in_addr addr;
	if (inet_pton(AF_INET, ip_str, &addr) == 1)
	{
		return 1; // valid IPv4 address
	}
	else if (inet_pton(AF_INET6, ip_str, &addr) == 1)
	{
		return 1; // valid IPv6 address
	}
	return 0; // not a valid IP address
}
void *receiveMessages(void *arg)
{
	int client_sock = *(int *)arg;
	char buff[BUFF_SIZE + 1];
	int bytes_received;

	while (1)
	{
		memset(buff, '\0', (strlen(buff) + 1));

		bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0); // Get message from server

		if (bytes_received < 0)
		{
			perror("Error: ");
			close(client_sock);
			pthread_exit(NULL);
		}
		buff[bytes_received] = '\0';

		// Display message from server
		printf("%s\n", "-----------------------------");
		printf("%s\n", buff);
		printf("%s\n", "-----------------------------");
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("%s\n", "Wrong number of arguments!");
		exit(0);
	}
	if (!checkValidIp(argv[1]))
	{
		printf("%s\n", "Invalid IP Address!");
		exit(0);
	}
	pthread_t receive_thread;

	// TCP
	int client_sock;
	int serv_port = atoi(argv[2]);
	char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;

	// Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serv_port);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	printf("Port:%d, IP:%s\n", serv_port, argv[1]);

	// Step 3: Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
	if (pthread_create(&receive_thread, NULL, receiveMessages, &client_sock) != 0)
	{
		printf("Error creating receive thread.\n");
		close(client_sock);
		return 0;
	}
	printf("Enter username:(Press Enter to exit):\n");
	while (1)
	{
		memset(buff, '\0', (strlen(buff) + 1));
		fgets(buff, BUFF_SIZE, stdin);
		buff[strlen(buff) - 1] = '\0';
		if (strlen(buff) == 0) // Client exit
		{
			printf("Client closed by user\n!");
			bytes_sent = send(client_sock, "", 0, 0); // Send close connection signal to server
			break;
		}
		// strcpy(buff, "REQ_LOCA{\"locationId\": 3}");

		bytes_sent = send(client_sock, buff, strlen(buff), 0); // Send message to server
		if (bytes_sent < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		} else {
			printf("Send %s to server with %d\n", buff, bytes_sent);
		}

		printf("Waiting for server...\n");
		bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0); // Get message from server
		if (bytes_received < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		buff[bytes_received] = '\0';
		// Dislay message from server
		printf("%s\n", "-----------------------------");
		printf("%s\n", buff);
		printf("%s\n", "-----------------------------");
	}
	close(client_sock);
	return 0;
}