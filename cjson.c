#include "cjson.h"

static cjson_return_code_t cjson_read_begin(const char *json_string, int *json_string_cursor, cjson_item_t *result);

static cjson_return_code_t cjson_read_number(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    cjson_number_t *data_p = (cjson_number_t *)malloc(sizeof(cjson_number_t));
    char temp_c;
    long long data_integer = 0;
    int data_dot = 0;
    int is_negative = 0;
    //判断是否是负数
    if(*(*json_string_cursor + json_string) == '-') {
        is_negative = 1;
        ++(*json_string_cursor);
    }  

    data_p->type = 0;
    while(1) {
        //暂存字符
        temp_c = *(json_string + *json_string_cursor);
        ++(*json_string_cursor);
        //判断是否为数字
        if(temp_c >= '0' && temp_c <= '9') {
            data_integer *= 10;
            data_integer += temp_c - '0';
            if(data_p->type) ++data_dot;
            continue;
        } 
        //判断是否为浮点数
        if(temp_c == '.') {
            data_p->type = 1;
            continue;
        }
        break;
    }

    --(*json_string_cursor);
    //负数处理
    if(is_negative) data_integer = -data_integer;
    //浮点数处理
    if(data_p->type) {
        data_dot = pow(10, data_dot);
        data_p->data.cjson_number_double = data_integer/(double)data_dot;
        //printf("%lf\n", data_p->data.cjson_number_double);
    } else {
        data_p->data.cjson_number_integer = data_integer;
    }
    //返回值
    result->data_p = data_p;
    result->type = CJSON_NUMBER;
    return CJSON_OK;
}

static cjson_return_code_t cjson_read_boolean(const char *json_string, int *json_string_cursor, cjson_item_t *result){
    cjson_boolean_t *data_p = (cjson_boolean_t *)malloc(sizeof(cjson_boolean_t));
    int error_code = 0;

    //判断为true或者false
    if('t' == *(json_string + *json_string_cursor)) {
        if(!memcmp((json_string + *json_string_cursor), "true", sizeof(char) * 4)) {
            data_p->data = 1;
            //光标移位
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
        //错误返回，释放资源
        free(data_p);
        return CJSON_ERROR_FORMAT;
    } else {
        result->data_p = (void *)data_p;
        result->type = CJSON_BOOLEAN;
        return CJSON_OK;
    }
}

static cjson_return_code_t cjson_read_null(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    //验证正确
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
    //循环到字符串结尾
    while(*(loop_i + string_unescape)) {
        if(*(loop_i + string_unescape) == '\\') {
            //判断是哪个转义字符，暂时不对其他转义处理
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
        //拷贝
        *(string_unescape + loop_j) = *(loop_i + string_unescape);
        loop_i++;
        loop_j++;
    }
    string_unescape[loop_j] = 0;
    return CJSON_OK;
}

static cjson_return_code_t cjson_read_string(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    int string_length = 0;
    cjson_string_t *data_p = (cjson_string_t *) malloc(sizeof(cjson_string_t));
    char *temp_string;
    ++(*json_string_cursor);
    //寻找结尾
    while(*(string_length + json_string + *json_string_cursor) != '"') {
        if(*(string_length + json_string + *json_string_cursor) == '\\') {
            string_length++;
        }
        string_length++;
    }
    //将光标移到字符串以外
    
    
    temp_string = (char *)malloc(sizeof(char) * (string_length + 1));
    memcpy(temp_string, json_string + *json_string_cursor, sizeof(char) * string_length);
    *json_string_cursor += string_length + 1;
    temp_string[string_length] = 0;

    //转义字符串
    cjson_read_string_escape(temp_string);
    //设定返回值
    data_p->data = temp_string;
    data_p->length = strlen(temp_string);
    result->data_p = (void *)data_p;
    result->type = CJSON_STRING;
    return CJSON_OK;
}

static cjson_return_code_t cjson_read_set(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    cjson_return_code_t return_code;
    cjson_item_t *set_head = (cjson_item_t *)malloc(sizeof(cjson_item_t));
    cjson_item_t *temp_node;
    cjson_set_t *set_object = (cjson_set_t *)malloc(sizeof(cjson_set_t));
    result->data_p = (void *)set_object;
    result->type = CJSON_SET;
    
    set_object->data = set_head;
    return_code = CJSON_OK;
    set_head->next = NULL;
    set_head->type = CJSON_HEAD;
    
    while(1) {
        //跳到下一个元素
        ++(*json_string_cursor);
        temp_node = (cjson_item_t *)malloc(sizeof(cjson_item_t));
        if((return_code = cjson_read_begin(json_string, json_string_cursor, temp_node)) != CJSON_OK) {
            break;
        }
        while(isspace(*(json_string + *json_string_cursor))) ++(*json_string_cursor);
        temp_node->next = set_head->next;
        set_head->next = temp_node;
        if(*(json_string + *json_string_cursor) == ',') {        
            continue;
        } else if(*(json_string + *json_string_cursor) == ']') {
            ++(*json_string_cursor);
            break;
        } else {
            return_code = CJSON_ERROR_FORMAT;
            break;
        }
    }
    return return_code;
}

static cjson_return_code_t cjson_read_object(const char *json_string, int *json_string_cursor, cjson_item_t *result) {
    cjson_return_code_t return_code = CJSON_OK;
    cjson_item_t *temp_item;
    // 初始化object的类型
    cjson_item_t *key_head = (cjson_item_t *)malloc(sizeof(cjson_item_t));
    cjson_item_t *value_head = (cjson_item_t *) malloc(sizeof(cjson_item_t));
    cjson_object_t *object_object = (cjson_object_t *)malloc(sizeof(cjson_object_t));
    object_object->key = key_head;
    object_object->value = value_head;
    key_head->type = CJSON_HEAD;
    value_head->type = CJSON_HEAD;
    key_head->next = NULL;
    value_head->next = NULL;
    //返回结果
    result->type = CJSON_OBJECT;
    result->data_p = (void *) object_object;

    //跳过object的开始{
    ++(*json_string_cursor);
    while(1) {
        //获取key，key只能为STRING类型
        temp_item = (cjson_item_t *)malloc(sizeof(cjson_item_t));
        cjson_read_begin(json_string, json_string_cursor, temp_item);
        if(temp_item->type != CJSON_STRING) {
            printf("error: key is not string %d\n", temp_item->type);
            return_code = CJSON_ERROR_FORMAT;
            break;
        }
        temp_item->next = key_head->next;
        key_head->next = temp_item;
        
        //判断下一个字符是否为:
        while(isspace(*(json_string + *json_string_cursor))) ++(*json_string_cursor);
        if(*(json_string + *json_string_cursor) != ':') {
            return_code = CJSON_ERROR_FORMAT;
            printf("error: %d\n", temp_item->type);
            break;
        }
        ++(*json_string_cursor);
        
        //获取value
        temp_item = (cjson_item_t *) malloc(sizeof(cjson_item_t));
        cjson_read_begin(json_string, json_string_cursor, temp_item);
        temp_item->next = value_head->next;
        value_head->next = temp_item;

        //判断下一个字符是否为,
        while(isspace(*(json_string + *json_string_cursor))) ++(*json_string_cursor);
        if(*(json_string + *json_string_cursor) == ',') {
            continue;
        }
        if(*(json_string + *json_string_cursor) == '}') {
            ++(*json_string_cursor);
            //printf("end\n");
            break;
        }
        return_code = CJSON_ERROR_FORMAT;
        break;
    }
    return return_code;
}

static cjson_return_code_t cjson_read_begin(const char *json_string, int *json_string_cursor, cjson_item_t* result) {
    cjson_return_code_t return_code;
    //寻找下一个非零字符
    while(isspace(*(json_string + *json_string_cursor))) ++(*json_string_cursor);

    //查表
    switch(*(json_string + *json_string_cursor)) {
        case '"': return_code = cjson_read_string(json_string, json_string_cursor, result); break;
        case 'n': return_code = cjson_read_null(json_string, json_string_cursor, result); break;
        case 't':
        case 'f': return_code = cjson_read_boolean(json_string, json_string_cursor, result); break;
        case '[': return_code = cjson_read_set(json_string, json_string_cursor, result); break;
        case '{': return_code = cjson_read_object(json_string, json_string_cursor, result); break;
        default : return_code = cjson_read_number(json_string, json_string_cursor, result); break;
    }
    return return_code;
}

extern int cjson_decode(const char *json_string, cjson_item_t *json_object) {
    int json_string_cursor = 0;
    return cjson_read_begin(json_string, &json_string_cursor, json_object);
} 

static int cjson_print_set(cjson_item_t *json_object, int tab) {
    int loop_i;
    while(json_object!= NULL) {
        for(loop_i = 0; loop_i < tab; ++loop_i) printf("  ");
        cjson_print_data(json_object, tab);
        printf("\n");
        json_object = json_object->next;
    }
    return 0;
}

static int cjson_print_object(cjson_object_t *json_object, int tab) {
    int loop_i;
    cjson_item_t *key_head = json_object->key->next;
    cjson_item_t *value_head = json_object->value->next;
    while(key_head != NULL) {
        for(loop_i = 0; loop_i < tab; ++loop_i) printf("  ");
        cjson_print_data(key_head, tab);
        printf(" -> ");
        cjson_print_data(value_head, tab);
        printf("\n");
        key_head = key_head->next;
        value_head = value_head->next;
    }
    return 0;
}

extern int cjson_print_data(cjson_item_t *json_object, int tab) {
    switch(json_object->type) {
        case CJSON_BOOLEAN:
            printf("boolean: ");
            printf(((cjson_boolean_t *)(json_object->data_p))->data?"True":"False");
        break;
        case CJSON_NUMBER:
            ((cjson_number_t *)(json_object->data_p))->type?
            printf("float: %lf", ((cjson_number_t *)(json_object->data_p))->data.cjson_number_double):
            printf("integer: %lld", ((cjson_number_t *)(json_object->data_p))->data.cjson_number_integer);
        break;
        case CJSON_NULL:
            printf("null");
        break;
        case CJSON_STRING:
            printf("string: %s", ((cjson_string_t *)(json_object->data_p))->data);
        break;
        case CJSON_SET:
            printf("set: \n");
            cjson_print_set(((cjson_set_t *)(json_object->data_p))->data->next, tab + 1);
        break;
        case CJSON_OBJECT:
            printf("object: \n");
            cjson_print_object(json_object->data_p, tab + 1);
        break;
        default: 
            printf("type: %d", json_object->type);
    }
    return 0;
}

static int cjson_set_destroy(cjson_set_t *json_set) {

}

static int cjson_object_destroy(cjson_object_t *json_object) {

}

extern int cjson_destroy(cjson_item_t *json_object) {
    switch(json_object->type) {
        case CJSON_BOOLEAN:
            free(json_object->data_p);
        break;
        case CJSON_NUMBER:
            free(json_object->data_p);
        break;
        case CJSON_STRING:
            free(((cjson_string_t *) (json_object->data_p))->data);
            free(json_object->data_p);
        break;
        case CJSON_SET:
            cjson_set_destroy((cjson_set_t *)(json_object->data_p));
            free(json_object->data_p);
        break;
        case CJSON_OBJECT:
            cjson_object_destroy((cjson_object_t *)(json_object->data_p));
            free(json_object->data_p);
        break;
    }
    return 0;
}