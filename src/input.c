#include "input.h"
#include "error.h"
#include <stdio.h>

String readLineStdin(int max_len) {
    String str = string_new();

    int ch;

    while ((ch = fgetc(stdin) != EOF) && ch != '\n') {
        if (max_len != -1 && (int)str.len >= max_len) {
            set_error(Error_InvalidInput);
            string_free(&str);
            break;
        }

        string_push_char(&str, ch);

        if (get_error()) {
            string_free(&str);
            break;
        }
    }

    return str;
}
