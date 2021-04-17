
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

/*
P0.31：UART_TXD   ：串口发送
P0.30：UART_RXD   ：串口接收
P0.28：ESP8266复位

*/
TaskHandle_t start_toggle_task_handle;

static void start_toggle_task_function(void *pvParameter)
{      
	
		//	taskENTER_CRITICAL();
			while (true)
    {
				//串口接收到数据?
			 // if(esp8266.len > 1) 
			//	{
            if(strstr(esp8266.buf,"D1")!=NULL)	    //收到翻转LED1指示灯状态的的指令
						{
							  nrf_gpio_pin_toggle(LED_1);		
                esp8266.len = 0;								
							  memset(esp8266.buf,0,30);
						}
						else if(strstr(esp8266.buf,"D2")!=NULL) //收到翻转LED2指示灯状态的的指令
						{
							  memset(esp8266.buf,0,30);
							  esp8266.len = 0;	
							  nrf_gpio_pin_toggle(LED_2);
						}		
            else if(strstr(esp8266.buf,"D3")!=NULL) //收到翻转LED3指示灯状态的的指令
						{
							  memset(esp8266.buf,0,30);
							  esp8266.len = 0;	
							  nrf_gpio_pin_toggle(LED_3);
						}	
            else if(strstr(esp8266.buf,"D4")!=NULL) //收到翻转LED4指示灯状态的的指令
						{
							  memset(esp8266.buf,0,30);
							  esp8266.len = 0;	
							  nrf_gpio_pin_toggle(LED_4);
						}
            					
				//} 
    }

		//	taskEXIT_CRITICAL();
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
	  ESP8266_SetUp();
	
		APP_ERROR_CHECK(err_code);
	  xTaskCreate(start_toggle_task_function, "start", configMINIMAL_STACK_SIZE + 24, NULL, 1, &start_toggle_task_handle);
    vTaskStartScheduler();
}
/********************************************END FILE*******************************************/
