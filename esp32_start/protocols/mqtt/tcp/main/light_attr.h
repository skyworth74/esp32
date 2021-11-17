#ifndef __LIGHT_ATTR__
#define __LIGHT_ATTR__
typedef struct light_attr_s{
 unsigned char light_start_flag;
 unsigned int state;
 unsigned int mode;
 unsigned int fading_time;
}light_attr;
typedef struct RGB{
int short rgb_r_pwm;
int short rgb_g_pwm;
int short rgb_b_pwm;
int short rgb_cool_pwm;
int short rgb_warm_pwm;
}xt_rgb;
typedef enum{
LIGHT_RED,
LIGHT_GREEN,
LIGHT_BLUE,
LIGHT_YELLOW,
LIGHT_PINK,
LIGHT_CYAN,//5
LIGHT_ROMATIC,
LIGHT_FOCUSE,
LIGHT_SUNSET,
LIGHT_MORNING,
LIGHT_WORK,//10
LIGHT_PARTY,
LIGHT_BLINK,
LIGHT_SELF_DEFINE,
LIGHT_CCT,//14



LIGHT_MAX
}light_mode_e;
typedef struct light_state_s{
light_mode_e light_mode;
unsigned char rgb[3];
unsigned int cct;
unsigned char light_effect_start_flag;
int gradient_time_ms;
unsigned char is_start;
unsigned char brightness;
}xt_light_state;

int light_state_send(xt_light_state light_state);
void light_multi_func(void *parg);
bool set_pwm(xt_rgb *current_pwm,xt_rgb *target_pwm,unsigned int *gradient_time,xt_light_state *light_state, unsigned int *time);

#endif

