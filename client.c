//Klayton DalPra
//client file. connects to the server to send messages

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define SERVER_IP "127.0.0.1" //192.168.0.1 didnt work for me
#define SERVER_PORT 8080

int main(int argc, char *argv[]){
	int sock_send;
	int send_len, bytes_sent;
	char text[80], buf[BUF_SIZE];
	struct sockaddr_in addr_send;

	sock_send = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock_send < 0) {
		perror("socket() failed");
		exit(1);
	}

	memset(&addr_send, 0, sizeof(addr_send));
	addr_send.sin_family = AF_INET;
	addr_send.sin_addr.s_addr = inet_addr(SERVER_IP);
	addr_send.sin_port = htons((unsigned short)SERVER_PORT);
	if(connect(sock_send, (struct sockaddr *) &addr_send, sizeof(addr_send)) < 0) {
		perror("connect() failed");
		close(sock_send);
		exit(1);
	}
	while (1) {
		printf("Send? ");
		scanf("%s", text);
		if(strcmp(text, "quit") == 0)
			break;

		strcpy(buf, text);
		send_len = strlen(text);
		bytes_sent = send(sock_send, buf, send_len, 0);
	}

	close(sock_send);
	return 0;
}
