/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "mqtt_gpio_ctrl.h"
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_B          (5) // Define the output GPIO
#define LEDC_CHANNEL_B            LEDC_CHANNEL_0
#define LEDC_CHANNEL_R            LEDC_CHANNEL_1
#define LEDC_CHANNEL_G            LEDC_CHANNEL_2
#define LEDC_CHANNEL_COOL            LEDC_CHANNEL_3
#define LEDC_CHANNEL_WARM            LEDC_CHANNEL_4




#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_b_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_B,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_OUTPUT_IO_B,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ledc_channel_config_t ledc_r_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_R,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_OUTPUT_IO_R,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ledc_channel_config_t ledc_g_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_G,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_OUTPUT_IO_G,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ledc_channel_config_t ledc_cool_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_COOL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_OUTPUT_IO_COOL,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ledc_channel_config_t ledc_warm_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_WARM,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_OUTPUT_IO_WARM,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };		
    
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_r_channel));
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_g_channel));
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_b_channel));
	
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_cool_channel));
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_warm_channel));	
}



#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_R) | (1ULL<<GPIO_OUTPUT_IO_G)|(1ULL<<GPIO_OUTPUT_IO_B)| (1ULL<<GPIO_OUTPUT_IO_COOL)|(1ULL<<GPIO_OUTPUT_IO_WARM))
//#define GPIO_INPUT_IO_0     4
//#define GPIO_INPUT_IO_1     5
//#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
//#define ESP_INTR_FLAG_DEFAULT 0

void mqtt_gpio_init(void)
{
	//zero-initialize the config structure.
	   gpio_config_t io_conf = {};
	   //disable interrupt
	   io_conf.intr_type = GPIO_INTR_DISABLE;
	   //set as output mode
	   io_conf.mode = GPIO_MODE_OUTPUT;
	   //bit mask of the pins that you want to set,e.g.GPIO18/19
	   io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	   //disable pull-down mode
	   io_conf.pull_down_en = 0;
	   //disable pull-up mode
	   io_conf.pull_up_en = 0;
	   //configure GPIO with the given settings
	   gpio_config(&io_conf);
	#if 0
	   //interrupt of rising edge
	   io_conf.intr_type = GPIO_INTR_POSEDGE;
	   //bit mask of the pins, use GPIO4/5 here
	   io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
	   //set as input mode
	   io_conf.mode = GPIO_MODE_INPUT;
	   //enable pull-up mode
	   io_conf.pull_up_en = 1;
	   gpio_config(&io_conf);
	
	   //change gpio intrrupt type for one pin
	   gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);
	   #endif


}
void rgb_gpioctrl(int gpio_num,led_status status)
{
    if(LED_ON==status){
	    gpio_set_level(gpio_num, 1);//GPIO_OUTPUT_IO_0
    }else{
      gpio_set_level(gpio_num, 0);  
    }

}

void gpio_pwm(xt_rgb pwm)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R, pwm.rgb_r_pwm));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R));
	
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_G, pwm.rgb_g_pwm));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_G));	
	
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, pwm.rgb_b_pwm));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B));


	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_COOL, pwm.rgb_cool_pwm));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_COOL));
	
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_WARM, pwm.rgb_warm_pwm));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_WARM));

	

}

