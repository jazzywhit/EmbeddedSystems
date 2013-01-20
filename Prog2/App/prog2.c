/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 2   -   Jesse Whitworth
-----------------------------------------------------------------------
			       prog2.c
-----------------------------------------------------------------------
The objective of this program is to implement the base station receiver 
for a wireless sensor network. In addition to the base station, there are 
multiple sensor nodes, each of which relays packets containing climactic data 
to the base station via the RF network. You are to write a C program to accept 
messages from the sensor nodes and display them on the screen. Your program 
must decode the incoming packets to interpret the messages. 
-----------------------------------------------------------------------*/

#include "stdio.h"
#include "includes.h"
#include "Error.h"
#include "string.h"
#include "pktParser.h"

#define PayloadHeaderDiff 8  //Amount of header before the data starts.
#define BaudRate 9600           /* RS232 Port Baud Rate */

#pragma pack(1) // Don't align on word boundaries

typedef struct
{
	CPU_INT08U payloadLen; // Number of data bytes
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
void AppMain();
void PrintPacketHeader(CPU_CHAR* message, CPU_INT08U *i);
void ParseW(CPU_INT08U *speed, CPU_INT16U *dir);
void ParseP(CPU_INT08U *depth);
CPU_INT32U ReverseBytes32(CPU_INT32U *original);
void ParseD(CPU_INT32U *dt);
void DisplayPacket(Payload *payload);

/*-------------------- P r i n t P a c k e t H e a d e r ( ) -------------------------------------
	Purpose:	Prints out the required notification for a successfully read packet.
*/
void PrintPacketHeader(CPU_CHAR* message, CPU_INT08U *i){
	BSP_Ser_Printf("NODE %u %s \n", *i, message);
}

/*-------------------- P a r s e W ( ) -------------------------------------
	Purpose:	Parses the Packed Wind packet into it's component parts using bitwise arithmetic.
*/
void ParseW(CPU_INT08U *speed, CPU_INT16U *dir){
	const CPU_INT08U Mask = 0xF0;
	
	BSP_Ser_Printf("  Wind Direction = %u  Speed = %u.%u\n\n", 
		*dir, (((speed[0] & Mask)>>4)*100) + ((speed[0] & ~Mask)*10) + ((speed[1] & Mask)>>4), (speed[1] & ~Mask));
}

/*-------------------- P a r s e P ( ) -------------------------------------
	Purpose:	Parses the Packed Precipitation packet into it's component parts using bitwise arithmetic.
*/
void ParseP(CPU_INT08U *depth){
	const CPU_INT08U Mask = 0xF0;
	
	BSP_Ser_Printf("  Precipitation Depth = %u.%u%u\n\n", 
		(((depth[0] & Mask)>>4)*10) + (depth[0] & ~Mask), (depth[1] & Mask)>>4, (depth[1] & ~Mask));
}

/*-------------------- R e v e r s e B y t e s 3 2 ( ) -------------------------------------
	Purpose:	Reverses the bytes in a 32bit unsigned integer.
*/
CPU_INT32U ReverseBytes32(CPU_INT32U *original){
	return ((*original >> 24)&0x000000FF) | ((*original << 8) &0x00FF0000) | 
           ((*original >> 8) &0x0000FF00) | ((*original << 24)&0xFF000000);
}

/*-------------------- P a r s e D ( ) -------------------------------------
	Purpose:	Parses the Packed Date/Time packet into it's component parts using bitwise arithmetic.
	Issue:		The date/time packet is packed in big endian.
			The bytes must be reversed before bitwise manipulation.
*/
void ParseD(CPU_INT32U *dt){
	CPU_INT32U rBytes = ReverseBytes32(dt);
	
	//Masks for the different packed components
	const CPU_INT32U MYear = 0xFFF00000;
	const CPU_INT32U MMonth = 0x000F0000;
	const CPU_INT16U MDay = 0xF800;
	const CPU_INT16U MHour = 0x07C0;
	const CPU_INT16U MMinute = 0x003F;

	BSP_Ser_Printf("  Time Stamp = %u/%u/%u %u:%u\n\n", 
		(rBytes & MMonth)>>16, (rBytes & MDay)>>11, (rBytes & MYear) >> 20,(rBytes & MHour)>>6,(rBytes & MMinute));

}

/*-------------------- D i s p l a y P a c k e t ( ) -------------------------------------
	Purpose:	Display the relevant packet information in a formatted manner.*/
void DisplayPacket(Payload *payload){

	switch(payload->msgType){
		case 'B':
			PrintPacketHeader("BAROMETRIC PRESSURE PACKET", &payload->srcAddr);
			BSP_Ser_Printf("  Pressure = %u\n\n", payload->dataPart.pres);
			break;
		case 'D':
			PrintPacketHeader("DATE/TIME STAMP PACKET", &payload->srcAddr);
			ParseD(&payload->dataPart.dateTime);
			break;
		case 'H':
			PrintPacketHeader("HUMIDITY PACKET", &payload->srcAddr);
			BSP_Ser_Printf("  Humidity = %u  Dew Point = %u\n\n", payload->dataPart.hum.hum, payload->dataPart.hum.dewPt);
			break;
		case 'I':
			PrintPacketHeader("NODE ID PACKET", &payload->srcAddr);
			payload->dataPart.id[(payload->payloadLen-PayloadHeaderDiff)] = '\0'; //Terminate the string
			BSP_Ser_Printf("  Node ID = %s\n\n", payload->dataPart.id);
			break;
		case 'P':
			PrintPacketHeader("PRECIPITATION PACKET", &payload->srcAddr);
			ParseP(payload->dataPart.depth);
			break;
		case 'R':
			PrintPacketHeader("SOLAR RADIATION INTENSITY PACKET", &payload->srcAddr);
			BSP_Ser_Printf("  Solar Radiation Intensity = %u\n\n", payload->dataPart.rad);
			break;
		case 'T':
			PrintPacketHeader("TEMPERATURE PACKET", &payload->srcAddr);
			BSP_Ser_Printf("  Temperature = %i\n\n", payload->dataPart.temp);
			break;
		case 'W':
			PrintPacketHeader("WIND PACKET", &payload->srcAddr);
			ParseW(payload->dataPart.wind.speed, &payload->dataPart.wind.dir);
			break;
		default:
			ShowError("Unknown Packet Type");
			break;
	}
}

/*--------------- m a i n ( ) -----------------*/

int main()
{
	
    //  Initialize the STM32F107 eval. board.
    BSP_IntDisAll();            // Disable all interrupts.
    BSP_Init();                 // Initialize BSP functions
    BSP_Ser_Init(BaudRate);     // Initialize the RS232 interface.

    AppMain();  //  Run the application.
    
    return 0;
}

/*-------------------- A p p M a i n ( ) ----------------------------*/
void AppMain(){
	Payload payload;

	for(;;){
	    ParsePkt(&payload);
	    if (payload.dstAddr != 1){
		ShowError("Not My Address");
		continue;
            }
	    DisplayPacket(&payload);
        }
}

