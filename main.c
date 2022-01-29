#include <stdio.h>
#include <string.h>
#include "trash_can.h"

int error_message();

int main(int argc, char *argv[]) {
 
    if (argc == 1) {
        error_message();
        return 0;
    }

    //---
    // Restore file
    //---
    if (strcmp(argv[1], "-r") == 0) {
        for (int i = 2; i < argc ; i++) {
            printf("restoring %s\n", argv[i]);
            restore_file(argv[i]);
        }
 
    //---
    // Restore all files
    //---
    } else if (strcmp(argv[1], "-R") == 0) {
        restore_all();
    
    //---
    // Clear bin
    //---
    } else if (strcmp(argv[1], "-C") == 0) {
        clear_trash();
    
    //---
    // Remove file from trash
    //---
    } else if (strcmp(argv[1], "-c") == 0) {
        if (argc == 3) {
            remove_file_from_trash(argv[2]);
        } else {
            error_message();
        }

    //---
    // List files
    //---
    } else if (strcmp(argv[1], "-l") == 0) {
        list_content();

    //---
    // Remove file
    //---
    } else {
        for (int i = 1; i < argc ; i++) {
            add_file_to_trash(argv[i]);
        }
        auto_clear();
    }
}

int error_message() {
    printf("Incorrect format, type del --help for more\n");
    return 0;
}
