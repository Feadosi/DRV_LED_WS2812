/**
 ** Company: "-----"
 ** Author: Liamachka Feadosi
 ** Created on: 16.09.2026
 **/

/***************************INCLUDES**********************************/
#include "ws2812.h"
/*************************END INCLUDES********************************/

/**************************VARIABLES**********************************/
static WS2812_Context_t strip;
/************************END VARIABLES********************************/

/**************************FUNCTIONS**********************************/
/** ===========================================
 ** Function name : WS2812_Init()
 ** Description   : Clear the buffers. Call once after CubeMX initialization.
 ** Parameters    : None
 ** Return        : None
 ** ============================================*/
void WS2812_Init(void)
{
  (void)memset(&strip, 0, sizeof(strip));
  strip.brightness = WS2812_COLOR_MAX;
}

/** ===========================================
 ** Function name : WS2812_SetRGB()
 ** Description   : Store one pixel in GRB order. Edit only while idle.
 ** Parameters    : index - Zero-based LED; red, green, blue - 0..255
 ** Return        : WS2812_OK, WS2812_BUSY or WS2812_ERROR
 ** ============================================*/
WS2812_State_t WS2812_SetRGB(uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
  if (index >= WS2812_LED_COUNT)
  {
    return WS2812_ERROR;
  }
  else if(strip.state == WS2812_BUSY)
  {
    return WS2812_BUSY;
  }
  else
  {
    strip.leds[index][0] = green;
    strip.leds[index][1] = red;
    strip.leds[index][2] = blue;
    return WS2812_OK;
  }
}

/** ===========================================
 ** Function name : WS2812_FillRGB()
 ** Description   : Set the entire strip to one RGB color.
 ** Parameters    : red, green, blue - 0..255
 ** Return        : WS2812_OK, WS2812_BUSY or WS2812_ERROR
 ** ============================================*/
WS2812_State_t WS2812_FillRGB(uint8_t red, uint8_t green, uint8_t blue)
{
  WS2812_State_t res;

  for (uint16_t index = 0; index < WS2812_LED_COUNT; ++index)
  {
    res = WS2812_SetRGB(index, red, green, blue);
    if (res != WS2812_OK)
    {
      return res;
    }
  }
  return WS2812_OK;
}

/** ===========================================
 ** Function name : WS2812_SetBrightness()
 ** Description   : Set global output brightness without changing stored colors.
 ** Parameters    : brightness - 0 (off) to 255 (full output)
 ** Return        : WS2812_OK or WS2812_BUSY
 ** ============================================*/
WS2812_State_t WS2812_SetBrightness(uint8_t brightness)
{
  if (strip.state == WS2812_BUSY)
  {
    return WS2812_BUSY;
  }
  strip.brightness = brightness;
  return WS2812_OK;
}

/** ===========================================
 ** Function name : WS2812_HSVtoRGB()
 ** Description   : Convert validated HSV components using integer arithmetic.
 ** Parameters    : hue - 0..359; saturation, value - 0..255; rgb - Output bytes
 ** Return        : None
 ** ============================================*/
static void WS2812_HSVtoRGB(uint16_t hue, uint8_t saturation, uint8_t value, uint8_t rgb[WS2812_BYTES_PER_LED])
{
  uint32_t chroma = ((uint32_t)value * saturation) / WS2812_COLOR_MAX;
  uint8_t low = (uint8_t)(value - chroma);
  uint8_t rise = (uint8_t)(low + (chroma * (hue % WS2812_HUE_SECTOR)) / WS2812_HUE_SECTOR);
  uint8_t fall = (uint8_t)(value - (chroma * (hue % WS2812_HUE_SECTOR)) / WS2812_HUE_SECTOR);

  switch (hue / WS2812_HUE_SECTOR)
  {
    case 0U: rgb[0] = value; rgb[1] = rise;  rgb[2] = low;   break;
    case 1U: rgb[0] = fall;  rgb[1] = value; rgb[2] = low;   break;
    case 2U: rgb[0] = low;   rgb[1] = value; rgb[2] = rise;  break;
    case 3U: rgb[0] = low;   rgb[1] = fall;  rgb[2] = value; break;
    case 4U: rgb[0] = rise;  rgb[1] = low;   rgb[2] = value; break;
    default: rgb[0] = value; rgb[1] = low;   rgb[2] = fall;  break;
  }
}

/** ===========================================
 ** Function name : WS2812_SetHSV()
 ** Description   : Store one LED color after conversion from HSV to RGB.
 ** Parameters    : index - Zero-based LED; hue - 0..359; saturation, value - 0..255
 ** Return        : WS2812_OK, WS2812_BUSY or WS2812_ERROR for invalid index/hue
 ** ============================================*/
WS2812_State_t WS2812_SetHSV(uint16_t index, uint16_t hue, uint8_t saturation, uint8_t value)
{
  uint8_t rgb[WS2812_BYTES_PER_LED];
  if ((index >= WS2812_LED_COUNT) || (hue >= WS2812_HUE_RANGE))
  {
    return WS2812_ERROR;
  }
  if (strip.state == WS2812_BUSY)
  {
    return WS2812_BUSY;
  }
  WS2812_HSVtoRGB(hue, saturation, value, rgb);
  return WS2812_SetRGB(index, rgb[0], rgb[1], rgb[2]);
}

/** ===========================================
 ** Function name : WS2812_FillHSV()
 ** Description   : Convert one HSV color and fill the entire strip.
 ** Parameters    : hue - 0..359; saturation, value - 0..255
 ** Return        : WS2812_OK, WS2812_BUSY or WS2812_ERROR for invalid hue
 ** ============================================*/
WS2812_State_t WS2812_FillHSV(uint16_t hue, uint8_t saturation, uint8_t value)
{
  uint8_t rgb[WS2812_BYTES_PER_LED];
  if (hue >= WS2812_HUE_RANGE)
  {
    return WS2812_ERROR;
  }
  if (strip.state == WS2812_BUSY)
  {
    return WS2812_BUSY;
  }
  WS2812_HSVtoRGB(hue, saturation, value, rgb);
  return WS2812_FillRGB(rgb[0], rgb[1], rgb[2]);
}

/** ===========================================
 ** Function name : WS2812_FillHalf()
 ** Description   : Prepare one LED or one reset chunk in the free half.
 ** Parameters    : half - 0 or 1
 ** Return        : None
 ** ============================================*/
static void WS2812_FillHalf(uint8_t half)
{
  uint8_t *data;
  uint32_t chunk;
  uint32_t index;

  data = &strip.pwmData[half * WS2812_BITS_PER_LED];
  chunk = strip.nextChunk;
  strip.nextChunk++;
  if (chunk < WS2812_RESET_HALVES || chunk >= WS2812_RESET_HALVES + WS2812_LED_COUNT)
  {
    (void)memset(data, 0, WS2812_BITS_PER_LED); // Leading and trailing low intervals
    return;
  }

  index = chunk - WS2812_RESET_HALVES;
  for (uint8_t channel = 0U; channel < WS2812_BYTES_PER_LED; ++channel)
  {
    uint8_t value = (uint8_t)(((uint32_t)strip.leds[index][channel] * strip.brightness) / WS2812_COLOR_MAX);
    for (uint8_t bit = 0U; bit < WS2812_BITS_PER_BYTE; ++bit)
    {
      data[channel * WS2812_BITS_PER_BYTE + bit] = ((value & (0x80U >> bit)) != 0U) ? WS2812_ONE_TICKS : WS2812_ZERO_TICKS;
    }
  }
}

/** ===========================================
 ** Function name : WS2812_Send()
 ** Description   : Start circular PWM DMA; HAL installs its own callbacks.
 ** Parameters    : None
 ** Return        : WS2812_OK, WS2812_BUSY or WS2812_ERROR
 ** ============================================*/
WS2812_State_t WS2812_Send(void)
{
  if (strip.state == WS2812_BUSY)
  {
    return WS2812_BUSY;
  }

  strip.nextChunk = 0;
  strip.sentChunks = 0;
  WS2812_FillHalf(0);
  WS2812_FillHalf(1);
  strip.state = WS2812_BUSY;

  if (HAL_TIM_PWM_Start_DMA(&WS2812_TIMER, WS2812_CHANNEL, (uint32_t *)strip.pwmData, WS2812_PWM_SIZE) != HAL_OK)
  {
    strip.state = WS2812_ERROR;
    return WS2812_ERROR;
  }

  return WS2812_OK;
}

/** ===========================================
 ** Function name : WS2812_GetState()
 ** Description   : Read the current transfer state.
 ** Parameters    : None
 ** Return        : WS2812_OK, WS2812_BUSY or WS2812_ERROR
 ** ============================================*/
WS2812_State_t WS2812_GetState(void)
{
  return strip.state;
}

/** ===========================================
 ** Function name : WS2812_HalfSent()
 ** Description   : Refill the consumed half or stop after the trailing reset.
 ** Parameters    : timer - HAL timer; half - 0 or 1
 ** Return        : None
 ** ============================================*/
static void WS2812_HalfSent(TIM_HandleTypeDef *timer, uint8_t half)
{
  if ((timer != &WS2812_TIMER) || (timer->Channel != WS2812_ACTIVE_CHANNEL) || (strip.state != WS2812_BUSY))
  {
    return;
  }

  ++strip.sentChunks;   // Increasing the number of sent chunks

  if (strip.sentChunks >= (WS2812_LED_COUNT + 2U * WS2812_RESET_HALVES))
  {
    if (HAL_TIM_PWM_Stop_DMA(timer, WS2812_CHANNEL) == HAL_OK)
    {
      strip.state = WS2812_OK;
    }
    else
    {
      strip.state = WS2812_ERROR;
    }
  }
  else
  {
    WS2812_FillHalf(half);
  }
}

/** ===========================================
 ** Function name : HAL_TIM_PWM_PulseFinishedHalfCpltCallback()
 ** Description   : Standard HAL callback for the first DMA half.
 ** Parameters    : timer - HAL timer
 ** Return        : None
 ** ============================================*/
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *timer)
{
  WS2812_HalfSent(timer, 0);
}

/** ===========================================
 ** Function name : HAL_TIM_PWM_PulseFinishedCallback()
 ** Description   : Standard HAL callback for the second DMA half.
 ** Parameters    : timer - HAL timer
 ** Return        : None
 ** ============================================*/
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *timer)
{
  WS2812_HalfSent(timer, 1);
}

/** ===========================================
 ** Function name : HAL_TIM_ErrorCallback()
 ** Description   : Standard HAL callback for a failed DMA transfer.
 ** Parameters    : timer - HAL timer
 ** Return        : None
 ** ============================================*/
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *timer)
{
  if ((timer != &WS2812_TIMER) || (timer->Channel != WS2812_ACTIVE_CHANNEL) || (strip.state != WS2812_BUSY))
  {
    return;
  }

  (void)HAL_TIM_PWM_Stop_DMA(timer, WS2812_CHANNEL); // State remains ERROR regardless of the stop result
  strip.state = WS2812_ERROR;
}
/************************END FUNCTIONS********************************/
