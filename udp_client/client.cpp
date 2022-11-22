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


/*
Generic hex checksum calculation.
TODO: use this in the psyonic API
*/
uint32_t get_checksum32(uint32_t* arr, int size)
{
	int32_t checksum = 0;
	for (int i = 0; i < size; i++)
		checksum += (int32_t)arr[i];
	return -checksum;
}



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
	DWORD read_timeout_ms = 500;
	/*set timeout*/
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)(&read_timeout_ms), sizeof(read_timeout_ms));


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
	si_other.sin_addr.S_un.S_addr = inet_addr("192.168.56.255");

	inet_ntop(AF_INET, &si_other.sin_addr.S_un.S_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
	printf("Targeting address: %s on port %d\r\n", inet_addr_buf, PORT);


	//start communication
	u32_fmt_t farr[40] = { 0 };
	SYSTEMTIME st;
	uint32_t fail_count = 0;

	uint64_t start_tick_64 = GetTickCount64();
	uint64_t report_ts = 0;
	while (1)
	{
		uint64_t tick = GetTickCount64() - start_tick_64;
		float t = (float)tick * .001f;

		/*create a payload*/
		for (int i = 0; i < 6; i++)
		{
			farr[i].f32 = sin_fast(wrap_2pi(t) + (float)i / TWO_PI);	//for later
		}
		farr[6].u32 = get_checksum32( (uint32_t*)(&(farr[0].u32)), 6);

		//sprintf(t_buf, "client uptime: %f\r\n", t);


		if(tick > report_ts)
		{
			report_ts = tick + 20;	//send udp packet once every 50 milliseconds (or so)
			//send the t_buf
			const char* p_payload = (const char*)(&farr[0]);
			if (sendto(s, p_payload, 7*sizeof(u32_fmt_t), 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
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
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}