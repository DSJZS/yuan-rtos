/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ipc.h"
#include "mutex.h"
#include "portable.h"
#include "queue.h"
#include "usart.h"
#include "gpio.h"
#include <stdarg.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task.h"
#include "scheduler.h"
#include "timer.h"
#include "kernel.h"
#include "semaphore.h"
#include "yr_def.h"
/* USER CODE END Includes */

void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
yr_task_t task0,task1,task2,task3;
yr_uint8_t task0_stack[1024];
yr_uint8_t task1_stack[1024];
yr_uint8_t task2_stack[1024];
yr_uint8_t task3_stack[1024];
yr_mutex_t mux0;
yr_queue_t queue0;
yr_uint8_t queue0_buffer[256];

void task_entry(void *param)
{
  int index = (int)param;
  int count = index;
  int receive = 0;
  yr_uint32_t pre_ticks = 0;

  for(;;) {
    yr_queue_receive( &queue0, &receive, YR_WAIT_FOREVER);
    yr_mutex_take( &mux0, YR_WAIT_FOREVER);
    YR_DEBUG_LOG( YR_DEBUG_INFO, "task %d : count = %d, receive = %d\r\n", index, count++, receive);
    yr_mutex_give( &mux0);
    yr_task_sleep_until( &pre_ticks, YR_MS_TO_TICKS(1000 - index * 100));
  }
}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void yr_putc(char c)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();

  YR_DEBUG_LOG(YR_DEBUG_INFO, "Start\r\n");

  yr_kernel_init();

  yr_mutex_init( &mux0, YR_IPC_FLAG_PRIO);
  yr_queue_init( &queue0, sizeof(int), queue0_buffer, sizeof(queue0_buffer), YR_IPC_FLAG_PRIO);
  yr_task_create( &task0, task_entry, (void *)0, task0_stack,sizeof(task0_stack), 0);
  yr_task_create( &task1, task_entry, (void *)1, task1_stack,sizeof(task1_stack), 1);
  yr_task_create( &task2, task_entry, (void *)2, task2_stack,sizeof(task2_stack), 2);
  yr_task_create( &task3, task_entry, (void *)3, task3_stack,sizeof(task3_stack), 3);

  yr_task_start( &task0);
  yr_task_start( &task1);
  yr_task_start( &task2);
  yr_task_start( &task3);

  yr_kernel_start();

  while (1)
  {
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif /* USE_FULL_ASSERT */
