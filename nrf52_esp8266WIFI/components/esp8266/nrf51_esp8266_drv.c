#include <stdint.h>
#include <string.h>
#include "pca10028.h"
#include "nrf51_esp8266_drv.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "nrf_gpio.h"

char  str_at[]="AT\r\n";                                    		  //  联机指令，返回"OK"
//char  str_at_cwmode[]="AT+CWMODE=3\r\n";                         	//  设置ESP8266的工作模式，返回"OK"或者"no change"
char  str_at_cwmode[]="AT+CWMODE=1\r\n";                         	//  设置ESP8266的工作模式，返回"OK"或者"no change"
char  str_at_cwjap[]="AT+CWJAP=\"TP-LINK_5098\",\"zhengtu.1992\"\r\n";     		//  连接到WiFi热点，FiYu为热点名称，88518851为密码；连接成功返回“OK”     
char  str_at_cifsr[]="AT+CIFSR\r\n";                              // 	本机IP地址查询指令
//char  str_at_cipstart[]="AT+CIPSTART=\"TCP\",\"192.168.191.1\",5000\r\n";    //  连接到TCP服务器，返回“OK”
char  str_at_cipstart[]="AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n";    //  连接到TCP服务器，返回“OK”
char  str_at_cipsend[]="AT+CIPSEND=6\r\n";   															   //  发送数据指令
char  str7[]="hello!\r\n";  
char  str_at_cipserver[]="AT+CIPSERVER=1,5000\r\n";   						//  建立TCP服务器，开放端口5000							
//char  str_at_cipmux[]="AT+CIPMUX=1\r\n";   												//	打开多路连接	
char  str_at_cipmux[]="AT+CIPMUX=0\r\n";   												//	垂乇斩嗦妨?	
char  str_rst[]="AT+RST\r\n"; 																	  //  软件复位
char  str_at_cipsend_mux[]="AT+CIPSEND=0,15\r\n";   							//  发送数据指令,基于多路连接模式
char  str12[]="Command Executed!\r\n";  

esp8266_t esp8266;


#define UART_TX_BUF_SIZE 256                          /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                            /**< UART RX buffer size. */


void uart_error_handle(app_uart_evt_t * p_event)
{
    uint8_t cr;
	
	  if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
		else if(p_event->evt_type == APP_UART_DATA_READY)
		{
			  while(app_uart_get(&cr) == NRF_SUCCESS)
	      {
		        esp8266.buf[esp8266.len++] = cr;
					  if(esp8266.len>=Buf_Max)esp8266.len = 0;
	      }
		}
}
//串口配置：波特率115200bps，流控关闭
void uart_config(void)
{
    uint32_t err_code;
	
    const app_uart_comm_params_t comm_params =
    {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,    //关闭流控
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200  //波特率设置为115200bps
    };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);
}
/*****************************************************************************************
 * 描  述 : 等待ESP8266模块响应
 * 参  数 : [in]timeout_ms - 等待超时时间(ms)
 * 返回值 : 无
 *****************************************************************************************/
void Esp8266_WaitForReply(uint32_t timeout_ms)
{
	uint8_t cr;
	
	do
	{
		nrf_delay_ms(1);
		timeout_ms--;
		while(app_uart_get(&cr) == NRF_SUCCESS)
		{
			esp8266.buf[esp8266.len++] = cr;
		}
	}
	while(timeout_ms > 0);
}
/*****************************************************************************************
 * 描  述 : 复位ESP8266模块
 * 参  数 : [in]RstType - 复位类型，外部复位或软件复位
 * 返回值 : 无
 *****************************************************************************************/
void ESP8266_Reset(uint8_t RstType)
{
	  esp8266.len = 0;
	  if(RstType == ESP8266_RST_EXT) 
	  {
		    nrf_gpio_pin_clear(ESP8266_RST);
		    nrf_delay_ms(300);
		    nrf_gpio_pin_set(ESP8266_RST);
		    nrf_delay_ms(2000);
		
		    if(strstr(esp8266.buf,"OK")!=NULL)
	      {
		        memset(esp8266.buf,0,Buf_Max);
	      }
	      else 
	      {
		        memset(esp8266.buf,0,Buf_Max);
	      }
	  }
	  else{}
}
/*****************************************************************************************
 * 描  述 : main函数
 * 参  数 : [in]pcmd  - 指向指令字符串
 *          [in]resp1 - 指向响应字符串1
 *          [in]resp2 - 指向响应字符串2，他和resp1是或的关系，只要模块响应的内容和其中一个符合就可以
 *          [in]timeout - 响应超时时间
 * 返回值 : true - 命令执行成功
 *          false - 命令执行失败
 *****************************************************************************************/
bool ESP8266_ExeCMD(char *pcmd,char *resp1,char *resp2,uint32_t timeout)
{
	  esp8266.len = 0;
	  printf("%s",pcmd);
	
	  if((resp1 == NULL) && (resp2 == NULL)) return true;
	
	  //nrf_delay_ms(timeout);
	  nrf_delay_ms(timeout);
	  if( (strstr(esp8266.buf,resp1)!=NULL) || (strstr(esp8266.buf,resp2)!=NULL))
	  {
		    memset(esp8266.buf,0,30);
		    return true;
	  }
	  else 
	  {
		    memset(esp8266.buf,0,30);
		    return false;
	  }	
}
//ESP8266配置
void ESP8266_SetUp(void)
{
	
		//发送联机指令，模块响应：OK
    while(!ESP8266_ExeCMD(str_at,"OK",NULL,1000)){}        
			
		//复位重启ESP8266模块，模块响应：OK
		ESP8266_Reset(ESP8266_RST_EXT);
		nrf_delay_ms(1000);
		//设置ESP8266工作模式指令。可设置的模式有3种：1-station模式 2-softAP模式 3-station+softAP模式。模块响应：OK
    while(!ESP8266_ExeCMD(str_at_cwmode,"OK","no change",1000)){} 
		
		//while(!ESP8266_ExeCMD(	"AT+CWAUTOCONN=0","OK",NULL,500)){} 
		nrf_delay_ms(1000);		
	  while(!ESP8266_ExeCMD(str_at_cwjap,"OK",NULL,1000)){} 		
    nrf_delay_ms(1000);
	  //设置为多路连接。可设置的模式有2种：0-单连接模式 1-多路连接模式。模块响应：OK   
		while(!ESP8266_ExeCMD(str_at_cipmux,"OK",NULL,1000)){} 
		nrf_delay_ms(1000);	
		while(!ESP8266_ExeCMD("AT+CIPMODE=1\r\n","OK",NULL,1000)){} 
		nrf_delay_ms(1000);
		while(!ESP8266_ExeCMD(str_at_cipstart,"OK",NULL,1000)){} 
		nrf_delay_ms(1000);	

			
		while(!ESP8266_ExeCMD("AT+CIPSEND\r\n","OK",NULL,1000)){} 
		nrf_delay_ms(1000);
		while(!ESP8266_ExeCMD("POST /devices/704931691/datapoints?type=3 HTTP/1.1\r\napi-key:QW=lOU0zHy9M1jo8NM=XCnjgREY=\r\nHost:api.heclouds.com\r\nContent-Length:13\r\n\r\n{\"temp\":1234}\r\n",NULL,NULL,1000)){} 
		nrf_delay_ms(1000);
//		while(!ESP8266_ExeCMD("+++","OK",NULL,1000)){} 
//		nrf_delay_ms(1000);
//		while(!ESP8266_ExeCMD("POST /devices/704931691/datapoints?type=3 HTTP/1.1\r\n",NULL,NULL,1000)){} 
//			   nrf_delay_ms(1000);
//		while(!ESP8266_ExeCMD("api-key:QW=lOU0zHy9M1jo8NM=XCnjgREY=\r\n",NULL,NULL,1000)){} 
//			nrf_delay_ms(1000);
//		while(!ESP8266_ExeCMD("Host:api.heclouds.com\r\n",NULL,NULL,1000)){} 
//			nrf_delay_ms(1000);
//		while(!ESP8266_ExeCMD("Content-Length:13\r\n\r\n",NULL,NULL,1000)){} 
//			nrf_delay_ms(1000);
//			while(!ESP8266_ExeCMD("{\"temp\":1234}\r\n",NULL,NULL,1000)){} 
//				nrf_delay_ms(1000);


	
			
		
		//while(!ESP8266_ExeCMD(str_at_cwmode,"WIFI GOT IP\r\n\r\nOK",NULL,1000)){} 
	  //建立TCP 服务器，并开放端口5000。模块响应：OK
	  //Server开启后自动建立Server监听，当有client接入时会自动按照顺序占用一个连接
		//while(!ESP8266_ExeCMD(str_at_cipserver,"OK",NULL,500)){} 
			
		esp8266.len = 0;

		//while(!ESP8266_ExeCMD(str_at_cifsr,"OK",NULL,500)){}
}
