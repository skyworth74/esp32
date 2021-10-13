/**
 * @file    xtime.h
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

#ifndef __XTIME_H__
#define __XTIME_H__

#include "xstddef.h"

#include <time.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */

struct xt_daytime_t {
  xt_u8 sec;
  xt_u8 min;
  xt_u8 hour;
  xt_u8 day;
  xt_u8 week;
  xt_u8 month;
  xt_u16 year;
};
typedef struct xt_daytime_t xt_daytime;

typedef struct tm xt_tm;    // tm_year + 1900 = now_year, tm_mon + 1 = now_mon, tm_mday, tm_hour, tm_min, tm_sec, tm_wday
typedef time_t xt_time; // xt_u64


struct xt_timezone_t {
	xt_s32 offset_hour;
	xt_s32 offset_minute;
	xt_s32 dst_enable;
	xt_s32 dst_started;
	xt_s32 dst_offset_hour;
	xt_s32 dst_offset_mintue;
	xt_s32 dst_start_month;
	xt_s32 dst_start_day;
	xt_s32 dst_start_hour;
	xt_s32 dst_start_minute;
	xt_s32 dst_end_month;
	xt_s32 dst_end_day;
	xt_s32 dst_end_hour;
	xt_s32 dst_end_minute;
};
typedef struct xt_timezone_t xt_timezone;



struct xt_timer_t {
  xt_bool used;
  xt_u32 status;
  xt_u64 expired_ms;
  xt_u64 oldtick;
};
typedef struct xt_timer_t xt_timer;

#define XtimerDeclare(name) static xt_timer *name = NULL


/**
 * @brief Get HZ for system, SHOULD be implemented in "port" directory for different targets.
 *
 * @return System HZ
 */
xt_u32 XtimeGetHZ(void);

/**
 * @brief Get tick for system, SHOULD be implemented in "port" directory for different targets.
 *
 * @return System tick
 */
xt_u64 XtimeGetTick(void);

/**
 * @brief Get used timer
 *
 *
 * @return The count of used timer
 */
xt_s32 XtimerCount(void);

/**
 * @brief Allocate a new timer from timer pool
 *
 * @param expired_ms [in] Set the expired time for timer in millisecond
 *
 * @return NON-NULL: Allocated successfully\n
 *         NULL: Allocated failed
 */
xt_timer *Xtimer(xt_s32 expired_ms);

/**
 * @brief Start the timer
 *
 * @param timer [in] Timer has been allocated
 *
 * @return XOK: Start successfully\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerStart(xt_timer *timer);

/**
 * @brief Stop the timer
 *
 * @param timer [in] Timer has been allocated
 *
 * @return XOK: Successfully\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerStop(xt_timer *timer);

/**
 * @brief Restart the timer
 *
 * @param timer [in] Timer has been allocated
 *
 * @return XOK: Successfully\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerRestart(xt_timer *timer);

/**
 * @brief Check whether the timer is started
 *
 * @param timer [in] Timer has been allocated
 *
 * @return XTRUE: Timer is started\n
 *         XFALSE: Not started\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerIsStarted(xt_timer *timer);

/**
 * @brief Check whether the timer is stopt or not
 *
 * @param timer [in] Time has been allocated
 *
 * @return XTRUE: Timer is stopt\n
 *         XFALSE: Not stopt\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerIsStopt(xt_timer *timer);

/**
 * @brief Check whether the timer is expired
 *
 * @param timer [in] Timer has been allocated
 *
 * @return XTRUE: Expiration occures\n
 *         XFALSE: Not expired\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerIsExpired(xt_timer *timer);
xt_s32 XtimerUpdateExpired(xt_timer *timer, xt_s32 expired_ms);
xt_s32 XtimerGetExpired(xt_timer *timer);

/**
 * @brief Remove the allocated timer
 *
 * @param timer [in] Timer has been allocated
 *
 * @return XOK: successfully\n
 *         XERROR_FUNC_ARGS: Invalid function parameter
 */
xt_s32 XtimerRemove(xt_timer *timer);

/**
 * @brief Sleep for specified ms milliseconds
 *
 * @param ptimer [in] A static timer point, you can defined as "static xt_timer *ptimer = NULL;"
 * @param ms     [in] Milliseconds to sleep
 *
 * @return XOK: We have sleeped ms milliseconds\n
 *         XERROR_FUNC_ARGS: Invalid function parameter\n
 *         XERROR_MEM: No more timer or memory can be used
 */
xt_s32 XtimerMsleep(xt_timer **ptimer, xt_s32 ms);
xt_s32 XnameTimerMsleep(xt_rostring name, xt_timer **ptimer, xt_s32 ms);
#define XTIMER_MSLEEP(ptimer, ms) XnameTimerMsleep(__FUNCTION__, ptimer, ms)

/**
 * @brief Get different ticks between old and new ticks
 *
 * @param old_tick [in] Older tick
 * @param new_tick [in] New tick
 *
 * @return Different ticks
 */
xt_u64 XtimeDiffTick(xt_u64 old_tick, xt_u64 new_tick);

/**
 * @brief Get milliseconds passed from old_tick to now
 *
 * @param old_tick [in] Older tick
 *
 * @return Milliseconds passed
 */
xt_u32 XtimerPassMs(xt_u64 old_tick);
xt_u32 XtimeGetMs(xt_void);



xt_s32 XtimerStopLeftTimeMs(xt_timer *timer);

xt_s32 XtimeNextUTC(xt_s32 hhmm);
xt_s32 XtimeDaySeconds(void);
xt_s32 XtimeDayHHMM(void);
xt_s32 XtimeDayTime(xt_daytime *d);
xt_s32 XtimeLocalDayTime(xt_daytime *local_d);
xt_s32 XtimeDay(xt_tm *t);
xt_time XtimeLocalDay(xt_tm *t);
xt_s32 XtimeUTC(void);
xt_s32 XtimeWeek(void);
xt_s32 XtimeUTC2HHMM(xt_s32 utc);
xt_s32 XtimeDayHHMMOffset(xt_s32 hhmm, xt_s32 offset_min);
xt_s32 XtimeTargetUTC(xt_s32 hhmm);
xt_s32 XtimeRepeatToDay(xt_s32 repeat[], xt_u8 count);
xt_s32 XtimeDayIsValid(xt_u8 day);
xt_s32 XtimeLocalDayHHMM(xt_void);
xt_s32 XtimeIsSync(xt_void);
xt_s32 XtimeStrTime(xt_u8 *str_time, xt_s32 size);
xt_s32 XtimeStrTime2UTC(xt_string str_time);

/**
 * @brief xt_tm ==> xt_time, not include timezone offset
 *
 * @param [in] xt_tm *t  
 * @param [out] None
 * 
 * @return 
 * 
 * @history
 * 1.Date         : 2018-7-24 12:52:24
 *   Author       : wangjinluan
 *   Modification : Created function
 */
xt_s32 XtimeMake(xt_tm *t);


/**
 * @brief xt_time ==> xt_tm
 *
 * @param [in] xt_s32 utc  
 * @param [out] None
 * 
 * @return xt_tm
 * 
 * @history
 * 1.Date         : 2018-7-24 12:52:29
 *   Author       : wangjinluan
 *   Modification : Created function
 */
xt_tm *XtimeClock(xt_s32 utc);



/*----------------------------------------------*
 * Time management configuration                *
 *----------------------------------------------*/
xt_timer *XtimeTimerPool(void);
xt_s32 XtimeTimerMaxCount(void);
xt_s32 XtimeInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __XTIME_H__ */

