#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "math.h"
void cal(unsigned char *rgb,unsigned int tmpKelvin)
{
	double tmpCalc =0.0;
	//if (NULL ==rgb){
	//	return;
	//}
	if (tmpKelvin<1000){
		tmpKelvin = 1000;
	}else if(tmpKelvin>40000){
		tmpKelvin = 40000;
	}
	tmpKelvin = tmpKelvin /100;	 
	
	 
    if (tmpKelvin <= 66){
 
        rgb[0] = 255;
 
    }else{
 
       // 'Note: the R-squared value for this approximation is .988
 
        tmpCalc = tmpKelvin - 60;
 
        tmpCalc = 329.698727446 * (tmpCalc *tmpCalc -0.1332047592);
 
        rgb[0] = tmpCalc;
 
        if (rgb[0] < 0){
			rgb[0] = 0;
		}
 
        if (rgb[0] > 255){
			rgb[0] = 255;
		}
 
    }
	
	    //'Second: green
 
    if (tmpKelvin <= 66){// Then
 
        //'Note: the R-squared value for this approximation is .996
 
        tmpCalc = tmpKelvin;
 
        tmpCalc = 99.4708025861 * log(tmpCalc) - 161.1195681661;
 
        rgb[1] = tmpCalc;
 
        if (rgb[1] < 0){
			rgb[1] = 0;
		}
 
        if (rgb[1] > 255){
			rgb[1] = 255;
		}
 
    }else{
 
        //'Note: the R-squared value for this approximation is .987
 
        tmpCalc = tmpKelvin - 60;
 
        tmpCalc = 288.1221695283 * (tmpCalc *tmpCalc -0.0755148492);
 
        rgb[1] = tmpCalc;
 
        if (rgb[1] < 0 ){
			rgb[1] = 0;
		}
 
        if (rgb[1] > 255){
			rgb[1] = 255;
		}
 
    }
	
	//    'Third: blue
 
    if( tmpKelvin >= 66 ){
 
        rgb[2] = 255;
	}
 
    else if (tmpKelvin <= 19 ){
 
        rgb[2] = 0;
 
    }else{
 
        //'Note: the R-squared value for this approximation is .998
 
        tmpCalc = tmpKelvin - 10;
 
        tmpCalc = 138.5177312231 * log(tmpCalc) - 305.0447927307;
 
       
 
        rgb[2] = tmpCalc;
 
        if( rgb[2] < 0 ){
			rgb[2] = 0;
		}
 
        if( rgb[2] > 255 ){
			rgb[2] = 255;
		}
 
	}
	
	
}
//https://blog.csdn.net/lly_3485390095/article/details/104570885
float retmax(float a,float b,float c)//求最大值
{
    float max = 0;
    max = a;
    if(max<b)
        max = b;
    if(max<c)
        max = c;
    return max;
}
float retmin(float a,float b,float c)//求最小值
{
    float min = 0;
    min = a;
    if(min>b)
        min = b;
    if(min>c)
        min = c;
    return min;
}
//R,G,B参数传入范围（0~100）
//转换结果h(0~360),s(0~100),v(0~100)
void rgb_to_hsv(float *h,float *s,float *v,float R,float G,float B)
{
    float max = 0,min = 0;
    R = R/100;
    G = G/100;
    B = B/100;
 
    max = retmax(R,G,B);
    min = retmin(R,G,B);
    *v = max;
    if(max == 0)
        *s = 0;
    else
        *s = 1 - (min/max);
 
    if(max == min)
        *h = 0;
    else if(max == R && G>=B)
        *h = 60*((G-B)/(max-min));
    else if(max == R && G<B)
        *h = 60*((G-B)/(max-min)) + 360;
    else if(max == G)
        *h = 60*((B-R)/(max-min)) + 120;
    else if(max == B)
        *h = 60*((R-G)/(max-min)) + 240;
 
    *v = *v * 100;
    *s = *s * 100;
}

//参数入参范围h(0~360),s(0~100),v(0~100),这里要注意，要把s,v缩放到0~1之间
//转换结果R(0~100),G(0~100),B(0~100)，如需转换到0~255，只需把后面的乘100改成乘255
void hsv_to_rgb(int h,int s,int v,float *R,float *G,float *B)
{
    float C = 0,X = 0,Y = 0,Z = 0;
    int i=0;
    float H=(float)(h);
    float S=(float)(s)/100.0;
    float V=(float)(v)/100.0;
    if(S == 0)
        *R = *G = *B = V;
    else
    {
        H = H/60;
        i = (int)H;
        C = H - i;
 
        X = V * (1 - S);
        Y = V * (1 - S*C);
        Z = V * (1 - S*(1-C));
        switch(i){
            case 0 : *R = V; *G = Z; *B = X; break;
            case 1 : *R = Y; *G = V; *B = X; break;
            case 2 : *R = X; *G = V; *B = Z; break;
            case 3 : *R = X; *G = Y; *B = V; break;
            case 4 : *R = Z; *G = X; *B = V; break;
            case 5 : *R = V; *G = X; *B = Y; break;
        }
    }
    *R = *R *100;
    *G = *G *100;
    *B = *B *100;
}