#ifndef __INK19_CJSON_H__
#define __INK19_CJSON_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <math.h>


//所有非set和map的数据，都需要在一开始对item赋予CJSON_PLACEHOLDER，在确认成功返回后再修改为对应的类型，防止发生错误时对数据重复释放
enum cjson_type {
    CJSON_PLACEHOLDER, //防止释放未分配的内存
    CJSON_NUMBER,
    CJSON_STRING,
    CJSON_BOOLEAN,
    CJSON_SET,
    CJSON_MAP,
    CJSON_NULL,
    CJSON_HEAD
};

typedef enum cjson_return_code_t{
    CJSON_OK,
    CJSON_ERROR_FORMAT,
    CJSON_ERROR_MALLOC,
    CJSON_ERROR_TYPE_ERROR
} cjson_return_code_t;

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
} cjson_set_t;

//对象元素
typedef struct _cjson_map_item_t {
    cjson_item_t *key;
    cjson_item_t *value;
    struct _cjson_map_item_t *next;
} cjson_map_item_t;

//对象
typedef struct {
    cjson_map_item_t *data;
} cjson_map_t;

typedef struct {
    int line;
    int word_in_line;
    int word;
    cjson_return_code_t err_code;
} cjson_err_t;

extern int cjson_decode(const char *json_string, cjson_item_t *json_object, cjson_err_t *err_data);
extern int cjson_print_data(cjson_item_t *json_object, int tab);
extern int cjson_destroy(cjson_item_t *json_object);
extern long long cjson_get_integer(cjson_item_t *json_item);
extern double cjson_get_double(cjson_item_t *json_item);
extern char * cjson_get_string(cjson_item_t *json_item);
extern int cjson_get_boolean(cjson_item_t *json_item);
extern int cjson_get_item_type(cjson_item_t *json_item);

#ifdef __cplusplus
}
#endif

#endif