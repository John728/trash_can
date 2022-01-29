#include "trash_can.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//char *get_homedir_path();
int remove_file(char *path);
int remove_dot_files(char *file_name);
char *get_path_to_trash();

int init_trash() {
    printf("Creating files...\n");
    
    // Creates /.trash folder
    char *trash_path_arg = get_path_to_trash();
    
    pid_t trash_mkdir_pid;
    extern char **environ;
    char *mkdir_argv[] = {"/bin/mkdir", trash_path_arg, NULL};

    if (posix_spawn(&trash_mkdir_pid, "/bin/mkdir", NULL, NULL, mkdir_argv, environ) != 0) {
        perror("spawn");
        exit(1);
    }
    free(trash_path_arg);

    int exit_status;
    if (waitpid(trash_mkdir_pid, &exit_status, 0) == -1) {
        perror("waitpid");
        exit(1);
    }
    
    return 0;

    // Might delete all this

    // Create config file for trash_can
    char path_arg2[100] = {'\0'};
    //strcat(path_arg2, homedir);
    strcat(path_arg2, "/.config/trash_can");
    
    pid_t config_mkdir_pid;
    extern char **environ;
    char *mkdir_argv2[] = {"/bin/mkdir", path_arg2, NULL};

    if (posix_spawn(&config_mkdir_pid, "/bin/mkdir", NULL, NULL, mkdir_argv2, environ) != 0) {
        perror("spawn");
        exit(1);
    }

    // wait for spawned processes to finish
    if (waitpid(config_mkdir_pid, &exit_status, 0) == -1) {
        perror("waitpid");
        exit(1);
    }

    char path3[] = "touch ~/.config/trash_can/config";
     // Create config file for trash_can
    pid_t cofig_touch_pid;
    extern char **environ;
    char *mkdir_argv3[] = {"/bin/touch", path3, NULL};

    if (posix_spawn(&cofig_touch_pid, "/bin/touch", NULL, NULL, mkdir_argv3, environ) != 0) {
        perror("spawn");
        exit(1);
    }

    // wait for spawned processes to finish
    if (waitpid(cofig_touch_pid, &exit_status, 0) == -1) {
        perror("waitpid");
        exit(1);
    }

    int trash_can_size = 0;
    printf("At what size should the trash be taken out? (in bytes) (recomended: 1000): ");
    scanf("%d", &trash_can_size);

    int trash_can_length = 0;
    printf("How many days until a file in trash is taken out? (recomended: 30): ");
    scanf("%d", &trash_can_length);

    char open_path[100] = {'\0'};
    //strcat(open_path, homedir);
    strcat(open_path, "/.config/trash_can/config");

    FILE *config_file = fopen(open_path, "w");
    if (config_file == NULL) {
        perror(open_path);  // prints why the open failed
        return 1;
    }
    
    char str_length[(int)((ceil(log10(trash_can_length))+1)*sizeof(char))];
    sprintf(str_length, "%d", trash_can_length);
    
    char config_content[1000] = {'\0'};
    strcat(config_content, "length:");
    strcat(config_content, str_length);
    
    strcat(config_content, "\n");

    char str_size[(int)((ceil(log10(trash_can_size))+1)*sizeof(char))];
    sprintf(str_size, "%d", trash_can_size);

    strcat(config_content, "size:");
    strcat(config_content, str_size);
    
    int i = 0;
    while (config_content[i] != '\0') {
        fputc(config_content[i], config_file);
        i++;
    }
    
    fclose(config_file);
    return 0;
}

int add_file_to_trash(char *file_name) {
    
    // Sanatize the name a bit
    if (file_name[strlen(file_name) - 1] == '/') {
        file_name[strlen(file_name) - 1] = '\0';
    }

    // Move the file
    pid_t trash_mv_pid;
    extern char **environ;

    char *path = get_path_to_trash();
    char *mv_argv[] = {"/bin/mv", file_name, path, NULL};

    if (posix_spawn(&trash_mv_pid, "/bin/mv", NULL, NULL, mv_argv, environ) != 0) {
        perror("posix_spawn");
        printf("Error: cannot delete file or file does not exist\n");
        exit(1);
        return 1;
    }
    free(path);

    int exit_status;
    if (waitpid(trash_mv_pid, &exit_status, 0) == -1) {
        perror("waitpid");
        exit(1);
        printf("Error: issue with waitpid\n");
        return 1;
    }
    
    if (exit_status != EXIT_SUCCESS) {
        return 1;
    }
    
    // Create file in trash with path for restoration
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
    
    char command2[100] = {'\0'};
    strcat(command2, "echo ");
    strcat(command2, cwd);
    strcat(command2, ">~/.trash/.");
    strcat(command2, file_name);
    strcat(command2, ".path");
    system(command2);
    
    // Concats the timestamp into the info file
    long tmp = (long)time(NULL);  
       
    char command3[100] = {'\0'};
    strcat(command3, "echo ");

    char str_size[(int)((ceil(log10(tmp))+1)*sizeof(char))];
    sprintf(str_size, "%d", (int)tmp);

    strcat(command3, str_size);
    strcat(command3, ">~/.trash/.");
    strcat(command3, file_name);
    strcat(command3, ".time");
    system(command3);

    return 0;
}

int list_content() {
    return system("ls ~/.trash");
}

int restore_all() {
    int counter = 0;
    
    // Gets path to trash
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char path[strlen(homedir) + 9];
    snprintf(path, sizeof(path), "%s%s%s", homedir, "/.trash/", "\0");

    DIR *dirp = opendir(path);
    struct dirent *de;
 
    // Loop through all files in the .trash file and restore all that arent dot directories
    while ((de = readdir(dirp)) != NULL) {
        if (!(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || (de->d_name)[0] == '.')) {
            restore_file(de->d_name);
            counter++;
        }
    }       
    printf("Restored %d files\n", counter);
    return 0;
}

int restore_file(char *file_name) {

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }

    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    char path[100] = {'\0'};

    char path_file_name[100] = {'\0'};
    strcat(path_file_name, homedir);
    strcat(path_file_name, "/.trash/.");
    strcat(path_file_name, file_name);
    strcat(path_file_name, ".path");

    FILE *path_file = fopen(path_file_name, "r");
    if (path_file == NULL) {
        perror(path_file_name);  // prints why the open failed
        return 1;
    }
    
    int c;
    int i = 0;
    while((c = fgetc(path_file)) != EOF) {
        path[i] = c;
        i++;
    }
    
    fclose(path_file);

    char command[100] = {'\0'};
    strcat(command, "mv ");
    strcat(command, homedir);
    strcat(command, "/.trash/");
    strcat(command, file_name);
    strcat(command, " ");
    strcat(command, path);
    system(command);
 
    remove_dot_files(file_name);

    return 0;
}

int clear_trash() {
    system("rm -rf ~/.trash/*");
    return system("rm -rf ~/.trash/.* 2> /dev/null");
}

int auto_clear() {
    
    // Block of code calculates the size of the trash currently stored
    // int size = 0;
        
    int counter = 0;
 
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
   
    char *trash_dir = get_path_to_trash();

    DIR *dirp = opendir(trash_dir);
    struct dirent *de;
 
    int timestamp = 0;
    char oldest_file_name[100] = {'\0'};
     
    while ((de = readdir(dirp)) != NULL) {
        
        if (!(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) {
        
            if (strstr(de->d_name, ".time")) {

                char file_name[100] = {'\0'};
                strcat(file_name, homedir);
                strcat(file_name, "/.trash/");
                strcat(file_name, de->d_name);

                FILE* fp = fopen(file_name, "r");
                if(!fp) {
                    perror("File opening failed");
                    return EXIT_FAILURE;
                }
                
                char timestamp_string[100] = {'\0'};
                int i = 0;
                int c; // note: int, not char, required to handle EOF
                while ((c = fgetc(fp)) != EOF) { // standard C I/O file reading loop
                    timestamp_string[i] = c;
                    i++;
                }
                int tmp_timestamp = atoi(timestamp_string);           
                
                if (timestamp == 0) {
                    timestamp = tmp_timestamp;
                    strcpy(oldest_file_name, de->d_name);
                     // Converts .file.time to just file
                    int i = 0;
                    for (i = 0; i < strlen(oldest_file_name) - 5; i++) {
                            oldest_file_name[i] = oldest_file_name[i + 1];
                    }
                    oldest_file_name[i - 1] = '\0';
                } else {
                    if (tmp_timestamp < timestamp) {
                        timestamp = tmp_timestamp;
                        strcpy(oldest_file_name, de->d_name);
                        
                        // Converts .file.time to just file
                        int i = 0;
                        for (i = 0; i < strlen(oldest_file_name) - 5; i++) {
                                oldest_file_name[i] = oldest_file_name[i + 1];
                        }
                        oldest_file_name[i - 1] = '\0';
                    }
                }
                fclose(fp);
            }    
            counter++;
        }
    }       

    // Hides
    counter = counter/3;

    

    if (counter > BIN_SIZE) {

        remove_file_from_trash(oldest_file_name);
        auto_clear();
    
    //} else if ((counter * 3)/4 = BIN_SIZE) {
    //    printf("BIN is approaching full size, once full, it will auto delete");
    }

    return 0;
}

int remove_file_from_trash(char *file_name) {
    
    printf("Removing %s\n", file_name);

    remove_file(file_name);   
    remove_dot_files(file_name);

    return 0;
}

int remove_dot_files(char *file_name) {

    // remove file_name.path
    char path_file_name[sizeof(file_name) + 6];
    snprintf(path_file_name, sizeof(path_file_name), "%s%s%s", ".", file_name, ".path");
    remove_file(path_file_name);

    // remove file_name.time
    char time_file_name[sizeof(file_name) + 6];
    snprintf(time_file_name, sizeof(time_file_name), "%s%s%s", ".", file_name, ".time");
    remove_file(time_file_name);

    return 0;
}

int remove_file(char *file_name) {
    
    // +9 for the /.trash and \0
    char *path = get_path_to_trash();
    snprintf(path, sizeof(path), "%s", file_name);
    
    // process to remove file from trash
    pid_t remove_file_pid;
    extern char **environ;

    char *rm_argv[] = {"/bin/rm", "-rf", path, NULL};

    if (posix_spawn(&remove_file_pid, "/bin/rm", NULL, NULL, rm_argv, environ) != 0) {
        perror("spawn");
        exit(1);
        printf("Error: cannot delete file\n");
        return 1;
    }

    int exit_status;
    if (waitpid(remove_file_pid, &exit_status, 0) == -1) {
        perror("waitpid");
        exit(1);
        printf("Error\n");
        return 1;
    }
    
    return 0;
}

char *get_path_to_trash() {
    // Gets path to trash
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char *path = malloc(strlen(homedir) + 50);
    snprintf(path, strlen(homedir) + 50, "%s%s%s", homedir, "/.trash/", "\0");

    return path;
}
