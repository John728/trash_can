// Will be run once in the program life-cycle.
// Will create a .trash folder at ~/ and it will also create a directory
// called trash_config in  ~/.config/ which will store settings such as 
// trash size. Will also ask the user for this info.
int init_trash();

// This function will be called when the user types 'del <file name>'. It
// will move the file from the directory to the trash folder. When the file
// is moved, it will create a <file name>.config file which will store the 
// original path of the file incase the user needs to restore the file.
int add_file_to_trash();

// Same as for the add_file_to_trash, but recursivly over a whole directory.
// Will only create one <file name>.config file.
int add_file_to_trash(char *file_name);

// Will list the content of the .trash folder, includeing files and directories.
// Will only list the directory names, not recursivly.
int list_content();

// Restores all content of the trash to their original place. If the original path
// does not exist, then it'll create the original path again using teh <file name>.config
// file.
int restore_all();

// Same as restore_all but with a single file
int restore_file();

// Same as restore_all but with a single directory
int restore_directory();

// Deletes all files in the trash.
int clear_trash();

// This funciton is run every time a file or directory is added to 
// the .trash. It checks the user settings, and if the size of the
// trash is over the total size specified by the user, it will 
// delete the oldest item, it any items over the specified time 
// of the user.
int auto_clear();
