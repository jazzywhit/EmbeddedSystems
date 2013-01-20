/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 3   -   Jesse Whitworth
-----------------------------------------------------------------------
			        Payload.h
-----------------------------------------------------------------------
Create the static buffer queues that will be used to handle the payloads
*/
#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "includes.h"
#include "BfrQ.h"

typedef struct
{
	CPU_INT08S payloadLen; // Number of data bytes
	CPU_INT08U dstAddr; // Destination address
	CPU_INT08U srcAddr; // Address of sending node
	CPU_CHAR msgType; // ASCII Message Type
	union
	{
		CPU_INT16U pres; // B Packet
		CPU_INT32U dateTime; // D Packet
		struct // H Packet
		{
			CPU_INT16S	dewPt;
			CPU_INT08U	hum;
		} hum;
		CPU_INT08U id[10];// I Packet //See Note Below
		CPU_INT08U depth[2]; // P Packet //See Note Below
		CPU_INT16U rad; // R Packet
		CPU_INT16S temp; // T Packet
		struct // W Packet
		{
			CPU_INT08U	speed[2]; //See Note Below
			CPU_INT16U	dir;
		} wind;
	} dataPart;
} Payload;

/*----- f u n c t i o n    p r o t o t y p e s -----*/
CPU_VOID ParseWind(CPU_CHAR *messageStr, CPU_INT08U *speed, CPU_INT16U *dir, CPU_INT08U srcAddr);
CPU_VOID ParsePrecip(CPU_CHAR *messageStr, CPU_INT08U *depth, CPU_INT08U srcAddr);
CPU_VOID ParseDate(CPU_CHAR *messageStr, CPU_INT32U *ts, CPU_INT08U srcAddr);
CPU_INT32U ReverseBytes32(CPU_INT32U *original);
CPU_BOOLEAN ConstructMessage(Payload *payload, CPU_CHAR *messageStr);
CPU_VOID ConstructError(Payload *payload, CPU_CHAR *message);
CPU_VOID ConstructPayload(CPU_VOID *payload);

//Requred Functions
CPU_VOID PayloadInit(BfrQ **payloadBfrQ, BfrQ **replyBfrQ);
CPU_VOID PayloadTask(CPU_VOID);

#endif