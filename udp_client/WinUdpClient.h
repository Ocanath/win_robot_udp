#ifndef WINUDPCLIENT_H
#define	WINUDPCLIENT_H	

#include<stdio.h>
#include<winsock2.h>
#include <WS2tcpip.h>
#include <stdint.h>


class WinUdpClient
{
public:
	struct sockaddr_in si_other;
	struct sockaddr_in si_us;
	int s;
	int slen;
	WSADATA wsa;
	WinUdpClient(uint16_t port)
	{
		slen = sizeof(si_other);
		//Initialise winsock
		printf("\nInitialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			printf("Failed. Error Code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Initialised.\n");
		//create socket
		if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			printf("socket() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		/*Obtain (a) host ip and display it to the console*/
		char namebuf[256] = { 0 };
		const char inet_addr_buf[256] = { 0 };
		int rc = gethostname(namebuf, 256);
		printf("hostname: %s\r\n", namebuf);
		hostent* phost = gethostbyname(namebuf);
		for (int i = 0; phost->h_addr_list[i] != NULL; i++)
		{
			PCSTR retv = inet_ntop(AF_INET, phost->h_addr_list[i], (PSTR)inet_addr_buf, 256);
			printf("Host has IP address %d: %s\r\n", i, inet_addr_buf);
		}

		memset((char*)&si_us, 0, sizeof(si_us));
		si_us.sin_family = AF_INET;

		//setup address structure
		memset((char*)&si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(port);
		//si_other.sin_addr.S_un.S_addr = inet_addr(inet_addr_buf); //assign the client the last IP address in the IPV4 list provided by the host name address list lookup. 
		//si_other.sin_addr.S_un.S_addr = inet_addr("192.168.56.255");
		si_other.sin_addr.S_un.S_addr = inet_addr("127.0.0.0");
		//si_other.sin_addr.S_un.S_addr = inet_addr("98.148.246.163");
		//si_other.sin_addr = in4addr_any;

		inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
		//printf("Target address: %s on port %d\r\n", inet_addr_buf, port);

	}
	int set_nonblocking(void)
	{
		//set to blocking and clear
		u_long iMode = 1;
		return ioctlsocket(s, FIONBIO, &iMode);
	}

	~WinUdpClient()
	{

	}
};



#endif
