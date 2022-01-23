#include <stdio.h>
#include <string.h>
#include "trash_can.h"

int main(int argc, char *argv[]) {
    if (strcmp(argv[1], "-r") == 0) {
        printf("restoring %s\n", argv[2]);
        restore_file(argv[2]);
    } else if (strcmp(argv[1], "-l") == 0) {
        list_content();
    } else {
        add_file_to_trash(argv[1]);
        auto_clear();
    }
    
}
