#ifndef __MQTT_GPIO_CTRL__
#define __MQTT_GPIO_CTRL__
#include "light_attr.h"
#define GPIO_OUTPUT_IO_R    3
#define GPIO_OUTPUT_IO_G    4
#define GPIO_OUTPUT_IO_B    5
#define GPIO_OUTPUT_IO_COOL    19
#define GPIO_OUTPUT_IO_WARM   18
typedef enum{
LED_ON,
LED_OFF
}led_status;

void rgb_gpioctrl(int gpio_num,led_status status);
void mqtt_gpio_init(void);
void  gpio_pwm(xt_rgb pwm);
void example_ledc_init(void);
xt_rgb *get_pwm(void);
void light_pwm(void *parg);

#endif

