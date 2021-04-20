
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
#include "semphr.h" 	 //信号量
#include "queue.h"		 //队列
#include "event_groups.h"//事件标志组

/*
P0.31：UART_TXD   ：串口发送
P0.30：UART_RXD   ：串口接收
P0.28：ESP8266复位

*/
TaskHandle_t start_toggle_task_handle;
TaskHandle_t StartTask_Handler;
TaskHandle_t WIFI_Task_Handler;
SemaphoreHandle_t BinarySemaphore;
EventGroupHandle_t Event_Handle = NULL;     //事件标志组（位0：WIFI连接状态 位1：PING心跳包2S快速发送模式）
const int WIFI_CONECT = (0x01 << 0);        //设置事件掩码的位 0；服务器连接模式，值1表示已经连接，0表示未连接
const int PING_MODE   = (0x01 << 1);        //设置事件掩码的位 1；PING心跳包发送模式，1表示开启30S发送模式，0表示未开启发送或开启2S快速发送模式

static void start_toggle_task_function(void *pvParameter)
{      
	

	
	  ESP8266_SetUp();
		xEventGroupSetBits(Event_Handle, WIFI_CONECT);  //服务器已连接，抛出事件标志 
		vTaskSuspend(NULL);	    						//服务器已连接，挂起自己，进入挂起态（任务由挂起转为就绪态时在这继续执行下去）
		xEventGroupClearBits(Event_Handle, WIFI_CONECT);//服务器或者wifi已断开，清除事件标志，继续执行本任务，重新连接 
}


void my_start_task(void *pvParameters)
{
	taskENTER_CRITICAL(); //进入临界区
	
	//创建二值信号量
	BinarySemaphore = xSemaphoreCreateBinary();	
	//事件标志组，用于标志wifi连接状态以及ping发送状态
	Event_Handle = xEventGroupCreate(); 
	//创建传感器消息体消息队列
//	Message_Queue = xQueueCreate(MESSAGE_DATA_TX_NUM, MESSAGE_DATA_TX_LEN); 
	
	//任务创建函数参数；1.任务函数 2.任务名称 3.任务堆栈大小 3.传递给任务函数的参数 4.任务优先级 5.任务控制块
	//创建WIFI任务
    xTaskCreate(start_toggle_task_function, 				"wifi_task", 				128, NULL, 7, &WIFI_Task_Handler); 			
	//创建MQTT命令缓冲处理任务
//    xTaskCreate(my_mqtt_buffer_cmd_task,"my_mqtt_buffer_cmd_task",  128, NULL, 6, &MQTT_Cmd_Task_Handler); 			
//	//创建MQTT数据接收发送缓冲处理任务
//    xTaskCreate(mqtt_buffer_rx_tx_task, "mqtt_buffer_rx_tx_task", 	256, NULL, 5, &MQTT_RxTx_Task_Handler); 
//	//创建led控制任务
//	xTaskCreate(my_led2_task, 			"my_led2_task",				128, NULL, 4, &Led2_Task_Handler);  
//    //创建DHT11任务，温湿度传感器
//    xTaskCreate(my_dht11_task, 			"my_dht11_task", 			128, NULL, 3, &DHT11_Task_Handler);
//    //创建SUN任务，光照传感器
//    xTaskCreate(my_sun_task, 			"my_sun_task",        		128, NULL, 3, &SUN_Task_Handler);	
//	//创建传感器数据处理任务，处理待发送的传感器数据，移入MQTT数据发送缓冲区
//    xTaskCreate(data_tx_to_buffer_task, "data_tx_to_buffer_task", 	512, NULL, 2, &DATA_TX_Task_Handler); 
			
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}
/*****************************************************************************************
 * 描  述 : main函数
 * 入  参 : 无
 * 返回值 : 无
 *****************************************************************************************/ 
int main(void)
{
	 uint32_t err_code;

		ret_code_t err_code1;
	  err_code = nrf_drv_clock_init();
	  APP_ERROR_CHECK(err_code1);
	
	
	  LEDS_CONFIGURE(LEDS_MASK);
    LEDS_OFF(LEDS_MASK); //LED初始状态为熄灭
	
	  //配置P0.25为输出，用于ESP8266复位
	  nrf_gpio_cfg_output(ESP8266_RST);
	  nrf_gpio_pin_set(ESP8266_RST);
    //串口初始化
    uart_config();
	
		APP_ERROR_CHECK(err_code);
	  xTaskCreate(start_toggle_task_function, "start", configMINIMAL_STACK_SIZE + 24, NULL, 1, &start_toggle_task_handle);
    vTaskStartScheduler();
}
/********************************************END FILE*******************************************/
