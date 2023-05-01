/**
​*​ ​@file​ logging.h
​*​ ​@brief​ Header file for logging module
​*
​*​ ​@author​ ​Matthew Skogen
​*​ ​@date​ April 27 ​2023
​*/
#ifndef __LOGGING_H__
#define __LOGGING_H__

#define MAX_PATH_SIZE   (256)
#define MAX_FILE_NAME   (64)
#define LINE_BUF_SIZE   (2048)
#define TIME_BUF_SIZE   (80)

// Create new logfile to write information for status and debugging to
int new_logfile(char *path);

// Write a string to the file
int write_logfile(char* write_str);

#endif // __LOGGING_H__