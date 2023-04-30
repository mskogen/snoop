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
#include <time.h>

#include "logging.h"
#include "image_collection.h"

// @15Hz intervals, create video once every minute
#define CAPTURE_FREQ_HZ         (15)
#define NUM_IMAGES_PER_VIDEO    (CAPTURE_FREQ_HZ*60)

// Global variables
bool exit_status = false;
bool syslog_open = false;
bool timer_active = false;
bool mutex_active = false;
timer_t timer;
static int num_images = 0;
static int num_videos = 0;
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
    int status = 0;

    // If we are exiting after this call, close all open file descriptors
    if (terminate) {

        // Shutdown timer so no more images are triggered
        if (timer_active) {
            status = timer_delete(timer);
            if (status != 0) {
                syslog(LOG_ERR, "Error timer_delete(): %s\n", strerror(errno));
            }
            timer_active = false;
        }

        if (mutex_active) {
            destroy_mutex();
            mutex_active = false;
        }

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

// Handler for capturing images at consistant intervals
void timer_thread_handler(union sigval sv)
{
    num_images = capture_image();
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

    char write_str[LINE_BUF_SIZE];
    char *base_dir = argv[1];
    char *host = argv[2];
    char *port = argv[3];
    int status = 0;
    int sleep_time = 0;

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

    // Setup camera sensor module
    if (init_camera(base_dir, host, port, CAPTURE_FREQ_HZ) == MUTEX_FAILURE) {
        write_logfile("Failure to initialize mutex. Closing...");
        cleanup(true);
        return EXIT_FAILURE;
    } else {
        mutex_active = true;
    }

    // Setup timer capture function
    struct sigevent timer_event;
    struct itimerspec itime_spec;

    memset(&timer_event, 0, sizeof(struct sigevent));
    timer_event.sigev_notify = SIGEV_THREAD;
    timer_event.sigev_notify_function = &timer_thread_handler;

    status = timer_create(CLOCK_REALTIME, &timer_event, &timer);

    if (status) {
        syslog(LOG_ERR, "Error timer_create(): %s\n", strerror(errno));
        cleanup(true);
        return EXIT_FAILURE;
    } else {
        timer_active = true;
    }

    // Set time to trigger camera capture at 15 Hz or ~66,666,660 nanosecs
    memset(&itime_spec, 0, sizeof(struct itimerspec));
    itime_spec.it_interval.tv_sec = 0;
    itime_spec.it_interval.tv_nsec = 66666660;
    itime_spec.it_value.tv_sec = 0;
    itime_spec.it_value.tv_nsec = 66666660;

    status = timer_settime(timer, 0, &itime_spec, NULL);

    if (status) {
        syslog(LOG_ERR, "Error timer_settime(): %s\n", strerror(errno));
        cleanup(true);
        return EXIT_FAILURE;
    }

    // Loop forever while we capture images, occasionally create a video file
    // to clean up existing images
    while (!exit_status) {
        // Guarantee sleep for 60 seconds so video processing only happens
        // once per minute.
        sleep_time = 60;
        while (sleep_time > 0) {
            sleep_time = sleep(sleep_time);
        }

        // Convert minute's worth of images to video data and delete image data
        num_videos = convert_to_video();
    }

    // Cleanup any opened resources
    cleanup(true);

    // Write status of captured data
    memset(write_str, 0, sizeof(write_str));
    sprintf(write_str, "Saved %i images %i videos", num_images, num_videos);
    write_logfile(write_str);

    write_logfile("Closing Snoop");

    return EXIT_SUCCESS;
}