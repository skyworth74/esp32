#ifndef __MQTT_IO_CONTROL__
#define __MQTT_IO_CONTROL__
#include "mqtt_gpio_ctrl.h"
#define EMQX_CONFIG
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))

typedef enum{
MQTT_IO_IDLE,
MQTT_IO_REQUEST,
MQTT_IO_RESPONSE,
MQTT_IO_SUBSCRIBE,
MQTT_IO_UNSUBSCRIBE,
MQTT_IO_PUBLISH,
MQTT_IO_LASTWILL,
MQTT_IO_MAX
}mqtt_io_cmd;
esp_mqtt_client_handle_t get_mqtt_client_handle(void);
char *get_topic_pub_array_0(void);
char *get_topic_pub_array_1(void);
char *get_topic_pub_array_2(void);
char *get_topic_pub_array_3(void);
char *get_topic_pub_array_4(void);
char *get_topic_sub_array_0(void);
char *get_topic_sub_array_1(void);
char *get_topic_sub_array_2(void);
char *get_topic_sub_array_3(void);
char *get_topic_sub_array_4(void);




int serialmethod(char * dest ,char *src ,unsigned int len);
void process_json(const char *buff, unsigned int lens,const char *topic);

void system_init_setup(void);
void msg_send(const char *payload,const char *topic,mqtt_io_cmd ucMessageID,char lastwill,unsigned char qos, unsigned int lens,unsigned int parg);
int light_pwm_send(xt_rgb light_pwm);



#endif

