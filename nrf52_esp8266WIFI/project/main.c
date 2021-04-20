
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
SemaphoreHandle_t BinarySemaphore;
EventGroupHandle_t Event_Handle = NULL;     //事件标志组（位0：WIFI连接状态 位1：PING心跳包2S快速发送模式）
const int WIFI_CONECT = (0x01 << 0);        //设置事件掩码的位 0；服务器连接模式，值1表示已经连接，0表示未连接
const int PING_MODE   = (0x01 << 1);        //设置事件掩码的位 1；PING心跳包发送模式，1表示开启30S发送模式，0表示未开启发送或开启2S快速发送模式

static void start_toggle_task_function(void *pvParameter)
{      
	
//		taskENTER_CRITICAL();
//	
//	//创建二值信号量
	BinarySemaphore = xSemaphoreCreateBinary();	
	//事件标志组，用于标志wifi连接状态以及ping发送状态
	Event_Handle = xEventGroupCreate(); 
	
	  ESP8266_SetUp();
		xEventGroupSetBits(Event_Handle, WIFI_CONECT);  //服务器已连接，抛出事件标志 
		vTaskSuspend(NULL);	    						//服务器已连接，挂起自己，进入挂起态（任务由挂起转为就绪态时在这继续执行下去）
		xEventGroupClearBits(Event_Handle, WIFI_CONECT);//服务器或者wifi已断开，清除事件标志，继续执行本任务，重新连接 
	
//	while (true)
//    {
//				//串口接收到数据?
//			 // if(esp8266.len > 1) 
//			//	{
//            if(strstr(esp8266.buf,"D1")!=NULL)	    //收到翻转LED1指示灯状态的的指令
//						{
//							  nrf_gpio_pin_toggle(LED_1);		
//                esp8266.len = 0;								
//							  memset(esp8266.buf,0,30);
//						}
//						else if(strstr(esp8266.buf,"D2")!=NULL) //收到翻转LED2指示灯状态的的指令
//						{
//							  memset(esp8266.buf,0,30);
//							  esp8266.len = 0;	
//							  nrf_gpio_pin_toggle(LED_2);
//						}		
//            else if(strstr(esp8266.buf,"D3")!=NULL) //收到翻转LED3指示灯状态的的指令
//						{
//							  memset(esp8266.buf,0,30);
//							  esp8266.len = 0;	
//							  nrf_gpio_pin_toggle(LED_3);
//						}	
//            else if(strstr(esp8266.buf,"D4")!=NULL) //收到翻转LED4指示灯状态的的指令
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
	  //ESP8266模块初始化

			//复位重启ESP8266模块，模块响应：OK
		ESP8266_Reset(ESP8266_RST_EXT);
		nrf_delay_ms(1000);
	
		APP_ERROR_CHECK(err_code);
	  xTaskCreate(start_toggle_task_function, "start", configMINIMAL_STACK_SIZE + 24, NULL, 1, &start_toggle_task_handle);
    vTaskStartScheduler();
}
/********************************************END FILE*******************************************/
