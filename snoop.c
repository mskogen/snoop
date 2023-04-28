/*****************************************************************************
​*​ ​Copyright​ ​(C)​ ​2023 ​by​ Matthew Skogen
​*
​*​ ​Redistribution,​ ​modification​ ​or​ ​use​ ​of​ ​this​ ​software​ ​in​ ​source​ ​or​ ​binary
​*​ ​forms​ ​is​ ​permitted​ ​as​ ​long​ ​as​ ​the​ ​files​ ​maintain​ ​this​ ​copyright.​ ​Users​ ​are
​*​ ​permitted​ ​to​ ​modify​ ​this​ ​and​ ​use​ ​it​ ​to​ ​learn​ ​about​ ​the​ ​field​ ​of​ ​embedded
​*​ ​software.​ ​Matthew Skogen ​and​ ​the​ ​University​ ​of​ ​Colorado​ ​are​ ​not​ ​liable​ ​for
​*​ ​any​ ​misuse​ ​of​ ​this​ ​material.
​*
*****************************************************************************/
/**
​*​ ​@file​ snoop.c
​*​ ​@brief​ Application entry point
​*
​*​ ​@author​s ​Matthew Skogen
​*​ ​@date​ April 23 ​2023
*
* @description: Middleware application intended to recieve camera data from an
*               http server and log frame data to an external drive.
​*/
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>

#include "logging.h"

#define MAX_CMD_LENGTH  (1024)

// Global variables
bool exit_status = false;
bool syslog_open = false;
// pthread_mutex_t thread_mutex;
// bool mutex_active = false;

// Handles both SIGINT and SIGTERM signals
static void signal_handler(int signum)
{
    if ((signum == SIGINT) || (signum == SIGTERM)) {
        syslog(LOG_INFO, "Caught signal, exiting\n");

        // Set global to shutdown daemon when possible
        exit_status = true;
        return;
    }
    
    // Unknown signal
    syslog(LOG_ERR, "Error: Caught unknown signal\n");
    return;
}

// Cleanup connections before closing
void cleanup(bool terminate)
{
    // int status = 0;

    // If we are exiting after this call, close all open file descriptors
    if (terminate) {

        // if (timer_active) {
        //     status = timer_delete(timer);
        //     if (status != 0) {
        //         syslog(LOG_ERR, "Error timer_delete(): %s\n", strerror(errno));
        //     }
        //     timer_active = false;
        // }

        // Close out connection to system logger
        if (syslog_open) {
            closelog();
            syslog_open = false;
        }
    }

    // Set exit status in case it hasn't been set already
    exit_status = true;

    return;
}

int main(int argc, char**argv)
{
    // Verify proper number of arguments passed in
    if (argc != 4) {
        printf("ERROR: Invalid number of arguments %i\n", argc);
        printf("Usage: ./snoop /path/to/storage/drive [http_ip] [http_port]\n");
        return EXIT_FAILURE;
    }

    char filePath[MAX_PATH_SIZE];
    char fileName[MAX_FILE_NAME];
    char command[MAX_CMD_LENGTH];
    char write_str[LINE_BUF_SIZE];
    char *base_dir = argv[1];
    char *host = argv[2];
    char *port = argv[3];
    int base_dir_len = strlen(base_dir);
    int num_images = 0;

    // Open connection to system logger with default LOG_USER facility
    openlog("snoop", LOG_CONS, LOG_USER);
    syslog_open = true;

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        syslog(LOG_ERR, "Error: Cannot register SIGINT\n");
        cleanup(true);
        return EXIT_FAILURE;
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        syslog(LOG_ERR, "Error: Cannot register SIGTERM\n");
        cleanup(true);
        return EXIT_FAILURE;
    }

    // Create daemon process and close initial start process
    pid_t pid = fork();
    switch (pid) {
    case -1:
        // Failed to create child process
        syslog(LOG_ERR, "Error fork()\n");
        cleanup(true);
        return EXIT_FAILURE;
    case 0:
        // Inside child process
        syslog(LOG_DEBUG, "Successfully created child process()\n");
        break;
    default:
        // Close initial parent process
        cleanup(true);
        return EXIT_FAILURE;
    }

    // Create a new logfile instance
    if (new_logfile(base_dir) == EXIT_FAILURE) {
        syslog(LOG_ERR, "Failed to open new logfile.\n");
        cleanup(true);
        return EXIT_FAILURE;
    }

    write_logfile("Starting Snoop");

    for (int i = 0; i < 100; i++) {
        memset(filePath, 0, sizeof(filePath));
        memset(fileName, 0, sizeof(fileName));
        memset(command, 0, sizeof(command));

        sprintf(filePath, "%s/", base_dir);
        sprintf(fileName, "test_image_%i.jpg", num_images++);
        memcpy(&filePath[base_dir_len+1], fileName, strlen(fileName));

        // printf("saving image to file [%s]\n", filePath);

        sprintf(command, "curl -s http://%s:%s/capture > %s", host, port, filePath);

        // printf("Sending command [%s]\n", command);

        system(command);

        usleep(33333);
    }

    memset(write_str, 0, sizeof(write_str));
    sprintf(write_str, "Wrote %i images to %s", num_images, base_dir);

    write_logfile(write_str);

    // Cleanup any opened resources
    cleanup(true);

    write_logfile("Closing Snoop");

    return EXIT_SUCCESS;
}