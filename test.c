#include "cjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
    FILE *file_p = fopen("test.data", "r");
    char data[1024];
    cjson_item_t rdata;
    fread(data, sizeof(char), 1024, file_p);
    cjson_decode(data, &rdata);
    cjson_print_data(&rdata, 0);
    return 0;
}