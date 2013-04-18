#include "lpc17xx.h"
#include "config.h"
#include "util.h"
#include "w5200.h"
#include <stdio.h>
#include <stdarg.h>

extern CONFIG_MSG Config_Msg;
extern CHCONFIG_TYPE_DEF Chconfig_Type_Def; 

extern uint8 txsize[MAX_SOCK_NUM];
extern uint8 rxsize[MAX_SOCK_NUM];


#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/*
extern uint8 MAC[6];
extern uint8 IP[4];
extern uint8 GateWay[4];
extern uint8 SubNet[4];
extern uint8 Enable_DHCP;
extern uint8 Dest_IP[4] ;
extern uint16 Dest_PORT ;
*/

//static uint8 Dest_IP[4] = {192, 168, 1, 103}; //DST_IP Address


CONFIG_MSG Config_Msg;
CHCONFIG_TYPE_DEF Chconfig_Type_Def;

// Configuration Network Information of W5200

uint8 Enable_DHCP = OFF;
//uint8 MAC[6] = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};//MAC Address
//uint8 IP[4] = {192, 168, 1, 23};//IP Address
//uint8 GateWay[4] = {192, 168, 1, 1};//Gateway Address
//uint8 SubNet[4] = {255, 255, 255, 0};//SubnetMask Address

//TX MEM SIZE- SOCKET 0:8KB, SOCKET 1:2KB, SOCKET2-7:1KB
//RX MEM SIZE- SOCKET 0:8KB, SOCKET 1:2KB, SOCKET2-7:1KB
uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};

//FOR TCP Client
//Configuration Network Information of TEST PC
//uint8 Dest_IP[4] = {192, 168, 1, 103}; //DST_IP Address
uint16 Dest_PORT = 3000; //DST_IP port

uint8 ch_status[MAX_SOCK_NUM] = { 0, };	/** 0:close, 1:ready, 2:connected */

uint8 TX_BUF[TX_RX_MAX_BUF_SIZE]; // TX Buffer for applications
uint8 RX_BUF[TX_RX_MAX_BUF_SIZE]; // RX Buffer for applications




void Set_network(uint8_t * ip,uint8_t * gateway,uint8_t * mac, uint8_t *sub)
{
		char buffer[100];
        uint8 tmp_array[6];       
        uint8 i;
        
        // MAC ADDRESS
        for (i = 0 ; i < 6; i++) Config_Msg.Mac[i] = mac[i];
        // Local IP ADDRESS
        Config_Msg.Lip[0] = ip[0]; Config_Msg.Lip[1] = ip[1]; Config_Msg.Lip[2] = ip[2]; Config_Msg.Lip[3] = ip[3];
        // GateWay ADDRESS
        Config_Msg.Gw[0] = gateway[0]; Config_Msg.Gw[1] = gateway[1]; Config_Msg.Gw[2] = gateway[2]; Config_Msg.Gw[3] = gateway[3];
        // Subnet Mask ADDRESS
        Config_Msg.Sub[0] = sub[0]; Config_Msg.Sub[1] = sub[1]; Config_Msg.Sub[2] = sub[2]; Config_Msg.Sub[3] = sub[3];
        
        setSHAR(Config_Msg.Mac);
//        setSUBR(Config_Msg.Sub);
        saveSUBR(Config_Msg.Sub);
        setGAR(Config_Msg.Gw);
        setSIPR(Config_Msg.Lip);

        // Set DHCP
        Config_Msg.DHCP = Enable_DHCP;    
        //Destination IP address for TCP Client
        //Chconfig_Type_Def.destip[0] = Dest_IP[0]; Chconfig_Type_Def.destip[1] = Dest_IP[1];
        //Chconfig_Type_Def.destip[2] = Dest_IP[2]; Chconfig_Type_Def.destip[3] = Dest_IP[3];
        //Chconfig_Type_Def.port = Dest_PORT;

        //Set PTR and RCR register	
        setRTR(6000);
        setRCR(3);

        //Init. TX & RX Memory size
        sysinit(txsize, rxsize);
        
        sprintf(buffer,"\r\nMAC : %.2X.%.2X.%.2X.%.2X.%.2X.%.2X", IINCHIP_READ(SHAR0+0),IINCHIP_READ(SHAR0+1),IINCHIP_READ(SHAR0+2),IINCHIP_READ(SHAR0+3),IINCHIP_READ(SHAR0+4),IINCHIP_READ(SHAR0+5));
        
        getSIPR (tmp_array);
        sprintf(buffer,"\r\nIP : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
        
        getSUBR(tmp_array);
        sprintf(buffer,"\r\nSN : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
        
        getGAR(tmp_array);
        sprintf(buffer,"\r\nGW : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
}

void Reset_W5200(void)
{
	//GPIO_ResetBits(GPIOB, WIZ_RESET);
	LPC_GPIO0 -> FIOCLR = (1<<21);

	Delay_ms(1);
	//GPIO_SetBits(GPIOB, WIZ_RESET);
	LPC_GPIO0 -> FIOSET = (1<<21);

	Delay_ms(500);
}


void LED3_onoff(uint8_t on_off)
{
	if (on_off == ON) {
		//GPIO_ResetBits(GPIOA, LED3); // LED on
		LPC_GPIO1 -> FIOCLR = (1<<19);
	}else {
		//GPIO_SetBits(GPIOA, LED3); // LED off
		LPC_GPIO1 -> FIOSET = (1<<19);
	}
}

void LED4_onoff(uint8_t on_off)
{
	if (on_off == ON) {
		//GPIO_ResetBits(GPIOA, LED4); // LED on
		LPC_GPIO1 -> FIOCLR = (1<<20);
	}else {
		//GPIO_SetBits(GPIOA, LED4); // LED off
		LPC_GPIO1 -> FIOSET = (1<<20);
	}
}

void Delay_ms( uint32_t time_ms )
{
 	volatile uint32_t j= time_ms * 10000;
	volatile uint32_t i;
	for(i = 0; i < j; i++)
	{
	}
}



