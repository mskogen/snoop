// Header file for logging module to write information about application to a log

#define MAX_PATH_SIZE   (256)
#define MAX_FILE_NAME   (64)
#define LINE_BUF_SIZE   (2048)

int new_logfile(char *path);
int write_logfile(char* write_str);
