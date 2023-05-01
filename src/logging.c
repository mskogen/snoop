/**
​*​ ​@file​: logging.c
​*​ ​@brief​: Implementation file for Logging module
*
​* @description: The logging module provides an interface to create a file for
*               application logging. Logs are timestamped for additional info.
*
​*​ ​@author:​ ​Matthew Skogen
​*​ ​@date​: April 27 ​2023
​*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#include "logging.h"

#define LOG_FILE_BASE   ("snoop_log")

// Want to save only one logfile at a time
static char g_logfile_path[MAX_PATH_SIZE];
static char g_time_buffer[TIME_BUF_SIZE];
static bool logfile_exists = false;

static void update_timestamp()
{
    time_t timestamp;
    struct tm *info;

    memset(g_time_buffer, 0, TIME_BUF_SIZE);

    // Create timestamp for application start, use UTC time
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(g_time_buffer, TIME_BUF_SIZE-1, "%m%d%Y_%H%M%S", info);

    return;
}

int new_logfile(char *base_dir)
{
    if (logfile_exists) {
        // logfile instance already exists
        return EXIT_FAILURE;
    }

    FILE *f_logfile = NULL;

    // Zero out our buffers
    memset(g_logfile_path, 0, sizeof(g_logfile_path));

    // Create path for logfile
    update_timestamp();
    snprintf(g_logfile_path, MAX_PATH_SIZE-1, "%s/%s_%s",
                base_dir, LOG_FILE_BASE, g_time_buffer);

    // Open a connection to the file, should always create a new file
    f_logfile = fopen(g_logfile_path, "w");

    if (f_logfile == NULL) {
        // Failed to create logfile
        return EXIT_FAILURE;
    }

    // Created file! Close for success
    logfile_exists = true;
    fclose(f_logfile);

    return EXIT_SUCCESS;
}

int write_logfile(char* write_str)
{
    // Make sure logfile exists
    if (!logfile_exists) {
        return EXIT_FAILURE;
    }

    // Open a connection to the file to append to logfile
    FILE *f_logfile = fopen(g_logfile_path, "a");

    if (f_logfile == NULL) {
        // Failed to open logfile
        return EXIT_FAILURE;
    }

    // File opened succesfully, write to file
    update_timestamp();
    fprintf(f_logfile, "[%s] %s\n", g_time_buffer, write_str);

    // Close file to finish writing
    fclose(f_logfile);

    return EXIT_SUCCESS;
}

