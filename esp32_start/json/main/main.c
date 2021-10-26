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
static const char *TAG = "JSON";
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
	*dest='\0';
	return 0;
}

void test_json(void ) {
    unsigned char  json[512] = {0};
	int delay_time =1000;
   
    cJSON *root = cJSON_CreateObject();
    cJSON *sensors = cJSON_CreateArray();
    cJSON *id1 = cJSON_CreateObject();
    cJSON *id2 = cJSON_CreateObject();
    cJSON *iNumber = cJSON_CreateNumber(10);

    cJSON_AddItemToObject(id1, "id", cJSON_CreateString("1"));
    cJSON_AddItemToObject(id1, "temperature1", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id1, "temperature2", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id1, "humidity", cJSON_CreateString("55"));
    cJSON_AddItemToObject(id1, "occupancy", cJSON_CreateString("1"));
    cJSON_AddItemToObject(id1, "illumination", cJSON_CreateString("23"));

    cJSON_AddItemToObject(id2, "id", cJSON_CreateString("2"));
    cJSON_AddItemToObject(id2, "temperature1", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id2, "temperature2", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id2, "humidity", cJSON_CreateString("55"));
    cJSON_AddItemToObject(id2, "occupancy", cJSON_CreateString("1"));
    cJSON_AddItemToObject(id2, "illumination", cJSON_CreateString("23"));

    cJSON_AddItemToObject(id2, "value", iNumber);

    cJSON_AddItemToArray(sensors, id1);
    cJSON_AddItemToArray(sensors, id2);

    cJSON_AddItemToObject(root, "sensors", sensors);
    char *str = cJSON_Print(root);

    uint32_t jslen = strlen(str);
    memcpy(json, str, jslen);
    printf("%s\n", json);

    cJSON_Delete(root);
    free(str);
    str = NULL;
	vTaskDelay(delay_time);
	printf("heap size:%d\n", esp_get_free_heap_size());
    
}

void public_thread(void *parg)
{
	char payload[256];
	cJSON *root = NULL;
	cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
	char *json_buf= NULL;
	while(1){
	     vTaskDelay(10);
		 			root = XjsonCreateObject();
			if (NULL==root){
                break;
			}
			memset(payload,0,sizeof(payload));
			fmt = XjsonCreateObject();			
			cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
            cJSON_AddItemToObject(root, "format", fmt);
            XjsonSetString(fmt, "type", "rect");
            XjsonSetInt(fmt, "width", 1920);
            XjsonSetInt(fmt, "height", 1080);
            cJSON_AddFalseToObject (fmt, "interlace");
            XjsonSetInt(fmt, "frame rate", 60);
            json_buf=XjsonToStringformatted(root);
			serialmethod(payload ,json_buf ,strlen(json_buf));
			ESP_LOGI(TAG, "[APP] IDF version: %s", payload);
			cJSON_Delete(root);
			free(json_buf);
			json_buf=NULL;
			root=NULL;
	}
}
void public_setup(void)
{
    TaskHandle_t taskHandle;
	int delaytime =100;
    xTaskCreate(public_thread,"public_thread",1024*4,&delaytime,5,&taskHandle);
	//xTaskCreate(test_json,"packageJson",1024*2,&delaytime,5,NULL);
}

void app_main(void)
{
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
	

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
	public_setup();
	while(1){
		test_json( );
		vTaskDelay(10);
		
	}
	
}