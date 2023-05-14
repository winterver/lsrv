#include <winsock2.h>
#include <afunix.h>
#include <stdio.h>
#include <string.h>
#include <thread>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("usage: clnt <path>\n");
		return -1;
	}

	if (strlen(argv[1]) >= UNIX_PATH_MAX)
	{
		printf("path too long\n");
		return -1;
	}

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	int err;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
	err = connect(server, (struct sockaddr*) &addr, sizeof(addr));	
	
	if (err < 0)
	{
		printf("invalid port\n");
		return -1;
	}

	// send verification request
	send(server, "VERIFY", 6, 0);

	printf("info: connected to server\n");

	int run = true;
	std::thread([&](){
		while(run)
		{
			char buf[64];
			int num = recv(server, buf, 64, 0);
			if (num < 0)
			{
				if (run)
					printf("error: failed to receive\n");
				continue;
			}
			printf("> %.*s", num, buf);
		}
	}).detach();

	for(;;)
	{
		#define MAXBUF 64
		char buf[MAXBUF];
		fgets(buf, MAXBUF, stdin);

		if (!strcmp(buf, "exit\n"))
		{	
			printf("info: disconnect\n");
			break;
		}
		
		err = send(server, buf, strlen(buf), 0);

		if (err < 0)
		{
			printf("error: failed to send\n");
			break;
		}
	}

	// stop the printing thread
	run = false;
	// notify server that client socket is about to close
	// if the server is blocking in recv(), after this call
	// the server-side recv() will return 0
	shutdown(server, SD_SEND);
	// wait for server to send a shutdown admission
	char buf[16];
	err = recv(server, buf, 16, 0);

	closesocket(server);
	WSACleanup();
}
