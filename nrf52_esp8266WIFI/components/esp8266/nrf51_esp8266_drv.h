#ifndef NRF51_ESP8266_DRV_H
#define NRF51_ESP8266_DRV_H
#include <stdint.h>

#define  ESP8266_RST_EXT   1    //通过复位引脚复位ESP8266
#define  ESP8266_RST_INT   2    //发送指令复位ESP8266

#define ESP8266_RST    28

#define  Buf_Max 80


typedef struct
{
	uint32_t len;  
	char buf[Buf_Max];

}esp8266_t;

extern esp8266_t esp8266;

void ESP8266_SetUp(void);
void uart_config(void);



#endif



