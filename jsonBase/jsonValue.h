#pragma once

typedef enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct __json_value json_value_t;
typedef struct __json_object json_object_t;
typedef struct __json_array json_array_t;

struct __json_value
{
    JsonType type;
    union
    {
        char* string;
        double number;
        json_object_t object;
        json_array_t array;
    } value;
};

struct __json_object
{
    int size;
    char** keys;
    json_value_t* values;
};

struct __json_array
{
    int size;
    json_value_t* values;
};