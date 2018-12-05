#include "cjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
    FILE *file_p = fopen("test.data", "r");
    char data[1024];
    cjson_item_t rdata;
    cjson_err_t error;
    fread(data, sizeof(char), 1024, file_p);
    if(CJSON_OK != cjson_decode(data, &rdata, &error)) {
        printf("line:%d, word:%d\n", error.line, error.word_in_line);
    }
    
    cjson_print_data(&rdata, 0);
    cjson_destroy(&rdata);
    fclose(file_p);
    return 0;
}