#pragma once
#ifndef __JSON_VALUE_H__
#define __JSON_VALUE_H__
#include "list.h"
#include "rbtree.h"

#define JSON_VALUE_STRING 1
#define JSON_VALUE_NUMBER 2
#define JSON_VALUE_OBJECT 3
#define JSON_VALUE_ARRAY 4
#define JSON_VALUE_TRUE 5
#define JSON_VALUE_FALSE 6
#define JSON_VALUE_NULL 7

typedef struct __json_value json_value_t;
typedef struct __json_object json_object_t;
typedef struct __json_array json_array_t;



json_value_t *json_value_parse(const char *jsonStr);



#endif // __JSON_VALUE_H__