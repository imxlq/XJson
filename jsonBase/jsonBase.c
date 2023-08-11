#include "jsonBase.h"
#include "stdio.h"
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "list.h"
#include "rbtree.h"

#define JSON_DEPTH_LIMIT 1020

typedef struct __json_element json_element_t;
typedef struct __json_member json_member_t;

struct __json_array
{
    int size;

    struct list_head head;
};

struct __json_object
{
    int size;

    struct list_head head;
    struct rb_root root;
};

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

struct __json_element
{
    struct list_head list;
    json_value_t value;
};

struct __json_member
{
    struct list_head list;
    struct rb_node node;
    json_value_t value;
    char name[1];
};

static int __parse_json_value(const char *cursor, const char **end, int depth, json_value_t *val);
static int __parse_json_number(const char *cursor, const char **end, double *num);
static int __json_string_length(const char *cursor);
static int __parse_json_string(const char *cursor, const char **end, char *str);
static int __parse_json_array(const char *cursor, const char **end, int depth, json_array_t *array);
static int __parse_json_object(const char *cursor, const char **end, int depth, json_object_t *obj);
static int __parse_json_hex4(const char *cursor, const char **end, unsigned int *code);
static int __parse_json_unicode(const char *cursor, const char **end, char *utf8);
static int __parse_json_elements(const char *cursor, const char **end, int depth, json_array_t *arr);
static int __parse_json_members(const char *cursor, const char **end, int depth, json_object_t *obj);
static int __parse_json_member(const char *cursor, const char **end, int depth, json_member_t *memb);
static int __insert_json_member(json_member_t *memb, struct list_head *pos, json_object_t *obj);
static void __destroy_json_elements(json_array_t *arr);
static void __destroy_json_value(json_value_t *val);
static void __destroy_json_members(json_object_t *obj);

static void __print_json_string(const char *str);
static void __print_json_number(double num);
static void __print_json_array(json_array_t *arr, int depth);
static void __print_json_object(json_object_t *obj, int depth);

json_value_t *json_value_parse(const char *jsonStr)
{
    json_value_t *value = NULL;
    int ret;

    value = (json_value_t *)malloc(sizeof(json_value_t));
    if (value == NULL)
    {
        return NULL;
    }

    while (isspace(*jsonStr))
    {
        jsonStr++;
    }

    ret = __parse_json_value(jsonStr, &jsonStr, 0, value);
    if (ret >= 0)
    {
        while (isspace(*jsonStr))
        {
            jsonStr++;
        }

        if (*jsonStr)
        {
            __destroy_json_value(value);
            ret = -2;
        }
    }
    if (ret < 0)
    {
        free(value);
        return NULL;
    }

    return value;
}

static int __parse_json_value(const char *cursor, const char **end, int depth, json_value_t *val)
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
        ret = __json_string_length(cursor);
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
        cursor++;
        ret = __parse_json_array(cursor, end, depth, &val->value.array);
        if (ret < 0)
        {
            return ret;
        }

        val->type = JSON_VALUE_ARRAY;
        break;
    }
    case '{':
    {
        cursor++;
        ret = __parse_json_object(cursor, end, depth, &val->value.object);
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

static int __parse_json_hex4(const char *cursor, const char **end, unsigned int *code)
{
    int hex;
    int i;

    // 初始化code
    *code = 0;
    for (i = 0; i < 4; i++)
    {
        hex = *cursor;
        if (hex >= '0' && hex <= '9')
        {
            hex = hex - '0';
        }
        else if (hex >= 'A' && hex <= 'F')
        {
            hex = hex - 'A' + 10;
        }
        else if (hex >= 'a' && hex <= 'f')
        {
            hex = hex - 'a' + 10;
        }
        else
        {
            return -2;
        }

        *code = (*code << 4) + hex;
        cursor++;
    }

    *end = cursor;
    return 0;
}

static int __parse_json_unicode(const char *cursor, const char **end, char *utf8)
{
    int ret;
    unsigned int code;
    unsigned int next;

    ret = __parse_json_hex4(cursor, end, &code);
    if (ret < 0)
    {
        return ret;
    }

    if (code >= 0xdc00 && code <= 0xdfff)
    {
        return -2;
    }

    if (code >= 0xd800 && code <= 0xdbff)
    {
        cursor = *end;
        if (*cursor != '\\')
        {
            return -2;
        }

        cursor++;
        if (*cursor != 'u')
        {
            return -2;
        }

        cursor++;
        ret = __parse_json_hex4(cursor, end, &next);
        if (ret < 0)
        {
            return ret;
        }

        if (next < 0xdc00 || next > 0xdfff)
        {
            return -2;
        }

        code = (((code & 0x3ff) << 10) | (next & 0x3ff)) + 0x10000;
    }

    if (code <= 0x7f)
    {
        utf8[0] = code;
        return 1;
    }
    else if (code <= 0x7ff)
    {
        utf8[0] = 0xc0 | (code >> 6);
        utf8[1] = 0x80 | (code & 0x3f);
        return 2;
    }
    else if (code <= 0xffff)
    {
        utf8[0] = 0xe0 | (code >> 12);
        utf8[1] = 0x80 | ((code >> 6) & 0x3f);
        utf8[2] = 0x80 | (code & 0x3f);
        return 3;
    }
    else
    {
        utf8[0] = 0xf0 | (code >> 18);
        utf8[1] = 0x80 | ((code >> 12) & 0x3f);
        utf8[2] = 0x80 | ((code >> 6) & 0x3f);
        utf8[3] = 0x80 | (code & 0x3f);
        return 4;
    }
}

static int __parse_json_array(const char *cursor, const char **end, int depth, json_array_t *arr)
{
    int ret;

    if (depth == JSON_DEPTH_LIMIT)
    {
        return -3;
    }

    INIT_LIST_HEAD(&arr->head);

    ret = __parse_json_elements(cursor, end, depth + 1, arr);
    if (ret < 0)
    {
        __destroy_json_elements(arr);
        return ret;
    }

    arr->size = ret;
    return 0;
}

static int __parse_json_elements(const char *cursor, const char **end, int depth, json_array_t *arr)
{
    int ret;
    int cnt = 0;
    json_element_t *elem;

    while (isspace(*cursor))
    {
        cursor++;
    }

    if (*cursor == ']')
    {
        *end = cursor + 1;
        return 0;
    }

    while (1)
    {
        elem = (json_element_t *)malloc(sizeof(json_element_t));
        if (elem == NULL)
        {
            return -1;
        }

        ret = __parse_json_value(cursor, &cursor, depth, &elem->value);
        if (ret < 0)
        {
            free(elem);
            return ret;
        }

        list_add_tail(&elem->list, &arr->head);
        cnt++;

        while (isspace(*cursor))
        {
            cursor++;
        }

        if (*cursor == ',')
        {
            cursor++;
            while (isspace(*cursor))
            {
                cursor++;
            }
        }
        else if (*cursor == ']')
        {
            break;
        }
        else
        {
            return -2;
        }
    }

    *end = cursor + 1;
    return cnt;
}

static int __parse_json_object(const char *cursor, const char **end, int depth, json_object_t *obj)
{
    int ret;

    if (depth == JSON_DEPTH_LIMIT)
    {
        return -3;
    }

    // 初始化链表和红黑树
    INIT_LIST_HEAD(&obj->head);
    obj->root.rb_node = NULL;

    ret = __parse_json_members(cursor, end, depth + 1, obj);
    if (ret < 0)
    {
        __destroy_json_members(obj);
        return ret;
    }

    obj->size = ret;
    return 0;
}

static int __parse_json_members(const char *cursor, const char **end, int depth, json_object_t *obj)
{
    int ret;
    int cnt = 0;
    json_member_t *memb;

    while (isspace(*cursor))
    {
        cursor++;
    }

    if (*cursor == '}')
    {
        *end = cursor + 1;
        return 0;
    }

    while (1)
    {
        if (*cursor != '\"')
        {
            return -2;
        }

        cursor++;
        ret = __json_string_length(cursor);
        if (ret < 0)
        {
            return ret;
        }

        memb = (json_member_t *)malloc(offsetof(json_member_t, name) + ret + 1);
        if (memb == NULL)
        {
            return -1;
        }

        ret = __parse_json_member(cursor, &cursor, depth, memb);
        if (ret < 0)
        {
            free(memb);
            return ret;
        }

        __insert_json_member(memb, obj->head.prev, obj);
        cnt++;

        while (isspace(*cursor))
        {
            cursor++;
        }

        if (*cursor == ',')
        {
            cursor++;
            while (isspace(*cursor))
            {
                cursor++;
            }
        }
        else if (*cursor == '}')
        {
            break;
        }
        else
        {
            return -2;
        }
    }

    *end = cursor + 1;
    return cnt;
}

static int __parse_json_member(const char *cursor, const char **end, int depth, json_member_t *memb)
{
    int ret;

    ret = __parse_json_string(cursor, &cursor, memb->name);
    if (ret < 0)
    {
        return ret;
    }

    while (isspace(*cursor))
    {
        cursor++;
    }

    if (*cursor != ':')
    {
        return -2;
    }

    cursor++;
    while (isspace(*cursor))
    {
        cursor++;
    }

    ret = __parse_json_value(cursor, &cursor, depth, &memb->value);
    if (ret < 0)
    {
        return ret;
    }

    *end = cursor;
    return 0;
}

static int __insert_json_member(json_member_t *memb, struct list_head *pos, json_object_t *obj)
{
    struct rb_node **p = &obj->root.rb_node;
    struct rb_node *parent = NULL;
    json_member_t *entry;

    while (*p)
    {
        parent = *p;
        entry = rb_entry(*p, json_member_t, node);
        if (strcmp(memb->name, entry->name) < 0)
        {
            p = &(*p)->rb_left;
        }
        else
        {
            p = &(*p)->rb_right;
        }
    }

    rb_link_node(&memb->node, parent, p);
    rb_insert_color(&memb->node, &obj->root);
    list_add(&memb->list, pos);
}

static void __destroy_json_elements(json_array_t *arr)
{
    struct list_head *pos, *tmp;
    json_element_t *elem;

    list_for_each_safe(pos, tmp, &arr->head)
    {
        elem = list_entry(pos, json_element_t, list);
        __destroy_json_value(&elem->value);
        free(elem);
    }
}

static void __destroy_json_value(json_value_t *val)
{
    switch (val->type)
    {
    case JSON_VALUE_STRING:
        free(val->value.string);
        break;
    case JSON_VALUE_ARRAY:
        __destroy_json_elements(&val->value.array);
        break;
    case JSON_VALUE_OBJECT:
        __destroy_json_members(&val->value.object);
        break;
    }
}

static void __destroy_json_members(json_object_t *obj)
{
    struct list_head *pos, *tmp;
    json_member_t *memb;

    list_for_each_safe(pos, tmp, &obj->head)
    {
        memb = list_entry(pos, json_member_t, list);
        __destroy_json_value(&memb->value);
        free(memb);
    }
}

void print_json_value(const json_value_t *val, int depth)
{
    switch (json_value_type(val))
    {
    case JSON_VALUE_STRING:
        __print_json_string(json_value_string(val));
        break;
    case JSON_VALUE_NUMBER:
        __print_json_number(json_value_number(val));
        break;
    case JSON_VALUE_ARRAY:
        __print_json_array(json_value_array(val), depth);
        break;
    case JSON_VALUE_OBJECT:
        __print_json_object(json_value_object(val), depth);
        break;
    case JSON_VALUE_TRUE:
        printf("true");
        break;
    case JSON_VALUE_FALSE:
        printf("false");
        break;
    case JSON_VALUE_NULL:
        printf("null");
        break;
    }

    if (depth == 0)
    {
        printf("\n");
    }
}

void json_value_destroy(json_value_t *val)
{
    __destroy_json_value(val);
    free(val);
}

int json_value_type(const json_value_t *val)
{
    return val->type;
}

const char *json_value_string(const json_value_t *val)
{
    if (json_value_type(val) != JSON_VALUE_STRING)
    {
        return NULL;
    }

    return val->value.string;
}

double json_value_number(const json_value_t *val)
{
    if (json_value_type(val) != JSON_VALUE_NUMBER)
    {
        return NAN;
    }

    return val->value.number;
}

json_array_t *json_value_array(const json_value_t *val)
{
    if (json_value_type(val) != JSON_VALUE_ARRAY)
    {
        return NULL;
    }

    return (json_array_t *)&val->value.array;
}

json_object_t *json_value_object(const json_value_t *val)
{
    if (json_value_type(val) != JSON_VALUE_OBJECT)
    {
        return NULL;
    }

    return (json_object_t *)&val->value.object;
}

static void __print_json_string(const char *str)
{
    printf("\"");

    while (*str)
    {
        switch (*str)
        {
        case '\r':
            printf("\\r");
            break;
        case '\n':
            printf("\\n");
            break;
        case '\t':
            printf("\\t");
            break;
        case '\b':
            printf("\\b");
            break;
        case '\f':
            printf("\\f");
            break;
        case '\\':
            printf("\\\\");
            break;
        case '\"':
            printf("\\\"");
            break;
        default:
            printf("%c", *str);
            break;
        }
        str++;
    }

    printf("\"");
}

static void __print_json_number(double num)
{
    long long integer = (long long)num;

    if (num == integer)
    {
        printf("%lld", integer);
    }
    else
    {
        printf("%lf", num);
    }
}

static void __print_json_array(json_array_t *arr, int depth)
{
    const json_value_t *val;
    int n = 0;
    int i;

    printf("[\n");
    json_array_for_each(val, arr)
    {
        if (n != 0)
        {
            printf(",\n");
        }

        n++;
        for (i = 0; i < depth + 1; i++)
        {
            printf("  ");
        }
        print_json_value(val, depth + 1);
    }

    printf("\n");
    for (i = 0; i < depth; i++)
    {
        printf("  ");
    }

    printf("]");
}

int json_array_size(const json_array_t *arr)
{
    return arr->size;
}

const json_value_t *json_array_next_value(const json_value_t *val, const json_array_t *arr)
{
    const struct list_head *pos;

    if (val)
    {
        pos = &list_entry(val, json_element_t, value)->list;
    }
    else
    {
        pos = &arr->head;
    }

    if (pos->next == &arr->head)
    {
        return NULL;
    }

    return &list_entry(pos->next, json_element_t, list)->value;
}

const json_value_t *json_array_prev_value(const json_value_t *val, const json_array_t *arr)
{
    const struct list_head *pos;

    if (val)
    {
        pos = &list_entry(val, json_element_t, value)->list;
    }
    else
    {
        pos = &arr->head;
    }

    if (pos->prev == &arr->head)
    {
        return NULL;
    }

    return &list_entry(pos->prev, json_element_t, list)->value;
}

static void __print_json_object(json_object_t *obj, int depth)
{
    const char *name;
    const json_value_t *val;
    int n = 0;
    int i;

    printf("{\n");
    json_object_for_each(name, val, obj)
    {
        if (n != 0)
        {
            printf(",\n");
        }

        n++;
        for (i = 0; i < depth + 1; i++)
        {
            printf("  ");
        }
        printf("\"%s\": ", name);
        print_json_value(val, depth + 1);
    }

    printf("\n");
    for (i = 0; i < depth; i++)
    {
        printf("  ");
    }

    printf("}");
}

const char *json_object_next_name(const char *name, const json_object_t *obj)
{
    const struct list_head *pos;

    if (name)
    {
        pos = &list_entry(name, json_member_t, name)->list;
    }
    else
    {
        pos = &obj->head;
    }

    if (pos->next == &obj->head)
    {
        return NULL;
    }

    return list_entry(pos->next, json_member_t, list)->name;
}

const char *json_object_prev_name(const char *name, const json_object_t *obj)
{
    const struct list_head *pos;

    if (name)
    {
        pos = &list_entry(name, json_member_t, name)->list;
    }
    else
    {
        pos = &obj->head;
    }

    if (pos->prev == &obj->head)
    {
        return NULL;
    }

    return list_entry(pos->prev, json_member_t, list)->name;
}

const json_value_t *json_object_next_value(const json_value_t *val, const json_object_t *obj)
{
    const struct list_head *pos;

    if (val)
    {
        pos = &list_entry(val, json_member_t, value)->list;
    }
    else
    {
        pos = &obj->head;
    }

    if (pos->next == &obj->head)
    {
        return NULL;
    }

    return &list_entry(pos->next, json_member_t, list)->value;
}

const json_value_t *json_object_prev_value(const json_value_t *val, const json_object_t *obj)
{
    const struct list_head *pos;

    if (val)
    {
        pos = &list_entry(val, json_member_t, value)->list;
    }
    else
    {
        pos = &obj->head;
    }

    if (pos->prev == &obj->head)
    {
        return NULL;
    }

    return &list_entry(pos->prev, json_member_t, list)->value;
}

const json_value_t *json_object_find(const char *name, const json_object_t *obj)
{
    struct rb_node *p = obj->root.rb_node;
    json_member_t *memb;
    int n;

    while (p)
    {
        memb = rb_entry(p, json_member_t, node);
        n = strcmp(name, memb->name);
        if (n < 0)
            p = p->rb_left;
        else if (n > 0)
            p = p->rb_right;
        else
            return &memb->value;
    }

    return NULL;
}