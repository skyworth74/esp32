协议解析
light_mode 灯的模式
typedef enum{
LIGHT_RED,//红色
LIGHT_GREEN,//绿色
LIGHT_BLUE,//蓝色
LIGHT_YELLOW,//黄色
LIGHT_PINK,//粉色、紫色
LIGHT_CYAN,//5青色
LIGHT_ROMATIC,//罗曼蒂克
LIGHT_FOCUSE,//聚焦
LIGHT_SUNSET,//日落
LIGHT_MORNING,//早晨
LIGHT_WORK,//10//工作模式
LIGHT_PARTY,//舞会模式
LIGHT_BLINK,//闪烁模式
LIGHT_SELF_DEFINE,//自定义模式
LIGHT_CCT,//14//CCT 模式
模式决定了解析内容,比如CCT模式解析成功后只会用到cct键值
红色模式忽略除颜色和渐变时间键值以外的内容
state 表示灯的开关状态


LIGHT_MAX
}light_mode_e;

{
  "color": 5570560,
  "cool": 20,
  "warm": 43,
  "light_mode":13,
  "gradient_time_ms":100,
  "cct":6400,
  "brightness":80,
  "state": 1,
  "ucMessageID": 0,
  "topic_index": 0
}

