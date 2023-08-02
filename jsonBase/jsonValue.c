#include "jsonValue.h"
#include "stdio.h"

json_value_t *json_value_parse(const char *jsonStr)
{
    json_value_t *value = NULL;
    int ret;

    value = (json_value_t *)malloc(sizeof(json_value_t));
    if(value == NULL)
    {
        return NULL;
    }

    if (jsonStr == NULL)
    {
        free(value);
        value = NULL;
        return NULL;
    }

    while (isspace(*jsonStr))
    {
        jsonStr++;
    }

    ret = __parser_json_value(jsonStr, value);
    if (ret < 0)
    {
        return NULL;
    }

    return value;
}

int __parser_json_value(const char *jsonStr, json_value_t *value)
{
    int ret = -1;

    return ret;
}