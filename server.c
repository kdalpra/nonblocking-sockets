//Klayton DalPra
//server file. accepts multiple clients using non blocking sockets so multiple clients can send messages at the same time.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define LISTEN_PORT 8080

void set_non_blocking(int sock) {
	int opts;
	opts = fcntl(sock, F_GETFL); //current socket flag
	if(opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(0);
	}
	opts = opts | O_NONBLOCK; //adds the non blocking flag
	if (fcntl(sock, F_SETFL, opts) < 0) { //sets the new flag
		perror("fcntl(F_SETFL)");
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	int sock_listen, sock_recv;
	struct sockaddr_in my_addr, recv_addr;
	int i, addr_size;
	char buf[BUF_SIZE];
	fd_set master_fds, read_fds; //file descriptors for select
	int fdmax; //max descriptor

	//makes a tcp socket
	sock_listen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock_listen < 0) {
		perror("socket() failed\n");
		exit(0);
	}

	//makes the socket non blocking so concurrent connections can send bytes
	set_non_blocking(sock_listen);

	//server memory address structure setup
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons((unsigned short)LISTEN_PORT);

	//bind socket to address structure
	i = bind(sock_listen, (struct sockaddr *) &my_addr, sizeof(my_addr));
	if(i < 0){
		perror("bind() failed\n");
		close(sock_listen);
		exit(0);
	}
	//have the newly made socket listen for incoming connections
	i = listen(sock_listen,5);
	if(i < 0){
		perror("listen() failed\n");
		close(sock_listen);
		exit(0);
	}

	FD_ZERO(&master_fds);//master file descriptor list
	FD_ZERO(&read_fds);
	FD_SET(sock_listen, &master_fds);//adds listening socket to master file descriptor
	fdmax = sock_listen;//largest file descriptor = listening socket

	int not_shutdown = 1;//loop variable, changed when a shutdown message is received

	while(not_shutdown){
		//listens for activity on the socket
		read_fds = master_fds;
		if(select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(0);
		}

		//loops through all file descriptors
		for(int i = 0; i <= fdmax; i++) {
			//checks iof the descriptor is ready
			if(FD_ISSET(i, &read_fds)) {
				//connection
				if(i == sock_listen) {
					addr_size = sizeof(recv_addr);
					sock_recv = accept(sock_listen, (struct sockaddr *)&recv_addr, &addr_size);//allows the new connection to be added
					if (sock_recv < 0) {
						perror("Accept error");
					}
					else {
						set_non_blocking(sock_recv);//the new connection socket is set to non blocking also
						FD_SET(sock_recv, &master_fds);//this adds the new socket to the master file descriptor list
						//updates the max file descriptor
						if (sock_recv > fdmax) {
							fdmax = sock_recv;
						}
						printf("Client connected\n");
					}
				}
				else {
					int nbytes = recv(i, buf, sizeof(buf), 0); // reads the data from the socket
					if(nbytes <= 0) {
						if(nbytes == 0) {
							printf("Client disconnected\n");
						}
						else {
							perror("recv");
						}
						close(i);//closes the socket
						FD_CLR(i, &master_fds);//removes the socket from the master file descriptor
					}

					//valid message was sent
					else {
						buf[nbytes] = '\0';
						//display the message that was received
						printf("Message: %s\n", buf);
						if (strcmp(buf, "shutdown") == 0){
							//change the loop variable to "false" if shutdown was received to stop the loop
							not_shutdown = 0;
						}
					}
				}
			}
		}
	}
	//closes the sockets
	for(int i = 0; i <= fdmax; i++) {
		if(FD_ISSET(i, &master_fds)) {
			close(i);
		}
	}
	close(sock_listen);//close the socket
	return 0;
}
