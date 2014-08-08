#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define		SERV_PORT	3422		// USB over IP
#define		SERVERHOST	"10.0.21.200"

#define		BUFSIZ		1024

int
main(int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_in	servaddr;
	int	n, i, j, col;
	int	addr, data;
	char sendline[BUFSIZ], recvline[BUFSIZ + 1];

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_aton(SERVERHOST, &servaddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	sendline[ 0] = 0xa1;
	sendline[ 1] = 0x11;

	addr = 0xf0000000;
	data = 0x40070000;

	for ( j=1 ; j<16 ; ++j) {
		col = (j >> 1) << ((1-(j & 1))*4);
		for (i = 0; i <= 512; ++i ) {
			addr = 0xf0000000 + i * 8;
			data = ((i+(j<<2)) & 0xff) << 24 | col << 16;
			*(int *)(&sendline[2]) = htonl( addr );
			*(int *)(&sendline[6]) = data;
	
			if (sendto(sockfd, sendline, 10, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
				perror("sendto()");
			}
			usleep(100);
		}
	}

	exit(EXIT_SUCCESS);
}

