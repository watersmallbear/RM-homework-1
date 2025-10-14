#include "gpio.h"
#include "main.h"
#include "task.h"
#include "usart.h"
#include "HW_can.hpp"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static CAN_RxHeaderTypeDef rx_header;

void UART_encode(uint32_t tick, float value);
void UART_decode(uint8_t *data);
void CAN_decode(uint8_t *data);
void CAN_encode(uint32_t tick, float value);

uint8_t rx_data[9] = {0,0,0,0,0,0,0,0,0};
uint8_t tx_data[9] = {0};
uint8_t can_tx_data[8] = {0};
uint8_t can_rx_data[8] = {0};
uint32_t true_tick = 0;

struct UartCommData{
    uint32_t tick;
    float value;
};

struct UartCommData UartTransmitDate;
struct UartCommData UartReceiveDate;

struct CANCommData
{
    uint32_t tick;
    float value1;
    uint8_t value2;
    bool flag1;
    bool flag2;
    bool flag3;
    bool flag4;
};

struct CANCommData CanReceiveData;
struct CANCommData CanTransmitData;

void MainInit(void){ 
    
    HAL_UART_Receive_DMA(&huart2,rx_data,9);

    CanFilter_Init(&hcan);
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);

    HAL_TIM_Base_Start_IT(&htim3);
}



void MainTask(void){
    
    true_tick ++;
    
    UartTransmitDate.tick = true_tick;
    UartTransmitDate.value =  sin((float)true_tick/1000) * 3000;

    CanTransmitData.tick = true_tick;
    CanTransmitData.value1 =  cos((float)true_tick/1000) * 3000;
    
    UART_encode(UartTransmitDate.tick,UartTransmitDate.value);
    CAN_encode(CanTransmitData.tick,CanTransmitData.value1);

    if (true_tick % 1000 == 0){
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

    }

    if (true_tick % 100 == 0){

        HAL_UART_Transmit_DMA(&huart1,tx_data,9);
    }
    CAN_Send_Msg(&hcan,can_tx_data,0x100,8);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    
    if (htim == &htim3){

        MainTask();
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, can_rx_data) ==
      HAL_OK) // 获得接收到的数据头和数据
  {
    if (rx_header.StdId == 0x100) { // 帧头校验
      CAN_decode(can_rx_data);                               // 校验通过进行具体数据处理
    }
  }
  HAL_CAN_ActivateNotification(
      hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // 再次使能FIFO0接收中断
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart == &huart2){
        
        UART_decode(rx_data);
        HAL_UART_Receive_DMA(&huart2, rx_data, 9);
    }
}

void UART_encode(uint32_t tick, float value){
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

void UART_decode(uint8_t *data){
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

void CAN_encode(uint32_t tick, float value){
   
    can_tx_data[0] = (tick >> 24) & 0xFF;
    can_tx_data[1] = (tick >> 16) & 0xFF;
    can_tx_data[2] = (tick >> 8) & 0xFF;
    can_tx_data[3] = tick & 0xFF;
    
    int16_t int_value = (int16_t)value + 3000;
    can_tx_data[4] = (int_value >> 8) & 0xFF;
    can_tx_data[5] = int_value & 0xFF;
}

void CAN_decode(uint8_t *data){
   
    CanReceiveData.tick = ((uint32_t)data[0] << 24) |
                          ((uint32_t)data[1] << 16) |
                          ((uint32_t)data[2] << 8) |
                          data[3];
    
    int16_t raw_value = (int16_t)((data[4] << 8) | data[5]);
    CanReceiveData.value1 = (float)raw_value - 3000;      
}
