#ifndef TASK_H
#define TASK_H

/* Includes */
#include "main.h"
#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function declarations */
void MainInit(void);
void MainTask(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/* External variables */
extern uint32_t tick;

#ifdef __cplusplus
}
#endif

#endif /* TASK_H */