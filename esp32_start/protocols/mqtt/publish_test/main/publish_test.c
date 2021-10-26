/* MQTT publish test

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
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "xjson.h"
#include "xtimer.h"
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))

static const char *TAG = "PUBLISH_TEST";
xt_timer  *sg_pulse_timer=NULL;
#define EMQX_CONFIG

static EventGroupHandle_t mqtt_event_group;
const static int CONNECTED_BIT = BIT0;
const int SUBSCRIBE_BIT = BIT1;
const int UNSUBSCRIBE_BIT = BIT2;
const int PUBLISH_BIT     = BIT3;
#define  MQTT_EVENT_BIT CONNECTED_BIT|SUBSCRIBE_BIT|\
	                    UNSUBSCRIBE_BIT|PUBLISH_BIT

static esp_mqtt_client_handle_t mqtt_client = NULL;
int serialmethod(char * dest ,char *src ,unsigned int len);
extern int aiotMqttSign(const char *productKey, const char *deviceName, const char *deviceSecret,
                     char clientId[150], char username[64], char password[65]);
int light_cmd(const char *buf,unsigned int lens,unsigned int pconfig);
int window_cmd(const char *buf,unsigned int lens,unsigned int pconfig);
int door_cmd(const char *buf,unsigned int lens,unsigned int pconfig);
int sensor_cmd(const char *buf,unsigned int lens,unsigned int pconfig);



static int qos_test = 0;
typedef enum{
MQTT_UNSUB,
MQTT_SUB,
MQTT_PUB,
MQTT_OTHER,
MQTT_DEFAULT

};
typedef enum{
MQTT_REQUEST,
MQTT_RESPONSE,
MQTT_IDLE,
MQTT_MAX,
}mqtt_cmd;
 char *mqtt_state[5]={
"unsubscribe",
"subscribe"	,
"publish",
"other",
"default"
};
char *light_state[]={
"off",
"on"
};


typedef struct topic_cmd_t{
    char * topic;
	int (*func)(const char *buf,unsigned int lens,unsigned int pconfig);

}topic_cmd;
#ifdef EMQX_CONFIG

const char *topic_pub_array[]={
	"testtopic/pub/light/onoff/101",
	"testtopic/pub/window/onoff",	
	"testtopic/pub/door/onoff",
	"testtopic/pub/light/onoff/102",
	"testtopic/pub/lastwill",
};
const char *topic_sub_array[]={
	"testtopic/sub/light",	
	"testtopic/sub/window",
	"testtopic/sub/door",
	"testtopic/sub/sensor",
};	
const topic_cmd json_cmd[3]={
{"testtopic/sub/light",                    light_cmd},
{"testtopic/sub/window",                   window_cmd},
{"testtopic/sub/door",                     door_cmd},
{"testtopic/sub/sensor",                  sensor_cmd},
};	
#else
const char *topic_pub_array[]={
	"/shadow/get/ggbj6g10Wgb/aircon_123456",
	"/sys/ggbj6g10Wgb/aircon_123456/thing/config/get_reply",	
	"/sys/ggbj6g10Wgb/aircon_123456/thing/config/push",
	"/sys/ggbj6g10Wgb/aircon_123456/thing/config/push_reply",
	"/sys/ggbj6g10Wgb/aircon_123456/thing/deviceinfo/delete_reply"
};
const topic_cmd json_cmd[3]={
{"/shadow/get/ggbj6g10Wgb/aircon_123456",                    light_cmd},
{"/sys/ggbj6g10Wgb/aircon_123456/thing/config/get_reply",    window_cmd},
{"/sys/ggbj6g10Wgb/aircon_123456/thing/config/push_reply",   door_cmd},

};


#endif
typedef struct MQTT_MSG_T{
uint32_t mqtt_topic_len;
char *topic;
uint32_t payload_len;
char *payload;


}xt_mqtt_msg;
typedef struct AMessage_t
{
char ucMessageID;
char payload[ 100 ];
char topic[100];
char lastwill;
uint8_t  qos;

} AMessage;
AMessage xMessage;

QueueHandle_t xMqttQueue;



#if CONFIG_EXAMPLE_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipse_org_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_EXAMPLE_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipse_org_pem_start[]   asm("_binary_mqtt_eclipse_org_pem_start");
#endif
extern const uint8_t mqtt_eclipse_org_pem_end[]   asm("_binary_mqtt_eclipse_org_pem_end");

extern const uint8_t config_json_start[]   asm("_binary_config_json_start");

extern const uint8_t config_json_end[]   asm("_binary_config_json_end");


//char topic[256] = "testtopic/1";// "esp8266/test";
void public_setup(void);
void process_json(const char *buff, unsigned int lens,const char *topic);
void msg_send(const char *payload,const char *topic,mqtt_cmd ucMessageID,char lastwill,unsigned char qos, unsigned int lens,unsigned int parg)
{
    AMessage *pxMessage;
	if (NULL==payload){
        return;
	}
	if(NULL != xMqttQueue){
	// Send a pointer to a struct AMessage object.	Don't block if the
	// queue is already full.
		xMessage.ucMessageID =ucMessageID;//(xMessage.ucMessageID+1)%MQTT_MAX;
	    snprintf(xMessage.payload,sizeof(xMessage.payload),"%s",payload);
	    snprintf(xMessage.topic,sizeof(xMessage.topic),"%s",topic);
		xMessage.lastwill = lastwill;
		xMessage.qos =qos;
		pxMessage = & xMessage;
		//xQueueGenericSend( xMqttQueue, ( void * ) &pxMessage, ( TickType_t ) 0, queueSEND_TO_BACK );
		xQueueSend( xMqttQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
	}


}
int light_cmd(const char *buf,unsigned int lens,unsigned int pconfig){
	ESP_LOGI(TAG, "%s", __func__);
	unsigned char  json[512] = {0};
	xt_json root = NULL;
	char device_model[10]="";
	int device_state =0;

	//unsigned int i =0;
	//unsigned int index =0xff;
	if (NULL==buf){
		ESP_LOGI(TAG, "buff is null ");
        return -1;
	}
	 memset(device_model,0,sizeof(device_model));
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
	    ESP_LOGI(TAG, " model [%s] device_state:[%d]", device_model,device_state);
	exit:
	XjsonDelete(root);
    return 0;
}
int window_cmd(const char *buf,unsigned int lens,unsigned int pconfig){
	ESP_LOGI(TAG, "%s", __func__);
	unsigned char  json[512] = {0};
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
	mqtt_cmd ucMessageID;
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
	ucMessageID = (mqtt_cmd)cmd;
	if (topic_index>ARRAY_SIZE(topic_pub_array)){
    goto exit;
	}
    msg_send(room_pos,topic_pub_array[topic_index],ucMessageID,0,0,strlen(room_pos),0);

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

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    static int msg_id = 0;
    static int actual_len = 0;
	static xt_mqtt_msg mqtt_msg;
    // your_context_t *context = event->context;
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
        //msg_id = esp_mqtt_client_subscribe(client, CONFIG_EXAMPLE_SUBSCIBE_TOPIC, qos_test);
        //memset(topic ,0,sizeof(topic));
		//snprintf(topic,sizeof(topic),"%s","testtopic/2/light");
		msg_id = esp_mqtt_client_subscribe(client, topic_sub_array[0], qos_test);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
		xEventGroupSetBits(mqtt_event_group, SUBSCRIBE_BIT);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		xEventGroupSetBits(mqtt_event_group, UNSUBSCRIBE_BIT);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		xEventGroupSetBits(mqtt_event_group, PUBLISH_BIT);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        printf("ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n", event->msg_id, event->total_data_len, event->data_len, event->current_data_offset);
        if (event->topic) {
            actual_len = event->data_len;
            msg_id = event->msg_id;
        }else {
            actual_len += event->data_len;
            // check consisency with msg_id across multiple data events for single msg
            if (msg_id != event->msg_id) {
                ESP_LOGI(TAG, "Wrong msg_id in chunked message %d != %d", msg_id, event->msg_id);
                abort();
            }
        }
		char *receiver_topic;
		char * dest;
		receiver_topic =(char *)malloc(event->topic_len+1);
		dest =(char *)malloc(event->topic_len);
		memset(receiver_topic, 0, event->topic_len+1);
		memset(dest, 0, event->topic_len);
		strncpy(receiver_topic,event->topic,event->topic_len);
		//serialmethod(dest ,receiver_topic ,event->topic_len);
		process_json(event->data, event->data_len,receiver_topic);
		free(receiver_topic);
		free(dest);
		mqtt_msg.mqtt_topic_len = event->topic_len;
		mqtt_msg.topic = event->topic;
		mqtt_msg.payload_len = event->data_len;
		mqtt_msg.payload = event->data;

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

char clientId[150];
char ali_username[64];
char ali_password[65];

static void mqtt_app_start(void)
{

    mqtt_event_group = xEventGroupCreate();

    #ifdef EMQX_CONFIG                 
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        .cert_pem = (const char *)mqtt_eclipse_org_pem_start,
        .username ="hsy",//"emqx",
        .password = "123456",//public",//https://www.emqx.com/zh/blog/esp8266-connects-to-the-public-mqtt-broker
        .client_id= "emqx_cloud182439ec",//https://blog.csdn.net/chen244798611/article/details/97972236
    };
	#else
	memset(clientId,0,sizeof(clientId));
	memset(ali_username,0,sizeof(ali_username));
	memset(ali_password,0,sizeof(ali_password));
	aiotMqttSign("ggbj6g10Wgb", "aircon_123456", "217bcfabb473aeb1533c2fb4e0fadca1",
                     clientId, ali_username, ali_password);	
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        .cert_pem = (const char *)mqtt_eclipse_org_pem_start,
        .username =ali_username,//"emqx",
        .password = ali_password,//public",//https://www.emqx.com/zh/blog/esp8266-connects-to-the-public-mqtt-broker
        .client_id= clientId,//https://blog.csdn.net/chen244798611/article/details/97972236
    };

    /*memcpy(mqtt_cfg.username,ali_username,strlen(ali_username));
	memcpy(mqtt_cfg.password,ali_password,strlen(ali_password));
	memcpy(mqtt_cfg.client_id,clientId,strlen(clientId));*/
	//mqtt_cfg.username=ali_username;
	//mqtt_cfg.password=ali_password;
	//mqtt_cfg.client_id=clientId;
	ESP_LOGI(TAG, "username:%s password:%s client_id:%s ", ali_username,ali_password,clientId);
	#endif

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
}

static void get_string(char *line, size_t size)
{

    int count = 0;
    while (count < size) {
        int c = fgetc(stdin);
        if (c == '\n') {
            line[count] = '\0';
            break;
        } else if (c > 0 && c < 127) {
            line[count] = c;
            ++count;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    char line[256];
    char pattern[32];
    char transport[32];
    int repeat = 0;
	char uri[200];
	char flag=0;
    char *json_buf=config_json_start;
	AMessage *pxMessage;
	esp_err_t err ;
	nvs_handle_t my_handle;
	int restart_counter =0;
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	memset(uri,0,sizeof(uri));
	

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
	serialmethod(uri ,json_buf ,strlen(json_buf));
	ESP_LOGI(TAG, "json:%s\n",uri);
    memset(uri,0,sizeof(uri));
    mqtt_app_start();
    public_setup();
	#ifdef EMQX_CONFIG  	
			snprintf(uri,sizeof(uri),"%s:%s","mqtt://ucda1c7c.cn-shenzhen.emqx.cloud","11407");
            ESP_LOGI(TAG, "[TCP transport] Startup..uri:%s",uri);//"mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
            esp_mqtt_client_set_uri(mqtt_client,uri);// "mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
    #else
		    snprintf(uri,sizeof(uri),"%s:%s","mqtt://a1zHzM6aRR7.iot-as-mqtt.cn-shanghai.aliyuncs.com","1883");
		    ESP_LOGI(TAG, "[TCP transport] Startup..uri:%s",uri);//"mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
		    esp_mqtt_client_set_uri(mqtt_client,uri);// "mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
	 #endif
	 esp_mqtt_client_start(mqtt_client);
	#if 0
    while (1) {
		if (0==flag){
        get_string(line, sizeof(line));
        //sscanf(line, "%s %s %d %d %d", transport, pattern, &repeat, &expected_published, &qos_test);
		sscanf(line, "%s", transport);
        //ESP_LOGI(TAG, "PATTERN:%s REPEATED:%d PUBLISHED:%d\n", pattern, repeat, expected_published);
       /* int pattern_size = strlen(pattern);
        free(expected_data);
        free(actual_data);
        actual_published = 0;
        expected_size = pattern_size * repeat;
        expected_data = malloc(expected_size);
        actual_data = malloc(expected_size);
        for (int i = 0; i < repeat; i++) {
            memcpy(expected_data + i * pattern_size, pattern, pattern_size);
        }
        printf("EXPECTED STRING %.*s, SIZE:%d\n", expected_size, expected_data, expected_size);*/
        esp_mqtt_client_stop(mqtt_client);

        if (0 == strcmp(transport, "tcp")) {
		 #ifdef EMQX_CONFIG  	
			snprintf(uri,sizeof(uri),"%s:%s","mqtt://ucda1c7c.cn-shenzhen.emqx.cloud","11407");
            ESP_LOGI(TAG, "[TCP transport] Startup..uri:%s",uri);//"mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
            esp_mqtt_client_set_uri(mqtt_client,uri);// "mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
         #else
		    snprintf(uri,sizeof(uri),"%s:%s","mqtt://a1zHzM6aRR7.iot-as-mqtt.cn-shanghai.aliyuncs.com","1883");
		    ESP_LOGI(TAG, "[TCP transport] Startup..uri:%s",uri);//"mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
		    esp_mqtt_client_set_uri(mqtt_client,uri);// "mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);
		 #endif
        } else if (0 == strcmp(transport, "ssl")) {
            ESP_LOGI(TAG, "[SSL transport] Startup..");
            esp_mqtt_client_set_uri(mqtt_client, CONFIG_EXAMPLE_BROKER_SSL_URI);
        } else if (0 == strcmp(transport, "ws")) {
            ESP_LOGI(TAG, "[WS transport] Startup..");
            esp_mqtt_client_set_uri(mqtt_client, CONFIG_EXAMPLE_BROKER_WS_URI);
        } else if (0 == strcmp(transport, "wss")) {
            ESP_LOGI(TAG, "[WSS transport] Startup..");
            esp_mqtt_client_set_uri(mqtt_client, CONFIG_EXAMPLE_BROKER_WSS_URI);
        } else {
            ESP_LOGE(TAG, "Unexpected transport");
            abort();
        }
        xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT|UNSUBSCRIBE_BIT|SUBSCRIBE_BIT|PUBLISH_BIT);
        esp_mqtt_client_start(mqtt_client);
        ESP_LOGI(TAG, "Note free memory: %d bytes", esp_get_free_heap_size());
		flag = 1;
		}{
			memset(line,0,sizeof(line));
			memset(transport,0,sizeof(transport));
			memset(pattern,0,sizeof(pattern));
			memset(transport,0,sizeof(transport));
			flag = 0;
            vTaskDelay(1000);
		}
    }
	#endif
	xMqttQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
	
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
	err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
	switch (err) {
	case ESP_OK:
	printf("Done\n");	
	printf("Restart counter = %d\n", restart_counter);
	break;
	case ESP_ERR_NVS_NOT_FOUND:
	printf("The value is not initialized yet!\n");
	nvs_set_i32(my_handle,"restart_counter",restart_counter++);
	break;
	default :
	printf("Error (%s) reading!\n", esp_err_to_name(err));
	}
	restart_counter++;
	nvs_set_i32(my_handle,"restart_counter",restart_counter);
    err = nvs_commit(my_handle);
	printf((err==ESP_OK)?"success\r\n":"failed\r\n");
	nvs_close(my_handle);

	while(1){
        vTaskDelay(10000);

	}
	vTaskDelete(NULL);
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

void process_json(const char *buff, unsigned int lens,const char *topic) {
    unsigned char index =0;
	unsigned char i =0;
	for (i=0;i<ARRAY_SIZE(json_cmd);i++){//sizeof(topic_array)/sizeof(topic_array[0])//
		//ESP_LOGI(TAG, "  topic:[%s] %s",topic_array[i],topic);
        if(0==strcmp(topic,json_cmd[i].topic)){
            index= i;
			ESP_LOGI(TAG, " find [%d] topic:[%s]",index,json_cmd[index].topic);
		    json_cmd[index].func(buff,lens,0);
			break;
        }
	}

	return;

}

void public_thread(void *parg)
{
    //parg=NULL;
	int arg=*(int*)parg;
	unsigned char state =0;
	int qos = 0;
	int msg_id=0;
	EventBits_t result =0;
	char payload[256];
	cJSON *root = NULL;
	cJSON *fmt = NULL;
   /* cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;*/
	char *json_buf= NULL;
	unsigned char index = 0;
    while(1){
		if (NULL==mqtt_event_group){
            vTaskDelay(arg);
		    continue;
		}
		result = xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT|UNSUBSCRIBE_BIT|SUBSCRIBE_BIT|PUBLISH_BIT, true, false, portMAX_DELAY);
		if (result){
            ESP_LOGI(TAG, " result bit [0x%x]", result);
		}
		//xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT|UNSUBSCRIBE_BIT|SUBSCRIBE_BIT|PUBLISH_BIT);
		switch(state){
        case MQTT_UNSUB:
			memset(payload ,0,sizeof(payload));
			snprintf(payload,sizeof(payload),"%s","say goodbyte");
			esp_mqtt_client_unsubscribe(mqtt_client,json_cmd[1].topic);
		    msg_id = esp_mqtt_client_publish(mqtt_client, topic_pub_array[4], payload, strlen(payload), qos_test, 1);//lastwill
		    ESP_LOGI(TAG, " state [%s]", mqtt_state[state]);
		    xEventGroupSetBits(mqtt_event_group, UNSUBSCRIBE_BIT);
		    state =MQTT_SUB;
			break;
		case MQTT_SUB:
			esp_mqtt_client_subscribe(mqtt_client,topic_sub_array[2],qos);
			//memset(topic ,0,sizeof(topic));
			//snprintf(topic,sizeof(topic),"%s/%s","testtopic/1","window");
			esp_mqtt_client_subscribe(mqtt_client,topic_sub_array[1],qos);
		    esp_mqtt_client_subscribe(mqtt_client,topic_sub_array[3],qos);
			xEventGroupSetBits(mqtt_event_group, SUBSCRIBE_BIT);
		    ESP_LOGI(TAG, " state [%s]", mqtt_state[state]);
			state =MQTT_PUB;		    
			break;
		case MQTT_PUB:
			xEventGroupSetBits(mqtt_event_group, PUBLISH_BIT);
			//memset(topic ,0,sizeof(topic));
		    memset(payload ,0,sizeof(payload));
		    //snprintf(topic,sizeof(topic),"%s/%s","testtopic/1","door");
		    root = XjsonCreateObject();
			 XjsonSetString(root, "onoff", "true");
			json_buf=XjsonToStringformatted(root);
			serialmethod(payload ,json_buf ,strlen(json_buf));
			cJSON_Delete(root);
			free(json_buf);
			json_buf=NULL;
			root=NULL;	
			//snprintf(payload,sizeof(payload),"%s","true");
			msg_id = esp_mqtt_client_publish(mqtt_client, topic_pub_array[2], payload, strlen(payload), qos_test, 0);
            ESP_LOGI(TAG, "[%d] Publishing...%s", msg_id,topic_pub_array[2]);
			ESP_LOGI(TAG, " state [%s]", mqtt_state[state]);
			state =MQTT_OTHER;
			break;
		case MQTT_OTHER:
			xEventGroupSetBits(mqtt_event_group, PUBLISH_BIT);
			//memset(topic ,0,sizeof(topic));
		    memset(payload ,0,sizeof(payload));
			//memset(light_state ,0,sizeof(light_state));
			index=(index+1)%100;
			#if 1
			root = XjsonCreateObject();
			if (NULL==root){
                break;
			}
			fmt = XjsonCreateObject();
			if(NULL==fmt){
                break;
			}
			cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
			
            cJSON_AddItemToObject(root, "format", fmt);
            XjsonSetString(fmt, "type", "rect");
            XjsonSetInt(fmt, "msgid", index);
            XjsonSetInt(fmt, "memory", esp_get_free_heap_size());
            cJSON_AddFalseToObject (fmt, "interlace");
            XjsonSetInt(fmt, "frame rate", 60);
            json_buf=XjsonToStringformatted(root);
			serialmethod(payload ,json_buf ,strlen(json_buf));
			cJSON_Delete(root);
			free(json_buf);
			json_buf=NULL;
			root=NULL;		

			#endif
			//snprintf(topic,sizeof(topic),"%s/%s","testtopic/201","light");
			msg_id = esp_mqtt_client_publish(mqtt_client, topic_pub_array[3], payload, strlen(payload), qos_test, 0);
            ESP_LOGI(TAG, "[%d] Publishing...%s", msg_id,topic_pub_array[3]);
			ESP_LOGI(TAG, " state [%s]", mqtt_state[state]);
			state =MQTT_DEFAULT;

			break;
		default:
		    state =MQTT_SUB;
			vTaskDelay(1000*10);//10s
			xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
		    ESP_LOGI(TAG, " state [%s]", "default");
		

		}
		//xTaskGetTickCount(  );
        if (XOK==XtimerMsleep(&sg_pulse_timer,1000*10)){
             ESP_LOGI(TAG, " 20 seconds tick:%d",xTaskGetTickCount(  ));
			 ESP_LOGI(TAG, "Note free memory: %d bytes", esp_get_free_heap_size());

        }
        /*for (int i = 0; i < expected_published; i++) {			
            //int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_EXAMPLE_PUBLISH_TOPIC, expected_data, expected_size, qos_test, 0);
            int msg_id = esp_mqtt_client_publish(mqtt_client, topic, expected_data, expected_size, qos_test, 0);
            ESP_LOGI(TAG, "[%d] Publishing...", msg_id);
        }*/

    }
}
void mqtt_process(void *parg)
{
    void *unuse=parg;
	mqtt_cmd mqtt_cmd_e=MQTT_IDLE;
	int qos =0;
	int msg_id =0;
	char payload[256]="";
	char pubtopic[256]="";
	AMessage *pxRxedMessage;
	while(1){
        //vTaskDelay(10000);
		ESP_LOGI(TAG, "%s", __func__);
	    

       if( xMqttQueue != 0 )
       {
       // Receive a message on the created queue.  Block for 10 ticks if a
       // message is not immediately available.
           if( xQueueReceive( xMqttQueue, &( pxRxedMessage ), ( TickType_t ) portMAX_DELAY ) )
           {
           // pcRxedMessage now points to the struct AMessage variable posted
           // by vATask.
          }
       }
	   mqtt_cmd_e = pxRxedMessage->ucMessageID;
	    switch(mqtt_cmd_e){
        case MQTT_REQUEST:
			esp_mqtt_client_subscribe(mqtt_client,topic_sub_array[2],qos);
		    //mqtt_cmd_e=MQTT_RESPONSE;
			break;
		case MQTT_RESPONSE:
			memset(payload,0,sizeof(payload));
			memset(pubtopic,0,sizeof(pubtopic));
			snprintf(payload,sizeof(payload),"%s",pxRxedMessage->payload);
			snprintf(pubtopic,sizeof(pubtopic),"%s",pxRxedMessage->topic);
			msg_id = esp_mqtt_client_publish(mqtt_client, pubtopic, payload, strlen(payload), qos_test, 0);
			break;
		case MQTT_IDLE:
			break;
		default:
		;

	    }

	}

}
void public_setup(void)
{
    TaskHandle_t taskHandle;
	int delaytime =100;
	XtimeInit();
    xTaskCreate(public_thread,"public_thread",1024*4,&delaytime,5,&taskHandle);
	xTaskCreate(mqtt_process,"mqtt_process",1024*4,&delaytime,6,NULL);
}
