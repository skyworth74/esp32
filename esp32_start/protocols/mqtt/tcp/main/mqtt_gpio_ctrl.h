#ifndef __MQTT_GPIO_CTRL__
#define __MQTT_GPIO_CTRL__

#define GPIO_OUTPUT_IO_R    3
#define GPIO_OUTPUT_IO_G    4
#define GPIO_OUTPUT_IO_B    5
#define GPIO_OUTPUT_IO_COOL    19
#define GPIO_OUTPUT_IO_WARM   18
typedef enum{
LED_ON,
LED_OFF
}led_status;
typedef struct RGB{
int short rgb_r_pwm;
int short rgb_g_pwm;
int short rgb_b_pwm;
int short rgb_cool_pwm;
int short rgb_warm_pwm;
}xt_rgb;
void rgb_gpioctrl(int gpio_num,led_status status);
void mqtt_gpio_init(void);
void  gpio_pwm(xt_rgb pwm);
void example_ledc_init(void);
#endif

