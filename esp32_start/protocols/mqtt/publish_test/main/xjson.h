/**
 * @file    xjson.h
 * @brief   JSON APIs base on cJSON
 * @author  zhangxc, zhangxc@leedarson.com
 * @version v1.0.0
 * @date    W31 Tue, 2016-08-02 21:22:11
 * @par     Copyright
 * Copyright (c) Leedarson Lighting 2000-2015. All rights reserved.
 *
 * @par     History
 * 1.Date        :W31 Tue, 2016-08-02 21:22:11
 *   Author      :zhangxc
 *   Version     :v1.0.0
 *   Modification:Create file
 */
#ifndef __XJSON_H__
#define __XJSON_H__

#include <stdlib.h>
#include <stdio.h>
#include "xstddef.h"
#include "string.h"
#if 1
#include "cJSON.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


#define XJSON_MAX_PATH_LENGTH 128

typedef cJSON *xt_json;

#define XjsonCreateObject() cJSON_CreateObject()
#define XjsonDelete(obj) cJSON_Delete(obj)
#define XjsonToStringformatted(obj) cJSON_Print(obj)
#define XjsonToString(obj) cJSON_PrintUnformatted(obj)
#define XjsonFromString(str) cJSON_Parse(str)
#define XjsonValue(obj, key) cJSON_GetObjectItem(obj, key)

xt_s32 XjsonParsePath(xt_rostring key,
                      xt_string parent_path, xt_s32 path_size,
                      xt_string key_name, xt_s32 key_size);
xt_json XjsonSearch(xt_json root, xt_rostring path);
xt_s32 XjsonSetJson(xt_json root, xt_rostring path, xt_json json);

xt_s32 XjsonLength(xt_json root, xt_rostring path);
xt_s32 XjsonKeysCount(xt_json root, xt_rostring path);
xt_s32 XjsonKeys(xt_json root,
                 xt_rostring path,
                 xt_string keys[],
                 xt_s32 maxsize);
xt_json XjsonSearchParent(xt_json root, xt_rostring path);
xt_json XjsonNewParent(xt_json root,
                       xt_rostring path,
                       xt_string node,
                       xt_s32 bufsize);

xt_s32 XjsonGetInt(xt_json root,
                   xt_rostring path,
                   xt_s32 *value,
                   xt_s32 defvalue);
xt_s32 XjsonGetString(xt_json root,
                      xt_rostring path,
                      xt_string value,
                      xt_s32 bufsize,
                      xt_rostring defvalue);
xt_s32 XjsonSetInt(xt_json root, xt_rostring path, xt_s32 value);
xt_s32 XjsonSetString(xt_json root, xt_rostring path, xt_rostring value);

xt_s32 XjsonArrayAppendInt(xt_json root, xt_rostring path, xt_s32 value);
xt_s32 XjsonArrayGetInt(xt_json root,
                        xt_rostring path,
                        xt_s32 values[],
                        xt_s32 maxsize);
xt_s32 XjsonArrayGetIntIndex(xt_json root, xt_rostring path, xt_s32 value);
xt_s32 XjsonArrayRemoveInt(xt_json root, xt_rostring path, xt_s32 value);
xt_s32 XjsonArrayRemoveDouble(xt_json root, xt_rostring path, xt_double value);

xt_s32 XjsonArrayAppendString(xt_json root, xt_rostring path, xt_rostring value);
xt_s32 XjsonArrayGetStringIndex(xt_json root, xt_rostring path, xt_rostring value);
xt_s32 XjsonArrayGetString(xt_json root, xt_rostring path,
                           xt_string values[], xt_s32 maxsize);
xt_s32 XjsonArrayRemoveString(xt_json root, xt_rostring path, xt_rostring value);


xt_s32 XjsonArrayAppendJson(xt_json root, xt_rostring path, xt_json value);
xt_s32 XjsonArrayGetJsonIndex(xt_json root, xt_rostring path, xt_json value);
xt_s32 XjsonArrayGetJson(xt_json root, xt_rostring path,
                           xt_json values[], xt_s32 maxsize);
xt_s32 XjsonArrayRemoveJson(xt_json root, xt_rostring path, xt_json value);

xt_s32 XjsonArrayDeleteInt(xt_json root, xt_rostring path, xt_s32 value);
xt_s32 XjsonArrayDeleteIndex(xt_json root, xt_rostring path, xt_s32 index);
xt_s32 XjsonArrayDeleteString(xt_json root, xt_rostring path, xt_rostring value);

xt_s32 XjsonRemove(xt_json root, xt_rostring path);
xt_s32 XjsonDump(xt_rostring msg, xt_json object);
xt_s32 XjsonArrayAppendDouble(xt_json root, xt_rostring path, xt_double value);
xt_s32 XjsonArrayGetDouble(xt_json root,
                        xt_rostring path,
                        xt_double values[],
                        xt_s32 maxsize);
xt_s32 XjsonArrayGetDoubleIndex(xt_json root, xt_rostring path, xt_double value);
xt_s32 XjsonArrayRemoveDouble(xt_json root, xt_rostring path, xt_double value);

xt_s32 XjsonSetDouble(xt_json root, xt_rostring path, xt_double value);
xt_s32 XjsonGetDouble(xt_json root, xt_rostring path,
                            xt_double *value, xt_double defvalue);
xt_s32 XjsonGetKeyAndIntByIndex(xt_json root, xt_rostring path, xt_s32 index,
                           xt_string key, xt_s32 key_size, xt_s32 *value);



#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __XJSON_H__
