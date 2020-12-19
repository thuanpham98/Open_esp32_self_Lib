#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include <esp_err.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/dac.h"
#include "driver/timer.h"

#define TURN_ON_LED gpio_set_level(GPIO_NUM_27,0)
#define TURN_OFF_LED gpio_set_level(GPIO_NUM_27,1)

#define ALLOW_DATA gpio_set_level(GPIO_NUM_26,1)
#define UNALLOW_DATA gpio_set_level(GPIO_NUM_26,0)

#define R1_SET(x) gpio_set_level(GPIO_NUM_23,x)
#define G1_SET(x) gpio_set_level( GPIO_NUM_4,x)
#define B1_SET(x) gpio_set_level( GPIO_NUM_5,x)

#define R2_SET(x) gpio_set_level(GPIO_NUM_12,x)
#define G2_SET(x) gpio_set_level(GPIO_NUM_13,x)
#define B2_SET(x) gpio_set_level(GPIO_NUM_14,x)

#define PIXCEL_PART1(r,g,b) do{R1_SET(r);G1_SET(g);B1_SET(b);} while(0)
#define PIXCEL_PART2(r,g,b) do{R2_SET(r);G2_SET(g);B2_SET(b);} while(0)

#define CHOOSE_ROW(row) do{gpio_set_level(GPIO_NUM_15,(row>>0)&1);gpio_set_level(GPIO_NUM_19,(row>>1)&1);gpio_set_level(GPIO_NUM_21,(row>>2)&1);gpio_set_level(GPIO_NUM_22,(row>>3)&1);} while(0)

#define EMPTY_LINE do{TURN_ON_LED;}while(0)

#define LATCH do{ALLOW_DATA;UNALLOW_DATA;}while(0)

#define MAKE_CLOCK(x) do{gpio_set_level(x,1);gpio_set_level(x,0);} while(0)

static const char TAG[]="esp32_nek";

uint8_t red =0 ;
uint8_t green =0;
uint8_t blue =0;
uint8_t count=0;
uint8_t buffer[3][64];

extern "C" {
    void app_main(void);
    void send_whole_row(uint8_t _r1,uint8_t _g1,uint8_t _b1,uint8_t _r2,uint8_t _g2,uint8_t _b2);
    void set_pixcel(uint8_t _r,uint8_t _g,uint8_t _b,uint32_t row,uint32_t column);
    void IRAM_ATTR timer_group0_isr(void *arg);
}

void set_pixcel(uint8_t _r,uint8_t _g,uint8_t _b,uint32_t row,uint32_t column){

red =0;
green=0;
blue=0;
            
            CHOOSE_ROW(row);
            TURN_OFF_LED;

            for(int col = 0; col < 64 ; col ++){
                if(col < 3){
                    red=0;
                    green=0;
                    blue=0;
                }
                red = buffer[0][col];
                green =buffer[1][col];
                blue = buffer[2][col];

                // vTaskDelay((4)/portTICK_PERIOD_MS);
                MAKE_CLOCK(GPIO_NUM_25);

            }
            LATCH;
            TURN_ON_LED; 
            
            vTaskDelay((1)/portTICK_PERIOD_MS);

}

void IRAM_ATTR timer_group0_isr(void *arg){
    
    (count< red)? R1_SET(1):R1_SET(0);
    (count< green)? G1_SET(1):G1_SET(0);
    (count< blue)? B1_SET(1):B1_SET(0);
    if(count==255){
        // R1_SET(0);
        // G1_SET(0);
        // B1_SET(0);
        count=0;
    }else{
        count++;
    }
    
    TIMERG0.int_clr_timers.t1 = 1;
    TIMERG0.hw_timer[TIMER_1].config.alarm_en=TIMER_ALARM_EN;

}

void app_main(void)
{
    // ESP_LOGE(TAG,"%d",esp_get_free_heap_size());
    // vTaskDelay(1000/portTICK_PERIOD_MS);

    // data pin
    gpio_pad_select_gpio(GPIO_NUM_15); // A
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(GPIO_NUM_19); // B
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(GPIO_NUM_21); // C
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(GPIO_NUM_22); // D
    gpio_set_direction(GPIO_NUM_22, GPIO_MODE_OUTPUT);

    // Lat pin
    gpio_pad_select_gpio(GPIO_NUM_26); 
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_26,0);

    // OE Pin 
    gpio_pad_select_gpio(GPIO_NUM_27); 
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);

    // CLK PIN 
    gpio_pad_select_gpio(GPIO_NUM_25); 
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_25,0);

    // RPG pin , RED ( 255,0,0) for 1
    gpio_pad_select_gpio(GPIO_NUM_23); //R1
    gpio_pad_select_gpio(GPIO_NUM_4); //G1
    gpio_pad_select_gpio(GPIO_NUM_5); //B1
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    // RPG pin , RED ( 255,0,0) for 2
    gpio_pad_select_gpio(GPIO_NUM_12); //R1
    gpio_pad_select_gpio(GPIO_NUM_13); //G1
    gpio_pad_select_gpio(GPIO_NUM_14); //B1
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);

    // config timer 
    timer_config_t timer_cf = {};
    timer_cf.alarm_en=TIMER_ALARM_EN;
    timer_cf.counter_dir=TIMER_COUNT_UP;
    timer_cf.divider= 20;
    timer_cf.auto_reload=TIMER_AUTORELOAD_EN;
    timer_cf.counter_en =TIMER_PAUSE;
    timer_cf.intr_type =TIMER_INTR_LEVEL;

    timer_init(TIMER_GROUP_0,TIMER_1,&timer_cf);
    timer_set_counter_value(TIMER_GROUP_0,TIMER_1,0);
    timer_set_alarm_value(TIMER_GROUP_0,TIMER_1,20);
    
    // timer_group_intr_enable(TIMER_GROUP_0,TIMER_INTR_T0);
    timer_enable_intr(TIMER_GROUP_0,TIMER_1);
    timer_isr_register(TIMER_GROUP_0,TIMER_1,timer_group0_isr,NULL,ESP_INTR_FLAG_IRAM,NULL);
    timer_start(TIMER_GROUP_0, TIMER_1);

    

    for(int i = 0 ; i < 64 ; i ++){
        buffer[0][i]=102;
        buffer[1][i]=0;
        buffer[2][i]=255;
    }
    buffer[0][1]=0;
    // buffer[1][1]=0;
    // buffer[2][1]=0;
    while (1)
    {
        for(uint8_t i =0 ; i < 16; i ++){
            set_pixcel(102,0,255,i,0); 
        }


        
        
        

    }
    
}
