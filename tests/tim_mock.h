#include <stdint.h>
typedef struct { uint32_t Channel; } TIM_HandleTypeDef;
extern TIM_HandleTypeDef htim3;
#define TIM_CHANNEL_1 0U
#define HAL_TIM_ACTIVE_CHANNEL_1 1U
#define HAL_OK 0U
uint8_t HAL_TIM_PWM_Start_DMA(TIM_HandleTypeDef *, uint32_t, uint32_t *, uint16_t);
uint8_t HAL_TIM_PWM_Stop_DMA(TIM_HandleTypeDef *, uint32_t);
