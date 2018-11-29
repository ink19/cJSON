#include "cjson.h"

static cjson_return_code_t cjson_read_begin(const char *json_string, int *json_string_cursor, cjson_item_t *result);

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
        if(!memcmp((json_string + *json_string_cursor), "true", sizeof(char) * 4)) {
            data_p->data = 1;
            *(json_string_cursor) += 4;
        } else {
            error_code = 1;
        }
    } else {
        if(!memcmp((json_string + *json_string_cursor), "false", sizeof(char) * 5)) {
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

static cjson_return_code_t cjson_read_string_escape(char *string_unescape) {
    int loop_i, loop_j;
    loop_i = loop_j = 0;
    while(*(loop_i + string_unescape)) {
        if(*(loop_i + string_unescape) == '\\') {
            switch(*(loop_i + string_unescape + 1)) {
                case 'a': *(loop_i + string_unescape + 1) = '\a'; break;
                case 'b': *(loop_i + string_unescape + 1) = '\b'; break;
                case 'f': *(loop_i + string_unescape + 1) = '\f'; break;
                case 'n': *(loop_i + string_unescape + 1) = '\n'; break;
                case 'r': *(loop_i + string_unescape + 1) = '\r'; break;
                case 't': *(loop_i + string_unescape + 1) = '\t'; break;
                case '\\': *(loop_i + string_unescape + 1) = '\\'; break;
                case '"': *(loop_i + string_unescape + 1) = '"'; break;
                default: goto escape_not_match; 
            }
            loop_i++;
        }
        escape_not_match:;
        *(string_unescape + loop_j) = *(loop_i + string_unescape);
        loop_i++;
        loop_j++;
    }
    string_unescape[loop_j] = 0;
    return CJSON_OK;
}

static cjson_return_code_t cjson_read_string(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    int string_length;
    cjson_string_t *data_p = (cjson_string_t *) malloc(sizeof(cjson_string_t));
    char *temp_string;
    ++(*json_string_cursor);
    while(*(string_length + json_string + *json_string_cursor) != '"') {
        if(*(string_length + json_string + *json_string_cursor) == '\\') {
            string_length++;
        }
        string_length++;
    }
    *json_string_cursor += string_length + 1;
    temp_string = (char *)malloc(sizeof(char) * (string_length + 1));
    memcpy(temp_string, json_string + *json_string_cursor, sizeof(char) * string_length);
    temp_string[string_length] = 0;
    cjson_read_string_escape(temp_string);
    data_p->data = temp_string;
    data_p->length = strlen(temp_string);
    result->data_p = (void *)data_p;
    result->type = CJSON_STRING;
    return CJSON_OK;
}

static cjson_return_code_t cjson_read_set(const char *json_string, int *json_string_cursor, cjson_item_t *result);

static cjson_return_code_t cjson_read_object(const char *json_string, int *json_string_cursor, cjson_item_t *result);

static cjson_return_code_t cjson_read_begin(const char *json_string, int *json_string_cursor, cjson_item_t* result) {
    while(isspace(*(json_string + *json_string_cursor))) ++(*json_string_cursor);
    
}

extern int cjson_decode(const char *json_string, cjson_item_t *json_object) {

} 