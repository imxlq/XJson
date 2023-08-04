#include "jsonValue.h"
#include "stdio.h"

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

static int __parse_json_value(const char *cursor, const char **end, json_value_t *val);
static int __parse_json_number(const char *cursor, const char **end, double *num);
static int __json_string_length(const char *cursor);
static int __parse_json_string(const char *cursor, const char **end, char *str);
static int __parse_json_array(const char *cursor, const char **end, json_value_t *val);
static int __parse_json_object(const char *cursor, const char **end, json_value_t *val);

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

    ret = __parse_json_value(jsonStr, &jsonStr, value);
    if (ret < 0)
    {
        return NULL;
    }

    return value;
}

static int __parse_json_value(const char *cursor, const char **end, json_value_t *val)
{
    int ret = -1;

    switch (*cursor)
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
        ret = __parse_json_number(cursor, end, &val->value.number);
        if (ret < 0)
        {
            return ret;
        }

        val->type = JSON_VALUE_NUMBER;
        break;
    }
    case '\"':
    {
        cursor++;
        ret = __parse_json_length(cursor);
        if (ret < 0)
        {
            return ret;
        }

        val->value.string = (char *)malloc(ret + 1);
        if (val->value.string == NULL)
        {
            return -1;
        }

        ret = __parse_json_string(cursor, end, val->value.string);
        if (ret < 0)
        {
            free(val->value.string);
            return ret;
        }

        val->type = JSON_VALUE_STRING;
        break;
    }
    case 't':
    {
        if (strncmp(cursor, "true", 4) != 0)
        {
            return -2;
        }

        *end = cursor + 4;
        val->type = JSON_VALUE_TRUE;

        break;
    }
    case 'f':
    {
        if (strncmp(cursor, "false", 5) != 0)
        {
            return -2;
        }

        *end = cursor + 5;
        val->type = JSON_VALUE_FALSE;
        break;
    }
    case '[':
    {
        ret = __parse_json_array(cursor, end, &val->value.array);
        if (ret < 0)
        {
            return ret;
        }

        val->type = JSON_VALUE_ARRAY;
        break;
    }
    case '{':
    {
        ret = __parse_json_object(cursor, val);
        if (ret < 0)
        {
            return ret;
        }

        val->type = JSON_VALUE_OBJECT;
        break;
    }
    case 'n':
    {
        if (strncmp(cursor, "null", 4) != 0)
        {
            return -2;
        }

        *end = cursor + 4;
        val->type = JSON_VALUE_NULL;
        break;
    }
    default:
        return -2;
    }

    return 0;
}

static int __parse_json_number(const char *cursor, const char **end, double *num)
{
    const char *p = cursor;
    if (*p == '-')
    {
        p++;
    }

    if (*p == '0' && (isdigit(p[1]) || p[1] == 'X' || p[1] == 'x'))
    {
        return -2;
    }

    *num = strtod(cursor, (char **)end);
    if (cursor == *end)
    {
        return -2;
    }

    return 0;
}

static int __json_string_length(const char *cursor)
{
    int len = 0;

    while (1)
    {
        if (*cursor == '\"')
            break;

        if (*(const unsigned char *)cursor < ' ')
        {
            return -2;
        }

        cursor++;
        if (cursor[-1] == '\\')
        {
            if (!*cursor)
            {
                return -2;
            }

            cursor++;
        }

        len++;
    }

    return len;
}

static int __json_string_length(const char *cursor)
{
    int len = 0;

    while (1)
    {
        if (*cursor == '\"')
            break;

        if (*(const unsigned char *)cursor < ' ')
        {
            return -2;
        }

        cursor++;
        if (*cursor == '\\')
        {
            cursor++;
        }

        len++;
    }

    return len;
}

static int __parse_json_string(const char *cursor, const char **end, char *str)
{
    int ret;

    while (*cursor != '\"')
    {
        if (*cursor == '\\')
        {
            cursor++;
            switch (*cursor)
            {
            case '\"':
                *str = '\"';
                break;
            case '\\':
                *str = '\\';
                break;
            case '/':
                *str = '/';
                break;
            case 'b':
                *str = '\b';
                break;
            case 'f':
                *str = '\f';
                break;
            case 'n':
                *str = '\n';
                break;
            case 'r':
                *str = '\r';
                break;
            case 't':
                *str = '\t';
                break;
            case 'u':
                cursor++;
                ret = __parse_json_unicode(cursor, &cursor, str);
                if (ret < 0)
                    return ret;

                str += ret;
                continue;

            default:
                return -2;
            }
        }
        else
        {
            *str = *cursor;
        }

        cursor++;
        str++;
    }

    *str = '\0';
    *end = cursor + 1;
    return 0;
}