#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <wepoll.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("usage: lsrv <port>\n");
		return -1;
	}

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	int err;
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	err = bind(listener, (struct sockaddr*) &addr, sizeof(addr));	
	
	if (err < 0)
	{
		printf("invalid port or port occupied\n");
		return -1;
	}

	listen(listener, SOMAXCONN);

	std::vector<SOCKET> socks;
	HANDLE kq = epoll_create1(0);
	
	struct epoll_event ev;
	ev.events = EPOLLIN; // EPOLLIN for a listner socket indicates an
						 // arrival of a new connection
	ev.data.sock = listener;
	epoll_ctl(kq, EPOLL_CTL_ADD, listener, &ev);	

	while(true)
	{
		#define MAXBUF 64
		#define MAXEV 32
		char buf[MAXBUF];
		struct epoll_event evs[MAXEV];

		int num_ev = epoll_wait(kq, evs, MAXEV, -1);
		for (int i = 0; i < num_ev; i++)
		{
			// current socket
			int sock = evs[i].data.sock;

			int is_listener;
			int optlen = sizeof(is_listener);
			getsockopt(sock, SOL_SOCKET, SO_ACCEPTCONN, 
				(char*) &is_listener, &optlen);

			if (is_listener)
			{
				SOCKET client = accept(sock, NULL, NULL);	
				
				char verify[6];
				int num = recv(client, verify, 6, MSG_WAITALL);
				if (num <= 0)
				{
					printf("info: a connection failed\n");
					closesocket(client);
					continue;
				}

				if (strncmp(verify, "VERIFY", 6))
				{
					printf("info: a verification failed\n");
					closesocket(client);
					continue;
				}

				printf("info: a client connected\n");
				socks.push_back(client);
				// register the client socket
				struct epoll_event ev;
				ev.events = EPOLLIN | EPOLLRDHUP;
				ev.data.sock = client;
				epoll_ctl(kq, EPOLL_CTL_ADD, client, &ev);
			}
			else	// not a listener socket
			{
				int size = recv(sock, buf, MAXBUF, 0);
				if (size <= 0)
				{
					printf("info: a client disconnected\n");
					// remove from the queue
					epoll_ctl(kq, EPOLL_CTL_DEL, sock, NULL);
					// remove from the vector, rather complicated 
					// berfore C++ 20.
					socks.erase(
						std::remove(socks.begin(), socks.end(), sock),
						socks.end()
					);
					// notify the client that its shutdown request
					// is admitted.
					shutdown(sock, SD_SEND);
					closesocket(sock);
				}
				else
				{
					// send it to all clients, including the sender.
					for(auto s : socks)
					{
						send(s, buf, size, 0);
					}
				}	
			}
		}
	}

	closesocket(listener);
	for(auto s : socks)
	{
		closesocket(s);
	}
	WSACleanup();
}
