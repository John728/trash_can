#include <stdio.h>
#include <string.h>
#include "trash_can.h"

int main(int argc, char *argv[]) {
    //init_trash();
    auto_clear();
    return 1;
    if (strcmp(argv[1], "-l") == 0) {
        list_content();
    } else {
        add_file_to_trash(argv[1]);
    }
}
