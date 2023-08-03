#include "jsonValue.h"
#include "stdio.h"

json_value_t *json_value_parse(const char *jsonStr)
{
    json_value_t *value = NULL;
    int ret;

    value = (json_value_t *)malloc(sizeof(json_value_t));
    if (value == NULL)
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

    if (jsonStr == NULL || value == NULL)
    {
        return ret;
    }

    while (isspace(*jsonStr))
    {
        jsonStr++;
    }

    switch (*jsonStr)
    {
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        ret = __parser_json_number(jsonStr, value);
        if (ret < 0)
        {
            return -2;
        }

        value->type = JSON_VALUE_NUMBER;
        break;
    }
    case '\"':
    {
        ret = __parser_json_string(jsonStr, value);
        if (ret < 0)
        {
            return -2;
        }

        value->type = JSON_VALUE_STRING;
        break;
    }
    case 't':
    {
        if (strncmp(jsonStr, "true", 4) != 0)
            return -2;

        value->type = JSON_VALUE_TRUE;
        jsonStr += 4;
        break;
    }
    case 'f':
    {
        if (strncmp(jsonStr, "false", 5) != 0)
            return -2;

        value->type = JSON_VALUE_FALSE;
        jsonStr += 5;
        break;
    }
    case '[':
    {
        ret = __parser_json_array(jsonStr, value);
        if (ret < 0)
        {
            return -2;
        }

        value->type = JSON_VALUE_ARRAY;
        break;
    }
    case '{':
    {
        ret = __parser_json_object(jsonStr, value);
        if (ret < 0)
        {
            return -2;
        }

        value->type = JSON_VALUE_OBJECT;
        break;
    }
    case 'n':
    {
        if (strncmp(jsonStr, "null", 4) != 0)
            return -2;

        value->type = JSON_VALUE_NULL;
        jsonStr += 4;
        break;
    }
    default:
        return -2;
    }

    return 0;
}