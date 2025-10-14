#include "gpio.h"
#include "main.h"
#include "task.h"
#include "usart.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void encode(uint32_t tick, float value);
void decode(uint8_t *data);

uint8_t rx_data[9] = {0,0,0,0,0,0,0,0,0};
uint8_t tx_data[9] = {0};
uint8_t true_data[9] = {0};

struct UartCommData{
    uint32_t tick;
    float value;
};

void MainInit(void){ 
    
    HAL_UART_Receive_DMA(&huart2,rx_data,9);

    HAL_TIM_Base_Start_IT(&htim3);
}

uint32_t true_tick = 0;
struct UartCommData UartTransmitDate;
struct UartCommData UartReceiveDate;

void MainTask(void){
    
    true_tick ++;
    
    UartTransmitDate.tick = true_tick;
    UartTransmitDate.value =  sin((float)true_tick/1000) * 3000;
    
    encode(UartTransmitDate.tick,UartTransmitDate.value);

    if (true_tick % 1000 == 0){
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

    }

    if (true_tick % 100 == 0){

        HAL_UART_Transmit_DMA(&huart1,tx_data,9);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    
    if (htim == &htim3){

        MainTask();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart == &huart2){
        
        decode(rx_data);
        HAL_UART_Receive_DMA(&huart2, rx_data, 9);
    }
}


void encode(uint32_t tick, float value){
    tx_data[0] = 0xAA;
    tx_data[1] = 0xBB;
    tx_data[2] = 0xCC;
    
    tx_data[3] = (tick >> 24) & 0xFF;
    tx_data[4] = (tick >> 16) & 0xFF;
    tx_data[5] = (tick >> 8) & 0xFF;
    tx_data[6] = tick & 0xFF;
    
    int16_t int_value = (int16_t)value + 3000;
    tx_data[7] = (int_value >> 8) & 0xFF;
    tx_data[8] = int_value & 0xFF;
}

void decode(uint8_t *data){
    if (data[0] != 0xAA || data[1] != 0xBB || data[2] != 0xCC) {
        return;
    }
    
    UartReceiveDate.tick = ((uint32_t)data[3] << 24) |
                          ((uint32_t)data[4] << 16) |
                          ((uint32_t)data[5] << 8) |
                          data[6];
    
    int16_t raw_value = (int16_t)((data[7] << 8) | data[8]);
    UartReceiveDate.value = (float)raw_value - 3000;
      
}
