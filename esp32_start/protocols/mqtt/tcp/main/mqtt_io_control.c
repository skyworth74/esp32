/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "xtimer.h"
#include "mqtt_io_control.h"
#include "xjson.h"
#include "xtimer.h"
#include "mqtt_gpio_ctrl.h"
#include "light_attr.h"

QueueHandle_t xMqttQueue;
static const char *TAG = "MQTT_IO_CONTROL";
int light_cmd(const char *buf,unsigned int lens,unsigned int pconfig);
int window_cmd(const char *buf,unsigned int lens,unsigned int pconfig);
int door_cmd(const char *buf,unsigned int lens,unsigned int pconfig);
int sensor_cmd(const char *buf,unsigned int lens,unsigned int pconfig);



typedef struct AMessage_t
{
char ucMessageID;
char payload[ 100 ];
char topic[100];
char lastwill;
uint8_t  qos;
unsigned int lens;

} AMessage;
AMessage xMessage;
typedef struct topic_cmd_t{
    char * topic;
	int (*func)(const char *buf,unsigned int lens,unsigned int pconfig);

}topic_cmd;

#ifdef EMQX_CONFIG
const topic_cmd json_sub_cmd[]={
{"testtopic/sub/light",                    light_cmd},
{"testtopic/sub/window",                   window_cmd},
{"testtopic/sub/door",                     door_cmd},
{"testtopic/sub/sensor",                  sensor_cmd},
};
const topic_cmd json_pub_cmd[]={
	{"testtopic/pub/light/onoff/101",NULL},
	{"testtopic/pub/window/onoff",	NULL},
	{"testtopic/pub/door/onoff",NULL},
	{"testtopic/pub/light/onoff/102",NULL},
	{"testtopic/pub/lastwill",NULL},


};
#endif

char *get_topic_pub_array_0(void)
{
return json_pub_cmd[0].topic;
}

char *get_topic_pub_array_1(void)
{
return json_pub_cmd[1].topic;
}
char *get_topic_pub_array_2(void)
{
return json_pub_cmd[2].topic;
}
char *get_topic_pub_array_3(void)
{
return json_pub_cmd[3].topic;
}
char *get_topic_pub_array_4(void)
{
return json_pub_cmd[4].topic;
}

char *get_topic_sub_array_0(void)
{
return json_sub_cmd[0].topic;
}

char *get_topic_sub_array_1(void)
{
return json_sub_cmd[1].topic;
}
char *get_topic_sub_array_2(void)
{
return json_sub_cmd[2].topic;
}
char *get_topic_sub_array_3(void)
{
return json_sub_cmd[3].topic;
}
char *get_topic_sub_array_4(void)
{
return json_sub_cmd[4].topic;
}




int light_cmd(const char *buf,unsigned int lens,unsigned int pconfig){
	ESP_LOGI(TAG, "%s", __func__);
	//unsigned char  json[512] = {0};
	xt_json root = NULL;
	char device_model[10]="";
	int device_state =0;
    int color  = 0;
	int light_mode  = 0;
	//int pwm_b  = 0;
	int cool = 0;
	int warm = 0;
	int cct = 2700;
	int gradient_time_ms =0;
	int brigheness =0;
	xt_light_state light_state; //= get_pwm();
	//unsigned int i =0;
	//unsigned int index =0xff;
	if (NULL==buf){
		ESP_LOGI(TAG, "buff is null ");
        return -1;
	}
	 memset(device_model,0,sizeof(device_model));
	 memset(&light_state,0,sizeof(xt_light_state));
	 //memset(name,0,sizeof(name));
	
	root=XjsonFromString(buf);
	if(NULL == root){
		ESP_LOGI(TAG, "root is null ");
          goto exit;
    }
	XjsonGetString(root,
                      "model",
                      device_model,
                      sizeof(device_model),
                      "dim");
    XjsonGetInt(root, "state", &device_state, 0);
	XjsonGetInt(root, "color", &color, 0);
	XjsonGetInt(root, "light_mode", &light_mode, 0);
	XjsonGetInt(root, "gradient_time_ms", &gradient_time_ms, 0);
	XjsonGetInt(root, "brightness", &brigheness, 0);
	//XjsonGetInt(root, "warm", &warm, 0);
	XjsonGetInt(root, "cct", &cct, 2700);
	ESP_LOGI(TAG, " model [%s] device_state:[%d]", device_model,device_state);

	if(1==device_state){
	    //rgb_gpioctrl(GPIO_OUTPUT_IO_R,LED_ON);
		//rgb_gpioctrl(GPIO_OUTPUT_IO_G,LED_ON);
	   // rgb_gpioctrl(GPIO_OUTPUT_IO_B,LED_ON);
		//rgb_gpioctrl(GPIO_OUTPUT_IO_WARM,LED_ON);
		//gpio_pwm(rgb_pwm);
	    light_state.rgb[0] = (color&0xFF0000)>>16;
	    light_state.rgb[1] = (color&0xFF00)>>8;
	    light_state.rgb[2] = color&0xFF;
		light_state.light_mode = (light_mode_e)light_mode;
        light_state.cct=cct;
		light_state.gradient_time_ms =gradient_time_ms;
		light_state.brightness = (unsigned char)(brigheness&0xff);
		light_state_send(light_state);
	}else{
       // rgb_gpioctrl(GPIO_OUTPUT_IO_R,LED_OFF);
		//rgb_gpioctrl(GPIO_OUTPUT_IO_G,LED_OFF);
		;//rgb_gpioctrl(GPIO_OUTPUT_IO_B,LED_OFF);
      //gpio_pwm(rgb_pwm);
	   // memset(&rgb_pwm,0,sizeof(xt_rgb));
	}
	exit:
	XjsonDelete(root);
    return 0;
}
int window_cmd(const char *buf,unsigned int lens,unsigned int pconfig){
	ESP_LOGI(TAG, "%s", __func__);
	//unsigned char  json[512] = {0};
	xt_json root = NULL;
	
	char name[32]="";
	char type[10]="";
	
	int width =0;
	//unsigned int i =0;
	//unsigned int index =0xff;
	if (NULL==buf){
		ESP_LOGI(TAG, "buff is null ");
        return -1;
	}
	 //memset(device_model,0,sizeof(device_model));
	 memset(name,0,sizeof(name));
	 memset(type,0,sizeof(type));
	root=XjsonFromString(buf);
	if(NULL == root){
		ESP_LOGI(TAG, "root is null ");
          goto exit;
    }
	XjsonGetString(root,"name",name,sizeof(name),"jack");
	XjsonGetString(root,"format.type",type,sizeof(type),"jack");	    
    XjsonGetInt(root,"format.width",&width,0);		
	ESP_LOGI(TAG, " name [%s] type:%s width:%d", name,type,width);
    exit:
    XjsonDelete(root);
    return 0;
}
int door_cmd(const char *buf,unsigned int lens,unsigned int pconfig){
	ESP_LOGI(TAG, "%s", __func__);
	xt_json root = NULL;
	int device_state = 0;
	char room_pos[33]="";
	int topic_index =0;
	int cmd;
	mqtt_io_cmd ucMessageID;
	if (NULL==buf){
        return -1;
	}

	root=XjsonFromString(buf);
	if(NULL == root){
		ESP_LOGI(TAG, "root is null ");
          goto exit;
    }
	XjsonGetString(root,
                      "room",
                      room_pos,
                      sizeof(room_pos),
                      "rose");
    XjsonGetInt(root, "state", &device_state, 0);
	XjsonGetInt(root, "ucMessageID", &cmd, 0);
	XjsonGetInt(root, "topic_index", &topic_index, 0);
	    ESP_LOGI(TAG, " room_pos [%s] device_state:[%d]", room_pos,device_state);
	ucMessageID = (mqtt_io_cmd)cmd;
	if (topic_index>ARRAY_SIZE(json_pub_cmd)){
    goto exit;
	}
    msg_send(room_pos,json_pub_cmd[topic_index].topic,ucMessageID,0,0,strlen(room_pos),0);

    exit:
	XjsonDelete(root);
    return 0;
}

int sensor_cmd(const char *buf,unsigned int lens,unsigned int pconfig){
	ESP_LOGI(TAG, "%s", __func__);
	xt_json root = NULL;
	if (NULL==buf){
        return -1;
	}

	root=XjsonFromString(buf);
	if(NULL == root){
		ESP_LOGI(TAG, "root is null ");
          goto exit;
    }

    exit:
	XjsonDelete(root);
    return 0;
}

int serialmethod(char * dest ,char *src ,unsigned int len)
{
    if (NULL==dest||NULL==src){
        return -1;
    }
    while(len){
		if (0x0d==*src||0x0a==*src||0x09==*src){
			src++;
		}else{
			*dest=*src;
			dest++;
			src++;
		}
        len--;
	}		
	return 0;
}


void msg_send(const char *payload,const char *topic,mqtt_io_cmd ucMessageID,char lastwill,unsigned char qos, unsigned int lens,unsigned int parg)
{
    AMessage *pxMessage;
	if (NULL==payload){
        return;
	}
	ESP_LOGI(TAG, "msg_send %s %s",topic, payload );
	if(NULL != xMqttQueue){
	// Send a pointer to a struct AMessage object.	Don't block if the
	// queue is already full.
	    memset(&xMessage,0,sizeof(AMessage));
		xMessage.ucMessageID =ucMessageID;//(xMessage.ucMessageID+1)%MQTT_MAX;
	    snprintf(xMessage.payload,sizeof(xMessage.payload),"%s",payload);
	    snprintf(xMessage.topic,sizeof(xMessage.topic),"%s",topic);
		xMessage.lastwill = lastwill;
		xMessage.qos =qos;
		xMessage.lens = lens;
		pxMessage = & xMessage;
		//xQueueGenericSend( xMqttQueue, ( void * ) &pxMessage, ( TickType_t ) 0, queueSEND_TO_BACK );
		xQueueSend( xMqttQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
	}


}

void process_json(const char *buff, unsigned int lens,const char *topic) {
    unsigned char index =0;
	unsigned char i =0;
	for (i=0;i<ARRAY_SIZE(json_sub_cmd);i++){//sizeof(topic_array)/sizeof(topic_array[0])//
		//ESP_LOGI(TAG, "  topic:[%s]",topic);
        if(0==strcmp(topic,json_sub_cmd[i].topic)){
            index= i;
			ESP_LOGI(TAG, " find [%d] topic:[%s]",index,json_sub_cmd[index].topic);
		    json_sub_cmd[index].func(buff,lens,0);
			break;
        }else{
			ESP_LOGI(TAG, " coould notfind [%d] topic:[%s]",i,json_sub_cmd[i].topic);

        	}
	}

	return;

}

void mqtt_io_ctrl_thread(void *parg)
{
    AMessage *pxRxedMessage;
	mqtt_io_cmd io_cmd;
	esp_mqtt_client_handle_t client =NULL;// get_mqtt_client_handle();
	int msg_id = 0;
    while(1){
		
       if( NULL ==xMqttQueue )
       {
           vTaskDelay(10000);
		   continue;
       }
	   client = get_mqtt_client_handle();
       // Receive a message on the created queue.  Block for 10 ticks if a
       // message is not immediately available.
       if( xQueueReceive( xMqttQueue, &( pxRxedMessage ), ( TickType_t ) portMAX_DELAY ) )
       {
           // pcRxedMessage now points to the struct AMessage variable posted
           // by vATask.
       }
	   io_cmd=(mqtt_io_cmd)pxRxedMessage->ucMessageID;
	   switch(io_cmd)
	   {
       case MQTT_IO_IDLE:
	   	break;
	   case MQTT_IO_REQUEST:
	   	break;
	   case MQTT_IO_RESPONSE:
	   	break;
	   case MQTT_IO_SUBSCRIBE:
       msg_id = esp_mqtt_client_subscribe(client,pxRxedMessage->topic, 0);
	   ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d topic:%s", msg_id,pxRxedMessage->topic);
		break;
	   case MQTT_IO_UNSUBSCRIBE:
	   	esp_mqtt_client_unsubscribe(client, pxRxedMessage->topic);
	   	break;
	   case MQTT_IO_PUBLISH:
	   msg_id = esp_mqtt_client_publish(client, pxRxedMessage->topic, pxRxedMessage->payload, strlen(pxRxedMessage->payload), pxRxedMessage->qos, pxRxedMessage->lastwill);
       ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
	   break;
	   case MQTT_IO_LASTWILL:
	   	break;
	   default:
	   ESP_LOGI(TAG, "%s io_cmd error",__func__);

	   }
      
 
    }

}
void system_init_setup(void)
{
    TaskHandle_t taskHandle;
	int delaytime =100;
	XtimeInit();
	//mqtt_gpio_init();
	example_ledc_init();
	xMqttQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
	if (NULL!=xMqttQueue){
        xTaskCreate(mqtt_io_ctrl_thread,"mqtt_io_ctrl_thread",1024*4,&delaytime,5,&taskHandle);
	}
	xTaskCreate(light_pwm,"light_pwm",512+128,&delaytime,6,NULL);
	xTaskCreate(light_multi_func,"light_muiti_func",2048+512,NULL,6,NULL);
}

