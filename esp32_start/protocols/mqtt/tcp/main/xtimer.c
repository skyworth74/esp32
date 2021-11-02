/**
 * @file    xtime.c
 * @brief   System time ticks
 * @author  zhangxc, zhangxc@leedarson.com
 * @version v1.0.0
 * @date    W51 Wed, 2015-12-16 07:06:40
 * @par     Copyright
 * Copyright (c) Leedarson Lighting 2000-2015. All rights reserved.
 *
 * @par     History
 * 1.Date        :W51 Wed, 2015-12-16 07:06:40
 *   Author      :zhangxc
 *   Version     :v1.0.0
 *   Modification:Create file
 */

#define XLOG_SET_TAG "xtimer"

#include "xtimer.h"
#include "string.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define XLOG_NONE(x,...) 
#define XLOG_D(x, ...)
#define XLOG_W(x, ...)
#define XLOG_E(X,...)

#define TIMEOUT_STATE_STOP  0
#define TIMEOUT_STATE_START 1

#define MAX_TIME_TIMER_COUNT 128

#define IDLE_TIMER_ID 0

#define TIMER_STATE_STOP  0
#define TIMER_STATE_START 1

static xt_timer *xtime_timers;

/*----------------------------------------------*
 * Time management configuration                *
 *----------------------------------------------*/  
static xt_timer sg_xtime_timers[MAX_TIME_TIMER_COUNT] = {0x00};
xt_timer *XtimeTimerPool(void) {
    return sg_xtime_timers;
}

xt_s32 XtimeTimerMaxCount(void) {
    return MAX_TIME_TIMER_COUNT;
}
xt_u32 XtimeGetHZ(void) {
    return 1000;
}

xt_s32 XtimeInit(void) {
  xt_s32 i;
  xtime_timers = XtimeTimerPool();

  for (i = 0; i < XtimeTimerMaxCount(); i++) {
    xtime_timers[i].status = TIMER_STATE_STOP;
    xtime_timers[i].used = 0;
  }

  return XOK;
}

xt_bool XtimerIsValid(xt_timer *timer) {
  if (NULL == timer) {
    return XFALSE;
  }

  xt_s32 i;

  for (i = 0; i < XtimeTimerMaxCount(); i++) {
    if (timer == &xtime_timers[i]) {
      return XTRUE;
    }
  }

  return XFALSE;
}

xt_s32 XtimerCount(void) {
  xt_s32 i;
  xt_s32 count = 0;

  for (i = 0; i < XtimeTimerMaxCount(); i++) {
    if (xtime_timers[i].used) {
      count++;
    }
  }

  return count;
}

xt_timer *Xtimer(xt_s32 expired_ms) {
  xt_s32 i;

  if (0 >= expired_ms) {
    return NULL;
  }

  for (i = 0; i < XtimeTimerMaxCount(); i++) {
    if (xtime_timers[i].used == XFALSE) {
      xtime_timers[i].used = XTRUE;
      xtime_timers[i].status = TIMER_STATE_STOP;
      xtime_timers[i].expired_ms = expired_ms;
      xtime_timers[i].oldtick = 0;
      XLOG_NONE("new timer:%p[%d]", &xtime_timers[i], i);
      return &xtime_timers[i];
    }
  }

  return NULL;
}

xt_s32 XtimerStart(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  timer->status = TIMER_STATE_START;
  timer->oldtick = XtimeGetTick();
  return XOK;
}

xt_s32 XtimerIsStarted(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  return (TIMER_STATE_START == timer->status);
}

xt_s32 XtimerIsStopt(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  return (TIMER_STATE_STOP == timer->status);
}

xt_s32 XtimerUpdateExpired(xt_timer *timer, xt_s32 expired_ms) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  timer->expired_ms = expired_ms;
  return XOK;
}
xt_s32 XtimerGetExpired(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  return timer->expired_ms;
}

xt_s32 XtimerIsExpired(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  if (XTRUE == XtimerIsStarted(timer)) {
    xt_u64 tick = XtimeGetTick();
    xt_u64 diff = XtimeDiffTick(timer->oldtick, tick);
    xt_u64 expired_ms = (timer->expired_ms * XtimeGetHZ()) / 1000;

    if (diff >= expired_ms) {
      XtimerStop(timer);
      XLOG_NONE("exp");
      return XTRUE;
    }
  }

  return XFALSE;
}


xt_s32 XtimerStopLeftTimeMs(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  if (timer == (xt_timer *)10029750) {
    XLOG_W("XtimerStopLeftTimeMs timer:%p",  timer);
  }

  if (XTRUE == XtimerIsStarted(timer)) {
    xt_u32 ms = XtimerPassMs(timer->oldtick);

    if ((timer->expired_ms - (xt_s32)ms) <= 0) {
      return 0;
    } else {
      return timer->expired_ms - ms;
    }
  }

  return timer->expired_ms;
}

xt_s32 XtimerStop(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  if (timer == (xt_timer *)10029750) {
    XLOG_W("XtimerStop timer:%p",  timer);
  }

  timer->status = TIMER_STATE_STOP;
  timer->oldtick = 0;
  return XOK;
}

xt_s32 XtimerSearchStop(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  XLOG_D("XtimerStop timer:%p",  timer);
  timer->status = TIMER_STATE_STOP;
  timer->oldtick = 0;
  return XOK;
}


xt_s32 XtimerRestart(xt_timer *timer) {
  XtimerStop(timer);
  XtimerStart(timer);

  if (timer == (xt_timer *)10029750) {
    XLOG_D("XtimerRestart timer:%p",  timer);
  }

  return XOK;
}

xt_s32 XtimerRemove(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  timer->used = XFALSE;
  timer->status = TIMER_STATE_STOP;
  timer->expired_ms = 0;
  timer->oldtick = 0;

  if (timer == (xt_timer *)10029750) {
    XLOG_D("XtimerRemove timer:%p",  timer);
  }

  return XOK;
}

xt_s32 XtimerSearchRemove(xt_timer *timer) {
  if (!XtimerIsValid(timer)) {
    return XERROR_FUNC_ARGS;
  }

  timer->used = XFALSE;
  timer->status = TIMER_STATE_STOP;
  timer->expired_ms = 0;
  timer->oldtick = 0;
  XLOG_W("XtimerRemove timer:%p", timer);
  return XOK;
}

xt_s32 XtimerMsleep(xt_timer **ptimer, xt_s32 ms) {
  xt_timer *timer;

  if (0 > ms) {
    return XERROR_FUNC_ARGS;
  }

  if (0 == ms) {
    return XOK;
  }

  if (NULL == ptimer) {
    return XERROR_FUNC_ARGS;
  }

  if (NULL == *ptimer) {
    *ptimer = Xtimer(ms);
    XtimerStart(*ptimer);
    XLOG_NONE("XtimerMsleep timer:%p start", *ptimer);
  }

  if (NULL == *ptimer) {
    return XERROR_MEM;
  }

  timer = *ptimer;
  xt_s32 ret = XtimerIsExpired(timer);

  if (XTRUE == ret) {
    XtimerStop(timer);
    XtimerRemove(timer);
    *ptimer = NULL;
    return XOK;
  } else {
    return XERROR;
  }
}

static xt_s32 nameTimerMsleep(xt_rostring name, xt_timer **ptimer, xt_s32 ms) {
  xt_timer *timer;

  if (0 > ms) {
    return XERROR_FUNC_ARGS;
  }

  if (0 == ms) {
    return XOK;
  }

  if (NULL == ptimer) {
    return XERROR_FUNC_ARGS;
  }

  if (NULL == *ptimer) {
    *ptimer = Xtimer(ms);
    XtimerStart(*ptimer);
    XLOG_NONE("XtimerMsleep timer:%p start", *ptimer);
  }

  if (NULL == *ptimer) {
    return XERROR_MEM;
  }

  timer = *ptimer;
  xt_s32 ret = XtimerIsExpired(timer);

  if (XTRUE == ret) {
    XtimerStop(timer);
    XLOG_NONE("XtimerRemove timer %s:[%p]",  name, timer);
    XtimerRemove(timer);
    *ptimer = NULL;
    return XOK;
  } else {
    return XERROR;
  }
}

xt_s32 XnameTimerMsleep(xt_rostring name, xt_timer **ptimer, xt_s32 ms) {
  if (ptimer == NULL) {
    XLOG_E("invalid timer");
    return XERROR_FUNC_ARGS;
  }

  if (*ptimer == NULL) {
    XLOG_NONE("timer:%s, ms:%d", name, ms);
  }

  return nameTimerMsleep(name, ptimer, ms);
}

xt_u64 XtimeDiffTick(xt_u64 old_tick, xt_u64 new_tick) {
  xt_u64 max = 0XFFFFFFFFFFFFFFFF;

  if (new_tick >= old_tick) {
    return new_tick - old_tick;
  }

  return (max - old_tick + new_tick);
}

xt_u32 XtimeGetMs(xt_void) {
  xt_u64 now_tick = XtimeGetTick();
  return (now_tick * 1000) / XtimeGetHZ();
}

xt_u32 XtimerPassMs(xt_u64 old_tick) {
  xt_u64 now_tick = XtimeGetTick();
  xt_u64 diff_tick = XtimeDiffTick(old_tick, now_tick);
  return (diff_tick * 1000) / XtimeGetHZ();
}



WEAK xt_s32 XtimeDay(xt_tm *t) {
  memset(t, 0, sizeof(*t));
  return -1;
}

WEAK xt_time XtimeLocalDay(xt_tm *t) {
  memset(t, 0, sizeof(*t));
  return -1;
}


WEAK xt_s32 XtimeMake(xt_tm *t) {
  return -1;
}

WEAK xt_s32 XtimeUTC(void) {
  return -1;
}

WEAK xt_s32 XtimeWeek(void) {
  return -1;
}

WEAK xt_s32 XtimeUTC2HHMM(xt_s32 utc) {
  return 0;
}


xt_s32 XtimeDayTime(xt_daytime *d) {
  xt_tm t;

  if (NULL == d) {
    return -1;
  }

  XtimeDay(&t);
  d->year = t.tm_year + 1900;
  d->month = t.tm_mon + 1;
  d->day = t.tm_mday;
  d->hour = t.tm_hour;
  d->min = t.tm_min;
  d->sec = t.tm_sec;
  d->week = t.tm_wday;
  return 0;
}

xt_s32 XtimeLocalDayTime(xt_daytime *d) {
  xt_tm t;

  if (NULL == d) {
    return -1;
  }

  XtimeLocalDay(&t);
  d->year = t.tm_year + 1900;
  d->month = t.tm_mon + 1;
  d->day = t.tm_mday;
  d->hour = t.tm_hour;
  d->min = t.tm_min;
  d->sec = t.tm_sec;
  d->week = t.tm_wday;
  return 0;
}


xt_s32 XtimeTargetUTC(xt_s32 hhmm) {
  xt_tm t;
  XtimeDay(&t);
  t.tm_hour = (hhmm >> 8) & 0xFF;
  t.tm_min = hhmm & 0xFF;
  t.tm_sec = 0;
  return XtimeMake(&t);
}

xt_s32 XtimeDaySeconds(void) {
  return 60 * 60 * 24;
}

xt_s32 XtimeDayHHMM(void) {
  xt_tm t;
  XtimeDay(&t);
  return (t.tm_hour << 8) | t.tm_min;
}

xt_s32 XtimeDayHHMMOffset(xt_s32 hhmm, xt_s32 offset_min) {
  xt_s32 hour = (hhmm >> 8) & 0xfF;
  xt_s32 min =  hhmm & 0xfF;

  if (hour == 0) {
    hour = 24;
  }

  xt_s32 total_min = hour * 60 + min;
  total_min = total_min + offset_min;
  hour = (total_min / 60) % 24;
  min = total_min % 60;
  return (hour << 8) | min;
}

xt_s32 XtimeNextUTC(xt_s32 hhmm) {
  xt_s32 current = XtimeUTC();
  xt_s32 target = XtimeTargetUTC(hhmm);
  //Xprintf("current:%d target:%d hhmm:%x\n", current, target, hhmm);

  if (current >= target) {
    return target + XtimeDaySeconds();
  } else {
    return target;
  }
}

xt_s32 XtimeRepeatToDay(xt_s32 repeat[], xt_u8 count) {
    if (repeat == NULL)
        return XERROR;
    
    xt_u8 i = 0;
    xt_u8 day = 0;
    for (i = 0; i < count; i++) {
        day = day | (1 << repeat[i]);
    }
    return day;
}

xt_s32 XtimeDayIsValid(xt_u8 day) {
    xt_daytime local_daytime;
    if (day == 0xff)
        return XERROR;
    XtimeLocalDayTime(&local_daytime);
    xt_u8 mask = 1 << local_daytime.week;
    if (day & mask)
        return XOK;
    else 
        return XERROR;
}

xt_s32 XtimeIsSync(xt_void) {
    xt_time t = XtimeUTC();
    if (t < 1514736000)
        return XERROR;
    else 
        return XOK;
}


xt_s32 XtimeLocalDayHHMM(xt_void) {
  xt_tm t;
  XtimeLocalDay(&t);
  return (t.tm_hour << 8) | t.tm_min;
}


// 1532052947 ==> 2018-07-20 10:15:47 000
//                                     |
//                                     ----ms
xt_s32 XtimeStrTime(xt_u8 *str_time, xt_s32 size) { 
    xt_daytime daytime;
    XtimeLocalDayTime(&daytime);

    if (str_time == NULL)
        return XERROR;

    // 2018-07-20 01:51:16 890
    if (size < 24)
        return XERROR;

    snprintf((char *)str_time, size, "%.2d-%.2d-%.2d %.2d:%.2d:%.2d 000", 
               daytime.year, daytime.month, daytime.day, daytime.hour, daytime.min, daytime.sec);
    return XOK;
}

xt_s32 XtimeStrTime2UTC(xt_string str_time) {
    if (str_time == NULL)
        return XERROR;
    xt_tm tm;
    memset(&tm, 0, sizeof(tm));

    sscanf(str_time, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    xt_s32 utc = XtimeMake(&tm);

    return utc;
}

xt_u64 XtimeGetTick(void)
{
	return xTaskGetTickCount( );

}


