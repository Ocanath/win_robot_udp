/*
	Simple udp client
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include <WS2tcpip.h>
#include "sin_math.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include <iostream>

#define BUFLEN 512	//Max length of buffer
#define PORT 10103	//The port on which to listen for incoming data

typedef union u32_fmt_t
{
	uint32_t u32;
	int32_t i32;
	float f32;
	int16_t i16[sizeof(uint32_t) / sizeof(int16_t)];
	uint16_t ui16[sizeof(uint32_t) / sizeof(uint16_t)];
	int8_t i8[sizeof(uint32_t) / sizeof(int8_t)];
	uint8_t ui8[sizeof(uint32_t) / sizeof(uint8_t)];
}u32_fmt_t;

int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char t_buf[BUFLEN];
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
	/*set timeout*/
	struct timeval read_timeout;
	read_timeout.tv_sec = 1;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)(&read_timeout), sizeof(read_timeout));


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
	u32_fmt_t farr[40] = { 0 };
	SYSTEMTIME st;

	while (1)
	{
		GetSystemTime(&st);
		float t = wrap_2pi( ((float)st.wMilliseconds) / 1000.f + ((float)st.wSecond) );
		for (int i = 0; i < 6; i++)
		{
			farr[i].f32 = sin_fast(t + (float)i / TWO_PI);
		}
		sprintf(t_buf, "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n",
			farr[0].f32,
			farr[1].f32,
			farr[2].f32,
			farr[3].f32,
			farr[4].f32,
			farr[5].f32
			);
		//printf("Enter t_buf : ");
		//gets(t_buf);
		//std::cin >> t_buf;

		//inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
		//printf("Sending payload to target address: %s on port %d\r\n", inet_addr_buf, PORT);

		//send the t_buf
		if (sendto(s, t_buf, strlen(t_buf), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
		//printf("Preparing to read from: %s on port %d\r\n", inet_addr_buf, PORT);

		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			//exit(EXIT_FAILURE);
		}

		inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
		printf("reply received from address: %s: %s\r\n", inet_addr_buf, buf);

		//puts(buf);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}