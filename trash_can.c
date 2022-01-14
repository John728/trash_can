#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>

int init_trash() {
    printf("Creating files...\n");
    
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    
    // Creates /.trash folder
    char trash_path_arg[100] = {'\0'};
    strcat(trash_path_arg, homedir);
    strcat(trash_path_arg, "/.trash");
    
    pid_t trash_mkdir_pid;
    extern char **environ;
    char *mkdir_argv[] = {"/bin/mkdir", trash_path_arg, NULL};

    if (posix_spawn(&trash_mkdir_pid, "/bin/mkdir", NULL, NULL, mkdir_argv, environ) != 0) {
        perror("spawn");
        exit(1);
    }

    int exit_status;
    if (waitpid(trash_mkdir_pid, &exit_status, 0) == -1) {
        perror("waitpid");
        exit(1);
    }
    
    // Create config file for trash_can
    char path_arg2[100] = {'\0'};
    strcat(path_arg2, homedir);
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


    system("touch ~/.config/trash_can/config");
    
    int trash_can_size = 0;
    printf("At what size should the trash be taken out? (in bytes) (recomended: 1000): ");
    scanf("%d", &trash_can_size);

    int trash_can_length = 0;
    printf("How many days until a file in trash is taken out? (recomended: 30): ");
    scanf("%d", &trash_can_length);

    char open_path[100] = {'\0'};
    strcat(open_path, homedir);
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
    // Move the file
    char command[100] = {'\0'};
    strcat(command, "mv ");
    strcat(command, file_name);
    strcat(command, " ~/.trash");
    system(command);

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
    strcat(command2, ".info");
    system(command2);
    
    long tmp = (long)time(NULL);  
    
    char command3[100] = {'\0'};
    strcat(command3, "echo ");

    char str_size[(int)((ceil(log10(tmp))+1)*sizeof(char))];
    sprintf(str_size, "%d", (int)tmp);

    strcat(command3, str_size);
    strcat(command3, ">>~/.trash/.");
    strcat(command3, file_name);
    strcat(command3, ".info");
    system(command3);
    
    return 0;
}

int add_directory_to_trash();

int list_content() {
    return system("ls ~/.trash");
}

int restore_all();

int restore_file(char *file_name) {

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }

    char path[100] = {'\0'};

    char path_file_name[100] = {'\0'};
    strcat(path_file_name, "/home/johnhenderson/.trash/.");
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

    char command[100] = {'\0'};
    strcat(command, "mv /home/johnhenderson/.trash/");
    strcat(command, file_name);
    strcat(command, " ");
    strcat(command, path);
    system(command);
    
    char command2[100] = {'\0'};
    strcat(command2, "rm /home/johnhenderson/.trash/.");
    strcat(command2, file_name);
    strcat(command2, ".path");
    system(command2);

    return 0;
}

int restore_directory();

int clear_trash() {
    return system("rm -rf ~/.trash/*");
}

int auto_clear() {
    
    // Block of code calculates the size of the trash currently stored
    // int size = 0;
    
    int counter = 0;

    DIR *dirp = opendir("/home/johnhenderson/.trash/");
    struct dirent *de;
    
    while ((de = readdir(dirp)) != NULL) {
        
        if (!(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) {
        
            //char file_name[100] = {'\0'};
            //strcat(file_name, "/home/johnhenderson/.trash/");
            //strcat(file_name, de->d_name);
            
            //FILE *file = fopen(file_name, "r");
            //if (file == NULL) {
            //    perror(file_name);  // prints why the open failed
            //    return 1;
            //}   
        
            //fseek(file, 0L, SEEK_END);
            //size += ftell(file);
            counter++;
        }
    }       
    counter = counter/2;
    
    if (counter > 4) {
        printf("Taking out the oldest item /n");
        
    }

    return 0;
}
