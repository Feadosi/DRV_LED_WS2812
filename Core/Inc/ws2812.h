/**
 ** Company: "-----"
 ** Author: Liamachka Feadosi
 ** Created on: 16.09.2026
 **/

#ifndef WS2812_H_
#define WS2812_H_

/**************************INCLUDES***********************************/
#include <stdint.h>
#include "tim.h"
#include <string.h>
/*************************END INCLUDES********************************/

/**************************DEFINES************************************/
#define WS2812_LED_COUNT      1U       // Physical strip length
#define WS2812_TIMER          htim1     // Timer configured in CubeMX
#define WS2812_CHANNEL        TIM_CHANNEL_1
#define WS2812_ACTIVE_CHANNEL HAL_TIM_ACTIVE_CHANNEL_1

#define WS2812_COLOR_MAX      255U
#define WS2812_HUE_RANGE      360U
#define WS2812_HUE_SECTOR     60U

#define WS2812_BYTES_PER_LED  3U
#define WS2812_BITS_PER_BYTE  8U
#define WS2812_BITS_PER_LED   24U

#define WS2812_PWM_SIZE       (2U * WS2812_BITS_PER_LED)
#define WS2812_RESET_HALVES   12U        // 288 low periods = 360 us per reset interval
#define WS2812_ZERO_TICKS     48U       // Timer clock 120 MHz, ARR = 149
#define WS2812_ONE_TICKS      95U       // Period = 1.25us; ONE_TICK = 0.8us = 95; ZERO_TICK = 0.4us = 48

/************************END DEFINES**********************************/

/*************************ENUMERATIONS********************************/
typedef enum
{
  WS2812_OK = 0,
  WS2812_BUSY,
  WS2812_ERROR
} WS2812_State_t;
/***********************END ENUMERATIONS******************************/

/***************************STRUCTS***********************************/
typedef struct
{
  uint8_t pwmData[WS2812_PWM_SIZE];   // PWM buffer for two LEDs
  uint8_t leds[WS2812_LED_COUNT][WS2812_BYTES_PER_LED];
  uint8_t brightness;               // Global output scale, 0..255
  uint32_t nextChunk;
  uint32_t sentChunks;
  volatile WS2812_State_t state;
} WS2812_Context_t;
/*************************END STRUCTS*********************************/

#ifdef __cplusplus
extern "C" {
#endif

/*********************PROTOTYPE FUNCTIONS*****************************/
void WS2812_Init(void);
WS2812_State_t WS2812_SetRGB(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
WS2812_State_t WS2812_FillRGB(uint8_t red, uint8_t green, uint8_t blue);
WS2812_State_t WS2812_SetBrightness(uint8_t brightness);
WS2812_State_t WS2812_SetHSV(uint16_t index, uint16_t hue, uint8_t saturation, uint8_t value);
WS2812_State_t WS2812_FillHSV(uint16_t hue, uint8_t saturation, uint8_t value);
WS2812_State_t WS2812_Send(void);
WS2812_State_t WS2812_GetState(void);
/*******************END PROTOTYPE FUNCTIONS***************************/

#ifdef __cplusplus
}
#endif

#endif /* WS2812_H_ */
