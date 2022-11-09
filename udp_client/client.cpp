/*
	Simple udp client
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include <iostream>

#define BUFLEN 512	//Max length of buffer
#define PORT 10103	//The port on which to listen for incoming data

int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

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

	//setup address structure
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	//si_other.sin_addr.S_un.S_addr = inet_addr(inet_addr_buf); //assign the client the last IP address in the IPV4 list provided by the host name address list lookup. 
	si_other.sin_addr.S_un.S_addr = inet_addr("192.168.29.255");

	inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
	printf("Targeting address: %s on port %d\r\n", inet_addr_buf, PORT);


	//start communication
	while (1)
	{
		printf("Enter message : ");
		//gets(message);
		std::cin >> message;

		inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
		printf("Sending payload to target address: %s on port %d\r\n", inet_addr_buf, PORT);

		//send the message
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
		printf("Preparing to read from: %s on port %d\r\n", inet_addr_buf, PORT);

		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		puts(buf);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}