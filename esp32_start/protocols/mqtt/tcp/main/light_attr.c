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
#include "lightcal.h"
#include "xtimer.h"
#define TAG "LIGHT_ATTR"
#define PWM_STEP_MS 10      					//每10ms更新一次pwm输出


#define PWM(X) (8192*(X)/255)
#define RGB(X) ((X)*255/8192)

typedef struct light_attr_cmd_s{
char *event;
int (*light_attr)(unsigned char*buff,unsigned int lens);
}light_attr_cmd;
QueueHandle_t xLightQueue;
QueueHandle_t xLight_state_queue;

xt_light_state sg_light_state;
static xt_timer *sg_pwm_timer = NULL;

xt_s32 Xrandom(xt_s32 s32Min, xt_s32 s32Max) ;
xt_bool light_set_brightness(xt_u8 level,unsigned char *rgb);

int light_onoff(unsigned char*buff,unsigned int lens);
int light_gradient(unsigned char*buff,unsigned int lens);


const light_attr_cmd light_attr_Cmd[]=
{
{"onoff",light_onoff},
{"gradient",light_gradient},


};

int light_onoff(unsigned char*buff,unsigned int lens)
{
return 0;
}
int light_gradient(unsigned char*buff,unsigned int lens)
{
return 0;
}

void light_pwm(void *parg)
{
   xt_rgb light_pwm;
   memset(&light_pwm,0,sizeof(xt_rgb));
   xLightQueue = xQueueCreate( 10, sizeof(  xt_rgb ) );
   if (NULL==xLightQueue){
     vTaskDelay(100);
	  return;
   }
   	
    while(1){
        //vTaskDelay(100);
		xQueueReceive(xLightQueue, &light_pwm, portMAX_DELAY);	
		gpio_pwm(light_pwm);
	    //ESP_LOGI(TAG, "r:%d g:%d b:%d line:%d",light_pwm.rgb_r_pwm,light_pwm.rgb_g_pwm,light_pwm.rgb_b_pwm,__LINE__);

    }


}

int light_pwm_send(xt_rgb light_pwm)
{
   if (NULL!=xLightQueue){
        xQueueSend(xLightQueue,&light_pwm,0);
		return 0;
   }else{
       return -1;
   }

}
int light_state_send(xt_light_state light_state)
{
   if (NULL!=xLight_state_queue){
        xQueueSend(xLight_state_queue,&light_state,0);
		return 0;
   }else{
       return -1;
   }

}

void light_multi_func(void *parg)
{
    light_mode_e light_mode = LIGHT_MAX;
	xt_light_state light_state;
	xt_rgb light_pwm;
	xt_rgb target_pwm;
	xt_rgb *current_pwm = &light_pwm;
	//float gain = 0;
	unsigned int time =0;
	//unsigned int time_previous = 0;
	unsigned int gradient_time_ms=1000;
	unsigned char rgb[3]={0,0,0};
	int result = pdFALSE;
	unsigned char step = 0;
	unsigned int rand =0;
	unsigned char brightness =100;	
    xLight_state_queue = xQueueCreate(10,sizeof(xt_light_state));
	memset(&light_state,0,sizeof(xt_light_state));
	memset(&light_pwm,0,sizeof(xt_rgb));
	memset(&target_pwm,0,sizeof(xt_rgb));
	sg_pwm_timer = Xtimer(1000);
	if (NULL==xLight_state_queue){
        ESP_LOGI(TAG, "creat queue fail");
	}
    while(1){
       //vTaskDelay(100);
       if (NULL!=xLight_state_queue){
	       result = xQueueReceive(xLight_state_queue,&light_state,10);
       }
	   //ESP_LOGI(TAG, "watermard:%d",uxTaskGetStackHighWaterMark(NULL));
	   light_mode = light_state.light_mode;
	   brightness = light_state.brightness;
	   switch(light_mode){
       case LIGHT_RED:
	   	if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=PWM(255);//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=0;
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		//light_pwm_send(light_pwm);
		//time_previous = 0;        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		   light_state_response(light_state);
		   gradient_time_ms=light_state.gradient_time_ms;
	   }

	   break;
	   case LIGHT_GREEN:
	   	if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=0;//计算目标值
		target_pwm.rgb_g_pwm=PWM(255);
		target_pwm.rgb_b_pwm=0;
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		//light_pwm_send(light_pwm);
		//time_previous = 0;        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		   light_state_response(light_state);
		   gradient_time_ms=light_state.gradient_time_ms;
	   }

	   break;
	   case LIGHT_BLUE:
	   	if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=0;//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=PWM(255);
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		//light_pwm_send(light_pwm);
		//time_previous = 0;        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		   step =1;
		   gradient_time_ms=light_state.gradient_time_ms;
	       light_state_response(light_state);
	   }


	   break;
	   case LIGHT_YELLOW:
	   	if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=PWM(255);//计算目标值
		target_pwm.rgb_g_pwm=PWM(255);
		target_pwm.rgb_b_pwm=0;
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;		        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    light_state_response(light_state);
		   gradient_time_ms=light_state.gradient_time_ms;
	   }

	   break;
	   case LIGHT_PINK:
	   	if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=PWM(255);//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=PWM(255);
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		//light_pwm_send(light_pwm);
		///time_previous = 0;        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    light_state_response(light_state);
		   gradient_time_ms=light_state.gradient_time_ms;
	   }

	   break;
	   case LIGHT_CYAN:
	   	if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=0;//计算目标值
		target_pwm.rgb_g_pwm=PWM(255);
		target_pwm.rgb_b_pwm=PWM(255);
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		//light_pwm_send(light_pwm);
		//time_previous = 0;        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    light_state_response(light_state);
		   gradient_time_ms=light_state.gradient_time_ms;
	   }

	   break;	   
	   case LIGHT_CCT:
	   if(light_state.is_start==XTRUE){
            break;
	   	}
	   	if (pdTRUE==result){
		    cal(rgb,light_state.cct);
			light_set_brightness(brightness,rgb);
		    target_pwm.rgb_r_pwm =rgb[0];
		    target_pwm.rgb_g_pwm =rgb[1];
		    target_pwm.rgb_b_pwm =rgb[2];
		    XtimerStop(sg_pwm_timer);		
		     gradient_time_ms=light_state.gradient_time_ms;
		    //time_previous = 0;        
	   	}
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    light_state_response(light_state);
		   gradient_time_ms=light_state.gradient_time_ms;
	   }		
	   break;
	   case LIGHT_SUNSET:
	   {
	   if (light_state.is_start==XTRUE){
        break;
	   }
	   if (pdTRUE==result){
	       cal(rgb,light_state.cct);
		   light_set_brightness(brightness,rgb);
	       target_pwm.rgb_r_pwm=PWM(rgb[0]);//计算目标值
	       target_pwm.rgb_g_pwm=PWM(rgb[1]);
	       target_pwm.rgb_b_pwm=PWM(rgb[2]);
	       XtimerStop(sg_pwm_timer);
	       //light_state.cct = 2700;
	       gradient_time_ms=light_state.gradient_time_ms;
	       cal(rgb,light_state.cct);
		   
	       ESP_LOGI(TAG, "r:%d g:%d b:%d line:%d",rgb[0],rgb[1],rgb[2],__LINE__);  
	       light_pwm.rgb_r_pwm =PWM(light_state.rgb[0]);
	       light_pwm.rgb_g_pwm =PWM(light_state.rgb[1]);
	       light_pwm.rgb_b_pwm =PWM(light_state.rgb[2]);
	       light_pwm_send(light_pwm);
	       //time_previous = 0;

	   }
	   if(XTRUE== set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		   light_state_response(light_state);
	   }
	   	
	  }
	   break;
	   case LIGHT_MORNING:
	   	if(XTRUE==light_state.is_start){
            break;
	   	}
	   if (pdTRUE==result){
	       cal(rgb,light_state.cct);
		   light_set_brightness(brightness,rgb);
	       target_pwm.rgb_r_pwm=PWM(rgb[0]);//计算目标值
	       target_pwm.rgb_g_pwm=PWM(rgb[1]);
	       target_pwm.rgb_b_pwm=PWM(rgb[2]);
	       XtimerStop(sg_pwm_timer);
	       light_state.cct = 2700;
	       gradient_time_ms=light_state.gradient_time_ms;
	       cal(rgb,light_state.cct);
		   light_set_brightness(brightness,rgb);
	       ESP_LOGI(TAG, "r:%d g:%d b:%d line:%d",rgb[0],rgb[1],rgb[2],__LINE__);
	       light_pwm.rgb_r_pwm =PWM(light_state.rgb[0]);
	       light_pwm.rgb_g_pwm =PWM(light_state.rgb[1]);
	       light_pwm.rgb_b_pwm =PWM(light_state.rgb[2]);
	       //light_pwm_send(light_pwm);
	       //time_previous = 0;

	   }


	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);
            light_state_response(light_state);
	   }
	   break;
	   case LIGHT_WORK:
	   	if(XTRUE==light_state.is_start){
            break;
	   	}
	   	if (pdTRUE==result){
		light_state.cct = 6300;
		cal(rgb,light_state.cct);
		light_set_brightness(brightness,rgb);

		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		target_pwm.rgb_r_pwm =PWM(rgb[0]);
		target_pwm.rgb_g_pwm =PWM(rgb[1]);
		target_pwm.rgb_b_pwm =PWM(rgb[2]);
		//light_pwm_send(light_pwm);
		
		
	   	}	   	
	   if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
	   		light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);
		    light_state_response(light_state);
	   }

	   break;
	   case LIGHT_PARTY:
	   	if (pdTRUE==result){
		target_pwm.rgb_r_pwm=PWM(255);//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=0;
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		light_pwm.rgb_r_pwm =0;
		light_pwm.rgb_g_pwm =PWM(255);
		light_pwm.rgb_b_pwm =0;
		//light_pwm_send(light_pwm);		
		step = Xrandom(0, 5);
	   	}
		switch(step){
        case 0:
		//if (light_state.is_start==XTRUE)	
		target_pwm.rgb_r_pwm=0;//计算目标值
		target_pwm.rgb_g_pwm=PWM(255);
		target_pwm.rgb_b_pwm=0;
		//XtimerStop(sg_pwm_timer);
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

		//time_previous = 0;			
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    step =Xrandom(0, 5);
			gradient_time_ms=light_state.gradient_time_ms;
		}

		break;
		case 1:
		target_pwm.rgb_r_pwm=0;//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=PWM(255);
		//XtimerStop(sg_pwm_timer);
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

		//time_previous = 0;			
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    step =Xrandom(0, 5);
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
			gradient_time_ms=light_state.gradient_time_ms;
		}

		break;
		case 2:
		target_pwm.rgb_r_pwm=0;//计算目标值
		target_pwm.rgb_g_pwm=PWM(255);
		target_pwm.rgb_b_pwm=PWM(255);
		//XtimerStop(sg_pwm_timer);
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

		//time_previous = 0;			
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    step =Xrandom(0, 5);
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
			gradient_time_ms=light_state.gradient_time_ms;
		}

		break;
		case 3:
		target_pwm.rgb_r_pwm=PWM(255);//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=PWM(255);
		//XtimerStop(sg_pwm_timer);
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

		//time_previous = 0;			
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    step =Xrandom(0, 5);
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
			gradient_time_ms=light_state.gradient_time_ms;
		}

		break;
		case 4:
		target_pwm.rgb_r_pwm=PWM(255);//计算目标值
		target_pwm.rgb_g_pwm=0;
		target_pwm.rgb_b_pwm=0;
		//XtimerStop(sg_pwm_timer);
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

		//time_previous = 0;			
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    step =Xrandom(0, 5);
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
			gradient_time_ms=light_state.gradient_time_ms;
		}

		break;
		case 5:
		target_pwm.rgb_r_pwm=PWM(125);//计算目标值
		target_pwm.rgb_g_pwm=PWM(255);
		target_pwm.rgb_b_pwm=PWM(28);
		//XtimerStop(sg_pwm_timer);
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

		//time_previous = 0;			
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    step =Xrandom(0, 5);
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
			gradient_time_ms=light_state.gradient_time_ms;
		}

		break;			
		default:
		;

		}
	   

	   break;
	   case LIGHT_SELF_DEFINE:
	   	 
		ESP_LOGI(TAG,"rand=%d",rand);
	   	if (pdTRUE==result){
		rand=Xrandom(0, 16777215) ;	
		target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16);//计算目标值
		target_pwm.rgb_g_pwm=PWM((rand&0xFF00)>>8);
		target_pwm.rgb_b_pwm=PWM((rand&0xFF));
		XtimerStop(sg_pwm_timer);
		//light_state.cct = 2700;
		gradient_time_ms=light_state.gradient_time_ms;
		light_pwm.rgb_r_pwm =0;
		light_pwm.rgb_g_pwm =PWM(255);
		light_pwm.rgb_b_pwm =0;
		//light_pwm_send(light_pwm);
		        
	   	}
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}

				
		if (XTRUE==set_pwm(&light_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		    rand=Xrandom(0, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16);//计算目标值
		    target_pwm.rgb_g_pwm=PWM((rand&0xFF00)>>8);
		    target_pwm.rgb_b_pwm=PWM((rand&0xFF));
			light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);
			 light_state_response(light_state);
		}

	   	break;
		case LIGHT_ROMATIC:
        if (pdTRUE==result){
		    rand=Xrandom(0, 16777215) ;	
		    target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16);//计算目标值
		    target_pwm.rgb_g_pwm=PWM((rand&0xFF00)>>8);
		    target_pwm.rgb_b_pwm=PWM((rand&0xFF));
		    XtimerStop(sg_pwm_timer);
		    //light_state.cct = 2700;
		    gradient_time_ms=light_state.gradient_time_ms;
		    light_pwm.rgb_r_pwm =0;
		    light_pwm.rgb_g_pwm =PWM(255);
		    light_pwm.rgb_b_pwm =0;
			step =0;
		}
		if(XTRUE==light_state.is_start){
		    light_state.light_effect_start_flag = 0;
            light_state.is_start =XFALSE;
		}		
		switch(step)
		{
        case 0:
		    if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		        rand=Xrandom(10, 16777215) ;
			    gradient_time_ms=light_state.gradient_time_ms;
			    target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
		        target_pwm.rgb_g_pwm=0;
		        target_pwm.rgb_b_pwm=PWM((rand&0xFF));
				step =1;
		   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			    light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	            light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
				 light_state_response(light_state);
		    }			
			
		break;
		case 1:
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
			rand=Xrandom(10, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
			target_pwm.rgb_g_pwm=0;
			target_pwm.rgb_b_pwm=PWM(rand&0xFF);
			step =2;
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
		}


		break;
		case 2:
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
			rand=Xrandom(10, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
			target_pwm.rgb_g_pwm=0;
			target_pwm.rgb_b_pwm=PWM(rand&0xFF);
			step =3;
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
		}

		break;
		case 3:
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
			rand=Xrandom(10, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
			target_pwm.rgb_g_pwm=0;
			target_pwm.rgb_b_pwm=PWM(rand&0xFF);
			step =4;
			 light_state_response(light_state);
		}

		break;
		case 4:
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
			rand=Xrandom(10, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
			target_pwm.rgb_g_pwm=0;
			target_pwm.rgb_b_pwm=PWM(rand&0xFF);
			step =5;
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
		}

		break;
		case 5:
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
			rand=Xrandom(10, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
			target_pwm.rgb_g_pwm=0;
			target_pwm.rgb_b_pwm=PWM(rand&0xFF);
			step =6;			
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);
			light_state_response(light_state);
		}

		break;
		case 6:
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
			rand=Xrandom(10, 16777215) ;
			gradient_time_ms=light_state.gradient_time_ms;
			target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16>0?(rand&0xFF0000)>>16:10);//计算目标值
			target_pwm.rgb_g_pwm=0;
			target_pwm.rgb_b_pwm=PWM(rand&0xFF);
			step =0;
	   	    light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	        light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			 light_state_response(light_state);
		}

		break;
		default:
		    if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){
		        rand=Xrandom(10, 16777215) ;
			    gradient_time_ms=light_state.gradient_time_ms;
			    target_pwm.rgb_r_pwm=PWM((rand&0xFF0000)>>16);//计算目标值
		        target_pwm.rgb_g_pwm=0;
		        target_pwm.rgb_b_pwm=PWM(rand&0xFF);
	   	        light_state.rgb[0]= RGB(target_pwm.rgb_r_pwm);
			    light_state.rgb[1]= RGB(target_pwm.rgb_g_pwm);
	            light_state.rgb[2]= RGB(target_pwm.rgb_b_pwm);			
			    light_state_response(light_state);				
		    }				

		}		
		break;
		case LIGHT_STATE_OFF://延时关灯
        if (pdTRUE==result){		    
		    target_pwm.rgb_r_pwm=PWM(0);//计算目标值
		    target_pwm.rgb_g_pwm=PWM(0);
		    target_pwm.rgb_b_pwm=PWM(0);
		    XtimerStop(sg_pwm_timer);
		    //light_state.cct = 2700;
		    gradient_time_ms=light_state.gradient_time_ms;
		}		
	    if (XTRUE==light_state.light_effect_start_flag){
            break;
	    }
		if (XTRUE==set_pwm(current_pwm,&target_pwm,&gradient_time_ms,&light_state, &time)){            
			if(XTRUE== light_off_response( )){
			    ESP_LOGI(TAG,"power off\r\n");
			}
		}

		break;
	   default:
	   ;
	   }
      

    }




}

bool set_pwm(xt_rgb *current_pwm,xt_rgb *target_pwm,unsigned int *gradient_time,xt_light_state *light_state, unsigned int *time)
{
    static int time_previous =0;
	float gain =0;
	float percent =0;
    if (NULL==current_pwm||NULL==target_pwm||NULL==gradient_time||NULL==light_state)
    {
        ESP_LOGI(TAG,"error pointer");
		return XFALSE;
    }
	int gradient_times = light_state->gradient_time_ms;
	if (XtimerIsStopt(sg_pwm_timer)) {
		XtimerUpdateExpired(sg_pwm_timer, gradient_times);
		XtimerStart(sg_pwm_timer);
	}

	if (XTRUE == XtimerIsExpired(sg_pwm_timer)) {
	   ESP_LOGI(TAG,"timer is expired");
	   light_state->light_effect_start_flag = 1;
	   light_state->is_start =XTRUE;
           return XTRUE;
	}

	if (0==light_state->light_effect_start_flag){
	          
       *time = XtimerPassMs(sg_pwm_timer->oldtick);
	   if (PWM_STEP_MS > (*time - time_previous)){
               return XFALSE;
       }else{
               time_previous = *time;
        }/**/
		   gain = *time / (xt_float)gradient_times;
		   ESP_LOGI(TAG, "gain:%f time:%d  gradient_time_ms:%d line:%d",gain,time,gradient_times,__LINE__);
		   gain = (gain > 1.0)?(1.0):(gain);
		   percent = current_pwm->rgb_r_pwm + (target_pwm->rgb_r_pwm - current_pwm->rgb_r_pwm) * gain; 
		   current_pwm->rgb_r_pwm=percent;
		   percent = current_pwm->rgb_g_pwm + (target_pwm->rgb_g_pwm - current_pwm->rgb_g_pwm) * gain; 
		   current_pwm->rgb_g_pwm=percent;
		   percent = current_pwm->rgb_b_pwm + (target_pwm->rgb_b_pwm - current_pwm->rgb_b_pwm) * gain; 
		   current_pwm->rgb_b_pwm=percent;
		   ESP_LOGI(TAG, "r:%d g:%d b:%d gain:%f line:%d",current_pwm->rgb_r_pwm,current_pwm->rgb_g_pwm,current_pwm->rgb_b_pwm,gain,__LINE__);
		   light_pwm_send(*current_pwm);
		   
		 }
	return XFALSE;

}

/* xrandom */
xt_s32 Xrandom(xt_s32 s32Min, xt_s32 s32Max) {
    xt_s32 s32Num;
    xt_u32 rand;

     rand=esp_random();
    s32Num = rand % (s32Max - s32Min + 1);
    return s32Num + s32Min;
}
xt_bool light_set_brightness(xt_u8 level,unsigned char *rgb)
{
   // 100/level=255/rgb
   //rgb=255*level/100
   if (NULL==rgb){
       return XFALSE;
   }
   rgb[0] =255*level/100;
   rgb[1] =255*level/100;
   rgb[2] =255*level/100;
   return XTRUE;
}

xt_bool light_off_response(xt_void)
{
	cJSON *root = NULL;
	char *json_buf;
	char payload[50];
	root=XjsonCreateObject();
	if (NULL==root){
		return XFALSE;
	}
	memset(payload,0,sizeof(payload));
	XjsonSetInt(root,"state",0);
	json_buf=(char *)malloc(50);
	memset(json_buf,0,50);
	json_buf=XjsonToStringformatted(root);
	serialmethod(payload ,json_buf ,strlen(json_buf));
    cJSON_Delete(root);
	free(json_buf);
	json_buf=NULL;
	root=NULL;
	msg_send(payload,get_topic_pub_array_0(),MQTT_IO_PUBLISH,0,0,strlen(payload),0);
	return XTRUE;   

}
xt_bool light_state_response(xt_light_state light_state)
{
	cJSON *root = NULL;
	char *json_buf;
	char payload[100];
	root=XjsonCreateObject();
	if (NULL==root){
		return XFALSE;
	}
	memset(payload,0,sizeof(payload));
	XjsonSetInt(root,"state",1);
	XjsonSetInt(root,"brightness",light_state.brightness);
	XjsonSetInt(root,"color",light_state.rgb[0]<<16|light_state.rgb[1]<<8|light_state.rgb[2]);
	XjsonSetInt(root,"light_mode",light_state.light_mode);
	json_buf=(char *)malloc(100);
	memset(json_buf,0,100);
	json_buf=XjsonToStringformatted(root);
	serialmethod(payload ,json_buf ,strlen(json_buf));
    cJSON_Delete(root);
	free(json_buf);
	json_buf=NULL;
	root=NULL;
	msg_send(payload,get_topic_pub_array_0(),MQTT_IO_PUBLISH,0,0,strlen(payload),0);
	return XTRUE;   

}

