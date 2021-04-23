#include <stdint.h>
#include <string.h>
#include "pca10028.h"
#include "nrf51_esp8266_drv.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "nrf_gpio.h"

char  str_at[]="AT\r\n";                                    		  //  ����ָ�����"OK"
//char  str_at_cwmode[]="AT+CWMODE=3\r\n";                         	//  ����ESP8266�Ĺ���ģʽ������"OK"����"no change"
char  str_at_cwmode[]="AT+CWMODE=1\r\n";                         	//  ����ESP8266�Ĺ���ģʽ������"OK"����"no change"
char  str_at_cwjap[]="AT+CWJAP=\"TP-LINK_5098\",\"zhengtu.1992\"\r\n";     		//  ���ӵ�WiFi�ȵ㣬FiYuΪ�ȵ����ƣ�88518851Ϊ���룻���ӳɹ����ء�OK��     
char  str_at_cifsr[]="AT+CIFSR\r\n";                              // 	����IP��ַ��ѯָ��
//char  str_at_cipstart[]="AT+CIPSTART=\"TCP\",\"192.168.191.1\",5000\r\n";    //  ���ӵ�TCP�����������ء�OK��
char  str_at_cipstart[]="AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n";    //  ���ӵ�TCP�����������ء�OK��
char  str_at_cipsend[]="AT+CIPSEND=6\r\n";   															   //  ��������ָ��
char  str7[]="hello!\r\n";  
char  str_at_cipserver[]="AT+CIPSERVER=1,5000\r\n";   						//  ����TCP�����������Ŷ˿�5000							
//char  str_at_cipmux[]="AT+CIPMUX=1\r\n";   												//	�򿪶�·����	
char  str_at_cipmux[]="AT+CIPMUX=0\r\n";   												//	��رն�·����	
char  str_rst[]="AT+RST\r\n"; 																	  //  �����λ
char  str_at_cipsend_mux[]="AT+CIPSEND=0,15\r\n";   							//  ��������ָ��,���ڶ�·����ģʽ
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
//�������ã�������115200bps�����عر�
void uart_config(void)
{
    uint32_t err_code;
	
    const app_uart_comm_params_t comm_params =
    {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,    //�ر�����
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200  //����������Ϊ115200bps
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
 * ��  �� : �ȴ�ESP8266ģ����Ӧ
 * ��  �� : [in]timeout_ms - �ȴ���ʱʱ��(ms)
 * ����ֵ : ��
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
 * ��  �� : ��λESP8266ģ��
 * ��  �� : [in]RstType - ��λ���ͣ��ⲿ��λ�������λ
 * ����ֵ : ��
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
 * ��  �� : main����
 * ��  �� : [in]pcmd  - ָ��ָ���ַ���
 *          [in]resp1 - ָ����Ӧ�ַ���1
 *          [in]resp2 - ָ����Ӧ�ַ���2������resp1�ǻ�Ĺ�ϵ��ֻҪģ����Ӧ�����ݺ�����һ�����ϾͿ���
 *          [in]timeout - ��Ӧ��ʱʱ��
 * ����ֵ : true - ����ִ�гɹ�
 *          false - ����ִ��ʧ��
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
//ESP8266����
void ESP8266_SetUp(void)
{
	
		//��������ָ�ģ����Ӧ��OK
    while(!ESP8266_ExeCMD(str_at,"OK",NULL,1000)){}        
			
		//��λ����ESP8266ģ�飬ģ����Ӧ��OK
		ESP8266_Reset(ESP8266_RST_EXT);
		nrf_delay_ms(1000);
		//����ESP8266����ģʽָ������õ�ģʽ��3�֣�1-stationģʽ 2-softAPģʽ 3-station+softAPģʽ��ģ����Ӧ��OK
    while(!ESP8266_ExeCMD(str_at_cwmode,"OK","no change",1000)){} 
		
		//while(!ESP8266_ExeCMD(	"AT+CWAUTOCONN=0","OK",NULL,500)){} 
		nrf_delay_ms(1000);		
	  while(!ESP8266_ExeCMD(str_at_cwjap,"OK",NULL,1000)){} 		
    nrf_delay_ms(1000);
	  //����Ϊ��·���ӡ������õ�ģʽ��2�֣�0-������ģʽ 1-��·����ģʽ��ģ����Ӧ��OK   
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
	  //����TCP �������������Ŷ˿�5000��ģ����Ӧ��OK
	  //Server�������Զ�����Server����������client����ʱ���Զ�����˳��ռ��һ������
		//while(!ESP8266_ExeCMD(str_at_cipserver,"OK",NULL,500)){} 
			
		esp8266.len = 0;

		//while(!ESP8266_ExeCMD(str_at_cifsr,"OK",NULL,500)){}
}
