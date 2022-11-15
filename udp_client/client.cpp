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
#define PORT 3425	//The port on which to listen for incoming data

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
	read_timeout.tv_sec = 3;
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
	uint32_t fail_count = 0;

	uint64_t start_tick_64 = GetTickCount64();
	uint64_t report_ts = 0;

	LARGE_INTEGER qpf_freq;
	QueryPerformanceFrequency(&qpf_freq);
	while(1)
	{
		std::string console_input;
		std::cin >> console_input;
		//printf("%s\r\n", console_input.data());

		LARGE_INTEGER start_count;
		QueryPerformanceCounter(&start_count);
		//uint64_t start_tick = GetTickCount64();
		sprintf(t_buf, "du motherfucking hast meichgsl\r\n");
		if (sendto(s, t_buf, strlen(t_buf), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d. Failcount: %d\r\n", WSAGetLastError(), fail_count);
			fail_count++;
			//exit(EXIT_FAILURE);
		}
		else
		{
			printf("%s\r\n", buf);
		}
		LARGE_INTEGER end_count;
		QueryPerformanceCounter(&end_count);
		LARGE_INTEGER elapsed_us;
		elapsed_us.QuadPart = end_count.QuadPart - start_count.QuadPart;
		elapsed_us.QuadPart *= 1000000;
		elapsed_us.QuadPart /= qpf_freq.QuadPart;
		printf("round trip time: ticks: %d, microseconds: %d\r\n", (int)(end_count.QuadPart - start_count.QuadPart), (int)(elapsed_us.QuadPart) );
	}

	closesocket(s);
	WSACleanup();

	return 0;
}