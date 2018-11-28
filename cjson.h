#ifndef __INK19_CJSON_H__
#define __INK19_CJSON_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum cjson_type {
    CJSON_NUMBER,
    CJSON_STRING,
    CJSON_BOOLEAN,
    CJSON_SET,
    CJSON_OBJECT
};

//基本类型的抽象
typedef struct cjson_item_t {
    void *data_p;
    enum cjson_type type;
    struct cjson_item_t *next;
} cjson_item_t;

//数字，包含浮点和整数
typedef struct {
    union {
        double cjson_number_double;
        long long cjson_number_integer;
    } data;
    int type;
} cjson_number_t;


//字符串类型
typedef struct {
    char *data;
    int length;
} cjson_string_t;

//布尔型
typedef struct {
    int data;
} cjson_boolean_t;

//数组
typedef struct {
    cjson_item_t *data;
    int length;
} cjson_set_t;

//对象
typedef struct {
    cjson_item_t *key;
    cjson_item_t *value;
    int length;
} cjson_object_t;

extern int json_decode(const char *json_string, cjson_item_t *json_object);

#ifdef __cplusplus
}
#endif

#endif