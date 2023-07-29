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

typedef struct __json_member json_member_t;
typedef struct __json_element json_element_t;

struct __json_value
{
    int type;
    union
    {
        char *string;
        double number;
        json_object_t object;
        json_array_t array;
    } value;
};

struct __json_array
{
    int size;

    json_element_t *head;
};

struct __json_element
{
    json_value_t value;
    json_element_t *next, *prev;
};

struct __json_object
{
    int size;

    struct list_head head;
    struct rb_root root;
};

struct __json_member
{
    struct list_head list;
    struct rb_node node;
    char key[1];
    json_value_t value;
};

#endif // __JSON_VALUE_H__