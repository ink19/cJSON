#include "cjson.h"

static cjson_return_code_t cjson_read_number(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    cjson_number_t *data_p = (cjson_number_t *)malloc(sizeof(cjson_number_t));
    char temp_c;
    long long data_integer = 0;
    int data_dot = 0;
    data_p->type = 0;
    while(1) {
        temp_c = *(json_string + *json_string_cursor);
        ++(*json_string_cursor);
        if(temp_c >= '0' && temp_c <= '9') {
            data_integer *= 10;
            data_integer += temp_c - '0';
            if(data_p->type) ++data_dot;
            continue;
        } 
        if(temp_c == '.') {
            data_p->type = 1;
            continue;
        }
        break;
    }

    --(*json_string_cursor);

    if(data_dot) {
        data_dot = pow(10, data_dot);
        data_p->data.cjson_number_double = data_integer/(double)data_dot;
    } else {
        data_p->data.cjson_number_integer = data_integer;
    }
    
    result->data_p = data_p;
    result->type = CJSON_NUMBER;
    return CJSON_OK;
}

static cjson_return_code_t cjson_read_boolean(const char *json_string, int *json_string_cursor, cjson_item_t *result){
    cjson_boolean_t *data_p = (cjson_number_t *)malloc(sizeof(cjson_number_t));
    int error_code = 0;
    if('t' == *(json_string + *json_string_cursor)) {
        if(!memcmp(json_string + *json_string_cursor), "true", sizeof(char) * 4)) {
            data_p->data = 1;
            *(json_string_cursor) += 4;
        } else {
            error_code = 1;
        }
    } else {
        if(!memcmp(json_string + *json_string_cursor), "false", sizeof(char) * 5)) {
            data_p->data = 0;
            *(json_string_cursor) += 5;
        } else {
            error_code = 1;
        }
    }

    if(error_code) {
        free(data_p);
        return CJSON_ERROR_FORMAT;
    } else {
        result->data_p = (void *)data_p;
        result->type = CJSON_BOOLEAN;
        return CJSON_OK;
    }
}

static cjson_return_code_t cjson_read_null(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    if(!memcmp(json_string + *json_string_cursor, "null", 4 * sizeof(char))) {
        result->type = CJSON_NULL;
        *json_string_cursor += 4;
        return CJSON_OK;
    } else {
        return CJSON_ERROR_FORMAT;
    }
} 



extern int cjson_decode(const char *json_string, cjson_item_t *json_object) {

}