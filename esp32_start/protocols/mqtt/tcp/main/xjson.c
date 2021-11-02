/**
 * @file    xjson.c
 * @brief   JSON APIs base on cJSON
 * @author  zhangxc, zhangxc@leedarson.com
 * @version v1.0.0
 * @date    W31 Tue, 2016-08-02 21:21:30
 * @par     Copyright
 * Copyright (c) Leedarson Lighting 2000-2015. All rights reserved.
 *
 * @par     History
 * 1.Date        :W31 Tue, 2016-08-02 21:21:30
 *   Author      :zhangxc
 *   Version     :v1.0.0
 *   Modification:Create file
 */

#include "xjson.h"
xt_string Xstrchr(xt_rostring s, xt_u8 c) {
  if (NULL == s) {
    return NULL;
  }

  while (*s) {
    if (*s == c) {
      return (xt_string)s;
    }

    s++;
  }

  return NULL;
}

xt_string Xstrpbrk(xt_rostring cs, xt_rostring ct) {
  xt_rostring sc1, sc2;

  for (sc1 = cs; *sc1 != '\0'; ++sc1) {
    for (sc2 = ct; *sc2 != '\0'; ++sc2) {
      if (*sc1 == *sc2) {
        return (xt_string) sc1;
      }
    }
  }

  return NULL;
}

xt_s32 Xstrspn(xt_rostring s, xt_rostring accept) {
  xt_rostring p;
  xt_rostring a;
  xt_s32 count = 0;

  for (p = s; *p != '\0'; ++p) {
    for (a = accept; *a != '\0'; ++a) {
      if (*p == *a) {
        ++count;
        break;
      }
    }

    if (*a == '\0') {
      return count;
    }
  }

  return count;
}
xt_string Xstrtok(xt_string s, xt_rostring delim, xt_string *save_ptr) {
  xt_string token;

  if (s == NULL) {
    s = *save_ptr;
  }

  if (s == NULL) {
    return NULL;
  }

  /* Scan leading delimiters.  */
  // Xprintf("Xstrspn s:%p\n", s);
  s += Xstrspn(s, delim);

  if (*s == '\0') {
    return NULL;
  }

  /* Find the end of the token.  */
  token = s;
  // Xprintf("Xstrpbrk\n");
  s = (xt_string)Xstrpbrk(token, delim);

  if (s == NULL) {
    /* This token finishes the string.  */
    // Xprintf("Xstrchr\n");
    *save_ptr = (xt_string)Xstrchr(token, '\0');
  } else {
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *s = '\0';
    *save_ptr = s + 1;
  }

  // Xprintf("token retun\n");
  return token;
}
static xt_s32 _XjsonArrayGetIntIndex(xt_json object, xt_s32 value) {
  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr) {
        if (cptr->type != cJSON_Number) {
          return XERROR;
        }

        if (cptr->valueint == value) {
          return index;
        }

        cptr = cptr->next;
        index++;
      }
    }
  }

  return XERROR;
}

static xt_s32 _XjsonArrayGetDoubleIndex(xt_json object, xt_double value) {
  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr) {
        if (cptr->type != cJSON_Number) {
          return XERROR;
        }

        if (cptr->valuedouble == value) {
          return index;
        }

        cptr = cptr->next;
        index++;
      }
    }
  }

  return XERROR;
}

static xt_s32 _XjsonArrayGetStringIndex(xt_json object, xt_rostring value) {
  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr) {
        if (cptr->type != cJSON_String) {
          return XERROR;
        }

        if (strcmp(cptr->valuestring, value) == 0) {
          return index;
        }

        cptr = cptr->next;
        index++;
      }
    }
  }

  return XERROR;
}

static xt_s32 _XjsonArrayGetJsonIndex(xt_json object, xt_json value) {
  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;
      xt_string value_string;
      xt_string child_string;
      while (cptr) {
        if (cptr->type != cJSON_Object) {
          return XERROR;
        }

        value_string = XjsonToString(value);
        child_string = XjsonToString(cptr);
        if (strcmp(value_string, child_string) == 0) {
          free(value_string);
          free(child_string);
          return index;
        }
        free(value_string);
        free(child_string);

        cptr = cptr->next;
        index++;
      }
    }
  }

  return XERROR;
}


// if key is "a.b.c.d", then parent_path is "a.b.c", key_name is "d"
xt_s32 XjsonParsePath(xt_rostring key,
                      xt_string parent_path, xt_s32 path_size,
                      xt_string key_name, xt_s32 key_size) {
  xt_string k = (xt_string)key;

  if (NULL == k || NULL == parent_path) {
    return XERROR;
  }

  memset(parent_path, 0, path_size);

  if (key_name) {
    memset(key_name, 0, key_size);
  }

  xt_string first = strchr((xt_string)k, '.');
  xt_string delim = strrchr((xt_string)k, '.');

  if (NULL == delim ||  // Not found "."
      first == k ||  // "." is a first char
      delim == k + strlen((xt_rostring)k) - 1) {  // "." is a last char
    return XERROR;
  }

  snprintf((xt_string)parent_path, (xt_s32)(delim - k) + 1, "%s", k);

  if (key_name) {
    snprintf((xt_string)key_name, key_size, "%s", delim + 1);
  }

  return XOK;
}

xt_json XjsonSearch(xt_json root, xt_rostring path) {
  xt_s8 path_copy[XJSON_MAX_PATH_LENGTH];
  xt_s8 *p, *q;
  xt_rostring delim = ".";
  xt_json obj, container;
  xt_string save;
  snprintf(path_copy, sizeof(path_copy), "%s", path);
  p = path_copy;
  container = root;

  if (NULL == path) {
    return container;
  }

  q = Xstrtok(p, delim, &save);

  while (q) {
    obj = cJSON_GetObjectItem(container, q);

    if (obj) {
      container = obj;
    } else {
      return NULL;
    }

    q = Xstrtok(NULL, delim, &save);
  }

  return container;//obj;
}

xt_s32 XjsonSetJson(xt_json root, xt_rostring path, xt_json json) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Object) {
      return XERROR;
    } else {

      object->child = json;
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  cJSON_AddItemToObject(parent, node, json);
  return XOK;
}


xt_json XjsonSearchParent(xt_json root, xt_rostring path) {
  xt_s8 parent_path[XJSON_MAX_PATH_LENGTH];
  xt_s32 ret = XjsonParsePath(path, parent_path,
                              sizeof(parent_path), NULL, 0);

  if (XOK != ret) {
    return NULL;
  }

  return XjsonSearch(root, parent_path);
}

xt_json XjsonNewParent(xt_json root, xt_rostring path,
                       xt_string node, xt_s32 bufsize) {
  xt_string last_delim;
  xt_s8 path_copy[XJSON_MAX_PATH_LENGTH];
  xt_string p, q;
  xt_json obj, parent;
  xt_s32 new_object_start = 0;
  xt_string save;
  snprintf(path_copy, sizeof(path_copy), "%s", path);
  parent = root;
  p = path_copy;
  last_delim = strrchr(path_copy, '.');

  if (last_delim == NULL) {
    snprintf(node, bufsize, "%s", path_copy);
    return parent;
  }

  q = Xstrtok(p, ".", &save);

  while (q) {
    // last_delim is points to the '.' delimiter,
    // but q points to the node name
    if (q == last_delim + 1) {  // reach to the last node
      snprintf(node, bufsize, "%s", q);
      return parent;
    }

    if (!new_object_start) {
      obj = cJSON_GetObjectItem(parent, q);

      if (obj == NULL) {
        new_object_start = 1;
        obj = cJSON_CreateObject();

        if (obj == NULL) {
          return NULL;
        }

        cJSON_AddItemToObject(parent, q, obj);
      } else if (obj->type != cJSON_Object) {
        return NULL;
      }

      parent = obj;
    } else {
      obj = cJSON_CreateObject();

      if (obj == NULL) {
        return NULL;
      }

      cJSON_AddItemToObject(parent, q, obj);
      parent = obj;
    }

    q = Xstrtok(NULL, ".", &save);
  }

  return NULL;
}

xt_s32 XjsonGetInt(xt_json root, xt_rostring path,
                   xt_s32 *value, xt_s32 defvalue) {
  if (NULL == root || NULL == path || NULL == value) {
    *value = defvalue;
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Number &&
        object->type != cJSON_True &&
        object->type != cJSON_False) {
      *value = defvalue;
      return XERROR;
    } else {
      *value = object->valueint;
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    *value = defvalue;
    return XERROR;
  }

  cJSON_AddNumberToObject(parent, node, defvalue);
  *value = defvalue;
  return XOK;
}

xt_s32 XjsonArrayRemoveInt(xt_json root, xt_rostring path, xt_s32 value) {
  xt_s32 index = XjsonArrayGetIntIndex(root, path, value);

  if (index >= 0) {
    xt_json array = XjsonSearch(root, path);

    if (array) {
      cJSON_DeleteItemFromArray(array, index);
    }
  }

  return XOK;
}

xt_s32 XjsonArrayRemoveDouble(xt_json root, xt_rostring path, xt_double value) {
  xt_s32 index = XjsonArrayGetDoubleIndex(root, path, value);

  if (index >= 0) {
    xt_json array = XjsonSearch(root, path);

    if (array) {
      cJSON_DeleteItemFromArray(array, index);
    }
  }

  return XOK;
}


xt_s32 XjsonSetInt(xt_json root, xt_rostring path, xt_s32 value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Number) {
      return XERROR;
    } else {
      cJSON_SetIntValue(object, value);
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  cJSON_AddNumberToObject(parent, node, value);
  return XOK;
}

xt_s32 XjsonGetString(xt_json root, xt_rostring path,
                      xt_string value, xt_s32 bufsize, xt_rostring defvalue) {
  if (NULL == root || NULL == path || NULL == value || NULL == defvalue) {
    return XERROR_FUNC_ARGS;
  }

  memset(value, 0, bufsize);
  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_String) {
      snprintf(value, bufsize, "%s", defvalue);
      return XERROR;
    } else {
      snprintf(value, bufsize, "%s", object->valuestring);
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    snprintf(value, bufsize, "%s", defvalue);
    return XERROR;
  }

  cJSON_AddStringToObject(parent, node, defvalue);
  snprintf(value, bufsize, "%s", defvalue);
  return XOK;
}

xt_s32 XjsonSetString(xt_json root, xt_rostring path, xt_rostring value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_String) {
      return XERROR;
    } else {
      if (object->valuestring) {
        free(object->valuestring);
      }

      object->valuestring =(char *) cJSON_strdup((unsigned  char *)value,NULL);
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  cJSON_AddStringToObject(parent, node, value);
  return XOK;
}

xt_s32 XjsonArrayGetStringIndex(xt_json root, xt_rostring path, xt_rostring value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);
  return _XjsonArrayGetStringIndex(object, value);
}


xt_s32 XjsonArrayAppendString(xt_json root, xt_rostring path, xt_rostring value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      if (_XjsonArrayGetStringIndex(object, value) >= 0) {
          /* pass for array */
          //return XOK;

      }

      cJSON_AddItemToArray(object, cJSON_CreateString(value));
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  xt_json array = cJSON_CreateArray();

  if (NULL == array) {
    return XERROR;
  }

  cJSON_AddItemToArray(array, cJSON_CreateString(value));
  cJSON_AddItemToObject(parent, node, array);
  return XOK;
}

xt_s32 XjsonArrayGetString(xt_json root, xt_rostring path,
                           xt_string values[], xt_s32 maxsize) {
  if (NULL == root || NULL == path || NULL == values) {
    return XERROR_FUNC_ARGS;
  }

  // memset((xt_u8 *)values, 0, sizeof(values[0]) * maxsize);
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr && index < maxsize) {
        if (cptr->type != cJSON_String) {
          return XERROR;
        }

        values[index] = cptr->valuestring;
        cptr = cptr->next;
        index++;
      }

      return index;
    }
  }

  return XERROR;
}

xt_s32 XjsonArrayRemoveString(xt_json root, xt_rostring path, xt_rostring value) {
  xt_s32 index = XjsonArrayGetStringIndex(root, path, value);

  if (index >= 0) {
    xt_json array = XjsonSearch(root, path);

    if (array) {
      cJSON_DeleteItemFromArray(array, index);
    }
  }

  return XOK;
}


xt_s32 XjsonArrayAppendInt(xt_json root, xt_rostring path, xt_s32 value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      if (_XjsonArrayGetIntIndex(object, value) >= 0) {
        /* pass for array */
        //return XOK;
      }

      cJSON_AddItemToArray(object, cJSON_CreateNumber(value));
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  xt_json array = cJSON_CreateArray();

  if (NULL == array) {
    return XERROR;
  }

  cJSON_AddItemToArray(array, cJSON_CreateNumber(value));
  cJSON_AddItemToObject(parent, node, array);
  return XOK;
}


xt_s32 XjsonLength(xt_json root, xt_rostring path) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array && object->type != cJSON_Object) {
      return XERROR;
    } else {
      return cJSON_GetArraySize(object);
    }
  }

  return XERROR;
}


xt_s32 XjsonArrayGetIntIndex(xt_json root, xt_rostring path, xt_s32 value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);
  return _XjsonArrayGetIntIndex(object, value);
}

xt_s32 XjsonArrayGetInt(xt_json root, xt_rostring path,
                        xt_s32 values[], xt_s32 maxsize) {
  if (NULL == root || NULL == path || NULL == values) {
    return XERROR_FUNC_ARGS;
  }

  memset((xt_u8 *)values, 0, sizeof(values[0]) * maxsize);
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr && index < maxsize) {
        if (cptr->type != cJSON_Number) {
          return XERROR;
        }

        values[index] = cptr->valueint;
        cptr = cptr->next;
        index++;
      }

      return index;
    }
  }

  return XERROR;
}

xt_s32 XjsonKeysCount(xt_json root, xt_rostring path) {
  if (NULL == root) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Object) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr) {
        cptr = cptr->next;
        index++;
      }

      return index;
    }
  }

  return XERROR;
}

xt_s32 XjsonKeys(xt_json root, xt_rostring path,
                 xt_string keys[], xt_s32 maxsize) {
  if (NULL == root || NULL == keys) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Object) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr && index < maxsize) {
        keys[index] = cptr->string;
        cptr = cptr->next;
        index++;
      }

      return index;
    }
  }

  return XERROR;
}

xt_s32 XjsonRemove(xt_json root, xt_rostring path) {
  xt_s8 parent_path[XJSON_MAX_PATH_LENGTH];
  xt_s8 node_path[XJSON_MAX_PATH_LENGTH];

  if (NULL == root || NULL == path) {
    return XERROR;
  }

  xt_s32 ret = XjsonParsePath(path, parent_path, sizeof(parent_path),
                              node_path, sizeof(node_path));

  if (ret != XOK) {
    return XERROR;
  }

  xt_json parent = XjsonSearch(root, parent_path);

  if (NULL == parent) {
    return XOK;
  }

  cJSON_DeleteItemFromObject(parent, node_path);
  return XOK;
}

xt_s32 XjsonDump(xt_rostring msg, xt_json object) {
  xt_s32 ret = 0;
  xt_string objectStr = XjsonToString(object);

  //ret = Xprintf("%s: %s", msg, objectStr);

  if (objectStr) {
    free(objectStr);
  }

 // return ret;
 return 0;//zhang add 20200812
}

xt_s32 XjsonArrayGetJsonIndex(xt_json root, xt_rostring path, xt_json value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);
  return _XjsonArrayGetJsonIndex(object, value);    
}

xt_s32 XjsonArrayAppendJson(xt_json root, xt_rostring path, xt_json value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      //if (_XjsonArrayGetStringIndex(object, value) >= 0) {
      //  return XOK;
      //}

      cJSON_AddItemToArray(object, value);
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  xt_json array = cJSON_CreateArray();

  if (NULL == array) {
    return XERROR;
  }

  cJSON_AddItemToArray(array, value);
  cJSON_AddItemToObject(parent, node, array);
  return XOK;
}

xt_s32 XjsonArrayGetJson(xt_json root, xt_rostring path,
                           xt_json values[], xt_s32 maxsize) {
  if (NULL == root || NULL == path || NULL == values) {
    return XERROR_FUNC_ARGS;
  }

  // memset((xt_u8 *)values, 0, sizeof(values[0]) * maxsize);
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr && index < maxsize) {
        if (cptr->type != cJSON_Object) {
          return XERROR;
        }

        values[index] = cptr;
        cptr = cptr->next;
        index++;
      }

      return index;
    }
  }

  return XERROR;
}

xt_s32 XjsonArrayRemoveJson(xt_json root, xt_rostring path, xt_json value) {
  xt_s32 index = XjsonArrayGetJsonIndex(root, path, value);

  if (index >= 0) {
    xt_json array = XjsonSearch(root, path);

    if (array) {
      cJSON_DeleteItemFromArray(array, index);
    }
  }

  return XOK;
}

xt_s32 XjsonArrayGetDoubleIndex(xt_json root, xt_rostring path, xt_double value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_json object = XjsonSearch(root, path);
  return _XjsonArrayGetDoubleIndex(object, value);
}

xt_s32 XjsonArrayAppendDouble(xt_json root, xt_rostring path, xt_double value) {
  if (NULL == root || NULL == path) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      if (_XjsonArrayGetDoubleIndex(object, value) >= 0) {
          /* pass for array */
          //return XOK;

      }

      cJSON_AddItemToArray(object, cJSON_CreateNumber(value));
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  xt_json array = cJSON_CreateArray();

  if (NULL == array) {
    return XERROR;
  }

  cJSON_AddItemToArray(array, cJSON_CreateNumber(value));
  cJSON_AddItemToObject(parent, node, array);
  return XOK;
}

xt_s32 XjsonArrayGetDouble(xt_json root, xt_rostring path,
                        xt_double values[], xt_s32 maxsize) {
  if (NULL == root || NULL == path || NULL == values) {
    return XERROR_FUNC_ARGS;
  }

  memset((xt_double *)values, 0, sizeof(values[0]) * maxsize);
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Array) {
      return XERROR;
    } else {
      xt_s32 index = 0;
      xt_json cptr = object->child;

      while (cptr && index < maxsize) {
        if (cptr->type != cJSON_Number) {
          return XERROR;
        }

        values[index] = cptr->valuedouble;
        cptr = cptr->next;
        index++;
      }

      return index;
    }
  }

  return XERROR;
}


xt_s32 XjsonSetDouble(xt_json root, xt_rostring path, xt_double value) {
    if (NULL == root || NULL == path) {
      return XERROR_FUNC_ARGS;
    }
    
    xt_s8 node[XJSON_MAX_PATH_LENGTH];
    xt_json object = XjsonSearch(root, path);
    
    if (object) {
      if (object->type != cJSON_Number) {
        return XERROR;
      } else {
        cJSON_SetIntValue(object, value);
        return XOK;
      }
    }
    
    xt_json parent = XjsonNewParent(root, path, node, sizeof(node));
    
    if (!parent || parent->type != cJSON_Object) {
      return XERROR;
    }
    
    cJSON_AddNumberToObject(parent, node, value);
    return XOK;

}

xt_s32 XjsonGetDouble(xt_json root, xt_rostring path,
                            xt_double *value, xt_double defvalue) {
  if (NULL == root || NULL == path || NULL == value) {
    return XERROR_FUNC_ARGS;
  }

  xt_s8 node[XJSON_MAX_PATH_LENGTH];
  xt_json object = XjsonSearch(root, path);

  if (object) {
    if (object->type != cJSON_Number &&
        object->type != cJSON_True &&
        object->type != cJSON_False) {
      *value = defvalue;
      return XERROR;
    } else {
      *value = object->valuedouble;
      return XOK;
    }
  }

  xt_json parent = XjsonNewParent(root, path, node, sizeof(node));

  if (!parent || parent->type != cJSON_Object) {
    return XERROR;
  }

  cJSON_AddNumberToObject(parent, node, defvalue);
  *value = defvalue;
  return XOK;
}

//通过json下标来获取key和value                            
xt_s32 XjsonGetKeyAndIntByIndex(xt_json root, xt_rostring path, xt_s32 index,
                           xt_string key, xt_s32 key_size, xt_s32 *value) {
  if (NULL == root || NULL == path ||  NULL == key || NULL == value) {
    return XERROR_FUNC_ARGS;
  }

  // memset((xt_u8 *)values, 0, sizeof(values[0]) * maxsize);
  xt_json object = XjsonSearch(root, path);


    if (object && object->type == cJSON_Object) {
      xt_json cptr = object->child;

      while (index != 0) {
        if (!cptr) {
            return XERROR;
        }
        index--;
        cptr = cptr->next;
        
      }

      if (cptr) {
        if (cptr->type != cJSON_Number) {
          return XERROR;
        } else {
          snprintf(key, key_size, "%s", cptr->string);
          *value = cptr->valueint;
          return XOK;
        }
      }
      return XERROR;
    }

  return XERROR;
}


