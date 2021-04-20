
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_drv_uart.h"
#include "nrf51_esp8266_drv.h"
#include "FreeRTOS.h"
#include "task.h"
#include "nrf_drv_clock.h"
#include "semphr.h" 	 //�ź���
#include "queue.h"		 //����
#include "event_groups.h"//�¼���־��

/*
P0.31��UART_TXD   �����ڷ���
P0.30��UART_RXD   �����ڽ���
P0.28��ESP8266��λ

*/
TaskHandle_t start_toggle_task_handle;
SemaphoreHandle_t BinarySemaphore;
EventGroupHandle_t Event_Handle = NULL;     //�¼���־�飨λ0��WIFI����״̬ λ1��PING������2S���ٷ���ģʽ��
const int WIFI_CONECT = (0x01 << 0);        //�����¼������λ 0������������ģʽ��ֵ1��ʾ�Ѿ����ӣ�0��ʾδ����
const int PING_MODE   = (0x01 << 1);        //�����¼������λ 1��PING����������ģʽ��1��ʾ����30S����ģʽ��0��ʾδ�������ͻ���2S���ٷ���ģʽ

static void start_toggle_task_function(void *pvParameter)
{      
	
//		taskENTER_CRITICAL();
//	
//	//������ֵ�ź���
	BinarySemaphore = xSemaphoreCreateBinary();	
	//�¼���־�飬���ڱ�־wifi����״̬�Լ�ping����״̬
	Event_Handle = xEventGroupCreate(); 
	
	  ESP8266_SetUp();
		xEventGroupSetBits(Event_Handle, WIFI_CONECT);  //�����������ӣ��׳��¼���־ 
		vTaskSuspend(NULL);	    						//�����������ӣ������Լ����������̬�������ɹ���תΪ����̬ʱ�������ִ����ȥ��
		xEventGroupClearBits(Event_Handle, WIFI_CONECT);//����������wifi�ѶϿ�������¼���־������ִ�б������������� 
	
//	while (true)
//    {
//				//���ڽ��յ�����?
//			 // if(esp8266.len > 1) 
//			//	{
//            if(strstr(esp8266.buf,"D1")!=NULL)	    //�յ���תLED1ָʾ��״̬�ĵ�ָ��
//						{
//							  nrf_gpio_pin_toggle(LED_1);		
//                esp8266.len = 0;								
//							  memset(esp8266.buf,0,30);
//						}
//						else if(strstr(esp8266.buf,"D2")!=NULL) //�յ���תLED2ָʾ��״̬�ĵ�ָ��
//						{
//							  memset(esp8266.buf,0,30);
//							  esp8266.len = 0;	
//							  nrf_gpio_pin_toggle(LED_2);
//						}		
//            else if(strstr(esp8266.buf,"D3")!=NULL) //�յ���תLED3ָʾ��״̬�ĵ�ָ��
//						{
//							  memset(esp8266.buf,0,30);
//							  esp8266.len = 0;	
//							  nrf_gpio_pin_toggle(LED_3);
//						}	
//            else if(strstr(esp8266.buf,"D4")!=NULL) //�յ���תLED4ָʾ��״̬�ĵ�ָ��
//						{
//							  memset(esp8266.buf,0,30);
//							  esp8266.len = 0;	
//							  nrf_gpio_pin_toggle(LED_4);
//						}
//            					
//				//} 
//    }

	//		taskEXIT_CRITICAL();
}

/*****************************************************************************************
 * ��  �� : main����
 * ��  �� : ��
 * ����ֵ : ��
 *****************************************************************************************/ 
int main(void)
{
	 uint32_t err_code;

		ret_code_t err_code1;
	  err_code = nrf_drv_clock_init();
	  APP_ERROR_CHECK(err_code1);
	
	
	  LEDS_CONFIGURE(LEDS_MASK);
    LEDS_OFF(LEDS_MASK); //LED��ʼ״̬ΪϨ��
	
	  //����P0.25Ϊ���������ESP8266��λ
	  nrf_gpio_cfg_output(ESP8266_RST);
	  nrf_gpio_pin_set(ESP8266_RST);
    //���ڳ�ʼ��
    uart_config();
	  //ESP8266ģ���ʼ��

			//��λ����ESP8266ģ�飬ģ����Ӧ��OK
		ESP8266_Reset(ESP8266_RST_EXT);
		nrf_delay_ms(1000);
	
		APP_ERROR_CHECK(err_code);
	  xTaskCreate(start_toggle_task_function, "start", configMINIMAL_STACK_SIZE + 24, NULL, 1, &start_toggle_task_handle);
    vTaskStartScheduler();
}
/********************************************END FILE*******************************************/
