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
#include "mqtt_io_control.h"

static const char *TAG = "MQTT_EXAMPLE";
#ifndef EMQX_CONFIG
char clientId[150];
char ali_username[64];
char ali_password[65];
#endif
esp_mqtt_client_handle_t mqtt_client;

esp_mqtt_client_handle_t get_mqtt_client_handle(void)
{
    return mqtt_client;
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
	char *topic_array;
    int msg_id =0;
    // your_context_t *context = event->context;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
			topic_array = get_topic_pub_array_0();
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
	       // msg_send("connect mqtt",topic_array,MQTT_IO_PUBLISH,0,1, strlen("connect mqtt"),0);
            msg_id = esp_mqtt_client_publish(client, topic_array, "connect mqtt", 0, 1, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
			topic_array = get_topic_sub_array_0();
             msg_send("light",topic_array,MQTT_IO_SUBSCRIBE,0,0, 0,0);
            //msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            //topic_array = get_topic_sub_array_1();
            //msg_send("window",topic_array,MQTT_IO_SUBSCRIBE,0,1,0,0);
            //msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            //msg_send("data_3","/topic/qos1",MQTT_IO_UNSUBSCRIBE,0,0,0,0);
            //msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            //ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
			
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
			//topic_array = get_topic_sub_array_0();
            //msg_send("light",topic_array,MQTT_IO_SUBSCRIBE,0,0, 0,0);
            //ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			// msg_send("data","/topic/qos0",MQTT_IO_PUBLISH,0,0, 0,0);
            //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
			char *receiver_topic=NULL;
			receiver_topic=(char *)malloc(event->topic_len+1);
			memset(receiver_topic,0,event->topic_len+1);
			strncpy(receiver_topic,event->topic,event->topic_len);
			//ESP_LOGI(TAG, "  topic:[%s]",receiver_topic);
			process_json(event->data, event->data_len,receiver_topic);
			free(receiver_topic);
			receiver_topic=NULL;
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

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    char uri[200];
	#ifdef EMQX_CONFIG
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,
       //.event_handle = mqtt_event_handler,
       // .cert_pem = (const char *)mqtt_eclipse_org_pem_start,
        .username ="hsy",//"emqx",
        .password = "123456",//public",//https://www.emqx.com/zh/blog/esp8266-connects-to-the-public-mqtt-broker
        .client_id= "emqx_cloud182439ec",//https://blog.csdn.net/chen244798611/article/details/97972236			
    };
	snprintf(uri,sizeof(uri),"%s:%s","mqtt://ucda1c7c.cn-shenzhen.emqx.cloud","11407");	
	#else
	memset(clientId,0,sizeof(clientId));
	memset(ali_username,0,sizeof(ali_username));
	memset(ali_password,0,sizeof(ali_password));
	aiotMqttSign("ggbj6g10Wgb", "aircon_123456", "217bcfabb473aeb1533c2fb4e0fadca1",
                     clientId, ali_username, ali_password);	
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        //.cert_pem = (const char *)mqtt_eclipse_org_pem_start,
        .username =ali_username,//"emqx",
        .password = ali_password,//public",//https://www.emqx.com/zh/blog/esp8266-connects-to-the-public-mqtt-broker
        .client_id= clientId,//https://blog.csdn.net/chen244798611/article/details/97972236
    };
	snprintf(uri,sizeof(uri),"%s:%s","mqtt://a1zHzM6aRR7.iot-as-mqtt.cn-shanghai.aliyuncs.com","1883");
	ESP_LOGI(TAG, "[TCP transport] Startup..uri:%s",uri);//"mqtt://broker-cn.emqx.io:1883");//CONFIG_EXAMPLE_BROKER_TCP_URI);

	#endif
#if 0//CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
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
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (NULL==mqtt_client){
         ESP_LOGI(TAG, "[APP] client==NULL..");
		 return;
    }
	esp_mqtt_client_set_uri(mqtt_client,uri);

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
	
	system_init_setup();
}
