
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
TaskHandle_t StartTask_Handler;
TaskHandle_t WIFI_Task_Handler;
TaskHandle_t my_task_1_Handler;
SemaphoreHandle_t BinarySemaphore;
EventGroupHandle_t Event_Handle = NULL;     //�¼���־�飨λ0��WIFI����״̬ λ1��PING������2S���ٷ���ģʽ��
const int WIFI_CONECT = (0x01 << 0);        //�����¼������λ 0������������ģʽ��ֵ1��ʾ�Ѿ����ӣ�0��ʾδ����
const int PING_MODE   = (0x01 << 1);        //�����¼������λ 1��PING����������ģʽ��1��ʾ����30S����ģʽ��0��ʾδ�������ͻ���2S���ٷ���ģʽ

static void start_toggle_task_function(void *pvParameter)
{      
	

	while(1)
{
	  ESP8266_SetUp();
		xEventGroupSetBits(Event_Handle, WIFI_CONECT);  //�����������ӣ��׳��¼���־ 
		vTaskSuspend(NULL);	    						//�����������ӣ������Լ����������̬�������ɹ���תΪ����̬ʱ�������ִ����ȥ��
		xEventGroupClearBits(Event_Handle, WIFI_CONECT);//����������wifi�ѶϿ�������¼���־������ִ�б�������������
	}		
}
	char data_of_sensor[500] = {0};
void my_task_1(void *pvParameters)
{

//	int temperature = 0;
	while(1)
	{
		//�����������¼�����ִ�д����񣬷������
		xEventGroupWaitBits((EventGroupHandle_t	)Event_Handle,		
							(EventBits_t		)WIFI_CONECT,
							(BaseType_t			)pdFALSE,				
							(BaseType_t			)pdTRUE,
							(TickType_t			)portMAX_DELAY);
//		  temperature=temperature+1;
	//		sprintf(data_of_sensor, "POST /devices/704931691/datapoints?type=3 HTTP/1.1\r\napi-key:QW=lOU0zHy9M1jo8NM=XCnjgREY=\r\nHost:api.heclouds.com\r\nContent-Length:20\r\n\r\n{\"temp\":\"%d\"}\r\n", temperature);  	
	//		while(!ESP8266_ExeCMD(data_of_sensor,NULL,NULL,1000)){} 		
	//		nrf_delay_ms(1000);				
	//			vTaskDelay(1);
			while(!ESP8266_ExeCMD("POST /devices/704931691/datapoints?type=3 HTTP/1.1\r\napi-key:QW=lOU0zHy9M1jo8NM=XCnjgREY=\r\nHost:api.heclouds.com\r\nContent-Length:13\r\n\r\n{\"temp\":1122}\r\n",NULL,NULL,1000)){} 
//			nrf_delay_ms(10000);
//			while(!ESP8266_ExeCMD("POST /devices/704931691/datapoints?type=3 HTTP/1.1\r\napi-key:QW=lOU0zHy9M1jo8NM=XCnjgREY=\r\nHost:api.heclouds.com\r\nContent-Length:13\r\n\r\n{\"temp\":3344}\r\n",NULL,NULL,1000)){} 
	}
}


void my_start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); //�����ٽ���
	
	//������ֵ�ź���
	BinarySemaphore = xSemaphoreCreateBinary();	
	//�¼���־�飬���ڱ�־wifi����״̬�Լ�ping����״̬
	Event_Handle = xEventGroupCreate(); 
	//������������Ϣ����Ϣ����
//	Message_Queue = xQueueCreate(MESSAGE_DATA_TX_NUM, MESSAGE_DATA_TX_LEN); 
	
	//���񴴽�����������1.������ 2.�������� 3.�����ջ��С 3.���ݸ��������Ĳ��� 4.�������ȼ� 5.������ƿ�
	//����WIFI����
    xTaskCreate(start_toggle_task_function, 				"wifi_task", 				128, NULL, 7, &WIFI_Task_Handler); 			
	//����MQTT����崦������
      xTaskCreate(my_task_1,"my_task_1",  128, NULL, 6, &my_task_1_Handler); 			
//	//����MQTT���ݽ��շ��ͻ��崦������
//    xTaskCreate(mqtt_buffer_rx_tx_task, "mqtt_buffer_rx_tx_task", 	256, NULL, 5, &MQTT_RxTx_Task_Handler); 
//	//����led��������
//	xTaskCreate(my_led2_task, 			"my_led2_task",				128, NULL, 4, &Led2_Task_Handler);  
//    //����DHT11������ʪ�ȴ�����
//    xTaskCreate(my_dht11_task, 			"my_dht11_task", 			128, NULL, 3, &DHT11_Task_Handler);
//    //����SUN���񣬹��մ�����
//    xTaskCreate(my_sun_task, 			"my_sun_task",        		128, NULL, 3, &SUN_Task_Handler);	
//	//�������������ݴ������񣬴�������͵Ĵ��������ݣ�����MQTT���ݷ��ͻ�����
//    xTaskCreate(data_tx_to_buffer_task, "data_tx_to_buffer_task", 	512, NULL, 2, &DATA_TX_Task_Handler); 
			
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
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
	
		APP_ERROR_CHECK(err_code);
	  xTaskCreate(my_start_task, "start", configMINIMAL_STACK_SIZE + 24, NULL, 1, &start_toggle_task_handle);
    vTaskStartScheduler();
}
/********************************************END FILE*******************************************/
