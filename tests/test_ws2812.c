#include <assert.h>
#include <stdio.h>
#include "ws2812.c"
TIM_HandleTypeDef htim3;
static int failStart;
static int failStop;
static int stopped;
uint8_t HAL_TIM_PWM_Start_DMA(TIM_HandleTypeDef *t,uint32_t c,uint32_t *p,uint16_t n) {
    assert(t==&htim3 && c==TIM_CHANNEL_1 && n==48);
    assert((uintptr_t)p % 4U == 0U);
    stopped=0;
    return failStart ? 1:HAL_OK;
}
uint8_t HAL_TIM_PWM_Stop_DMA(TIM_HandleTypeDef *t,uint32_t c) {
    assert(t==&htim3 && c==TIM_CHANNEL_1);stopped=1;return failStop ? 1:HAL_OK;
}
static void verifyFrame(void) {
    uint8_t expected[WS2812_LED_COUNT][3];
    memcpy(expected,strip.leds,sizeof(expected));
    assert(WS2812_Send()==WS2812_OK);
    assert(WS2812_Send()==WS2812_BUSY);
    uint8_t brightness = strip.brightness;
    assert(WS2812_SetBrightness(17)==WS2812_BUSY && strip.brightness==brightness);
    assert(WS2812_SetHSV(0, 60, 255, 255)==WS2812_BUSY);
    assert(WS2812_FillHSV(120, 255, 255)==WS2812_BUSY);
    assert(memcmp(expected,strip.leds,sizeof(expected))==0);
    TIM_HandleTypeDef other={HAL_TIM_ACTIVE_CHANNEL_1};
    HAL_TIM_PWM_PulseFinishedCallback(&other);assert(strip.sentChunks==0);
    htim3.Channel=2;HAL_TIM_PWM_PulseFinishedCallback(&htim3);assert(strip.sentChunks==0);
    htim3.Channel=HAL_TIM_ACTIVE_CHANNEL_1;
    unsigned total=WS2812_LED_COUNT+4;
    for(unsigned chunk=0;chunk<total;chunk++) {
        unsigned offset=(chunk%2)*24;
        for(unsigned bit=0;bit<24;bit++) {
            unsigned sample=strip.pwmData[offset+bit];
            if(chunk<2 || chunk>=WS2812_LED_COUNT+2) assert(sample==0);
            else {
                unsigned value=(expected[chunk-2][bit/8] * strip.brightness) / 255U;
                assert(sample==((value & (0x80>>(bit%8)))?95:48));
            }
        }
        if(chunk%2) HAL_TIM_PWM_PulseFinishedCallback(&htim3);
        else HAL_TIM_PWM_PulseFinishedHalfCpltCallback(&htim3);
        assert(stopped==(chunk==total-1));
    }
    assert(WS2812_GetState()==(failStop ? WS2812_ERROR : WS2812_OK));
}
int main(void) {
    WS2812_Init();WS2812_SetRGB(0,255,0,0);
    assert(strip.leds[0][0]==0 && strip.leds[0][1]==255 && strip.leds[0][2]==0);
    if(WS2812_LED_COUNT>=3) {
        WS2812_SetRGB(1,0,255,0);WS2812_SetRGB(2,0,0,255);
        assert(strip.leds[1][0]==255 && strip.leds[2][2]==255);
    }
    verifyFrame();verifyFrame();
    WS2812_FillRGB(23,42,81);verifyFrame();
    failStart=1;assert(WS2812_Send()==WS2812_ERROR);
    assert(WS2812_GetState()==WS2812_ERROR);failStart=0;verifyFrame();
    failStop=1;verifyFrame();failStop=0;verifyFrame();
    assert(WS2812_Send()==WS2812_OK);HAL_TIM_ErrorCallback(&htim3);
    assert(WS2812_GetState()==WS2812_ERROR && stopped);verifyFrame();
    failStop=1;
    assert(WS2812_Send()==WS2812_OK);HAL_TIM_ErrorCallback(&htim3);
    assert(WS2812_GetState()==WS2812_ERROR && stopped);
    failStop=0;verifyFrame();
    static const uint16_t hsvCases[][6] = {
        {0,255,255,255,0,0}, {60,255,255,255,255,0},
        {120,255,255,0,255,0}, {180,255,255,0,255,255},
        {240,255,255,0,0,255}, {300,255,255,255,0,255},
        {30,255,255,255,127,0}, {359,255,255,255,0,5},
        {59,255,255,255,250,0}, {61,255,255,251,255,0},
        {210,0,123,123,123,123}, {170,255,0,0,0,0},
        {0,128,128,128,64,64}
    };
    for (unsigned n=0; n<sizeof(hsvCases)/sizeof(hsvCases[0]); ++n) {
        const uint16_t *c=hsvCases[n];
        assert(WS2812_SetHSV(0,c[0],c[1],c[2])==WS2812_OK);
        assert(strip.leds[0][0]==c[4] && strip.leds[0][1]==c[3] && strip.leds[0][2]==c[5]);
        assert(WS2812_FillHSV(c[0],c[1],c[2])==WS2812_OK);
        for(unsigned i=0;i<WS2812_LED_COUNT;++i)
            assert(strip.leds[i][0]==c[4] && strip.leds[i][1]==c[3] && strip.leds[i][2]==c[5]);
        verifyFrame();
    }
    uint8_t saved[WS2812_LED_COUNT][3];
    memcpy(saved,strip.leds,sizeof(saved));
    assert(WS2812_SetHSV(0,360,255,255)==WS2812_ERROR);
    assert(WS2812_FillHSV(65535,255,255)==WS2812_ERROR);
    assert(WS2812_SetHSV(WS2812_LED_COUNT,0,255,255)==WS2812_ERROR);
    assert(memcmp(saved,strip.leds,sizeof(saved))==0);
    assert(WS2812_FillRGB(255,128,1)==WS2812_OK);
    memcpy(saved,strip.leds,sizeof(saved));
    static const uint8_t levels[]={0,1,128,254,255};
    for(unsigned n=0;n<sizeof(levels);++n) {
        assert(WS2812_SetBrightness(levels[n])==WS2812_OK);
        verifyFrame();
        assert(memcmp(saved,strip.leds,sizeof(saved))==0);
    }
    WS2812_Init();
    assert(strip.brightness==255);
    printf("PASS: %u LEDs, GRB order, reset, repeat, HAL callbacks, errors, HSV and brightness\n",WS2812_LED_COUNT);
    return 0;
}
