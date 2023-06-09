/**
​*​ ​@file​: image_collection.c
​*​ ​@brief​: Implementation file for Image Collection module
*
​* @description: The image collection module is used for interfacing with a
*               camera sensor that is running an http server and images
*               can be transfered via http requests. The intent of this module
*               is to capture the images, and additional features will allow
*               the user to convert the images to video streams if run on a 
*               compatible system.
*
​*​ ​@author:​ ​Matthew Skogen
​*​ ​@date​: April 27 ​2023
​*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

#include "image_collection.h"
#include "logging.h"

#define MAX_CMD_LENGTH  (1024)

// Global variables
static char g_file_path[MAX_PATH_SIZE];
static char *g_base_dir = NULL;
static int g_base_dir_len = 0;
static int g_num_images = 0;
static int g_num_folders = 0;
static int g_num_videos = 0;
static char *g_host = NULL;
static char *g_port = NULL;
static int g_frame_rate = 0;
pthread_mutex_t g_lock;
bool g_mutex_active = false;

int init_camera(char *base_dir, char *host, char *port, int frame_rate)
{
    // Initialize mutex
    if (pthread_mutex_init(&g_lock, NULL) != 0) {
        return MUTEX_FAILURE;
    } else {
        g_mutex_active = true;
    }

    // Create a new timestamp for unique data folders, use UTC time
    time_t timestamp;
    struct tm *info;
    char time_buffer[TIME_BUF_SIZE];
    int time_buf_len = 0;

    memset(time_buffer, 0, TIME_BUF_SIZE);
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(time_buffer, TIME_BUF_SIZE-1, "%m%d%Y_%H%M%S", info);
    time_buf_len = strlen(time_buffer);

    // Create data directory for storing image/video data too
    g_base_dir = base_dir;
    g_base_dir_len = strlen(base_dir);
    sprintf(g_file_path, "%s/data_%s", base_dir, time_buffer);
    g_base_dir_len += 6 + time_buf_len;
    mkdir(g_file_path, S_IRWXU | S_IRWXG | S_IRWXO);

    // Save host and port for camera capture calls
    g_host = host;
    g_port = port;
    g_frame_rate = frame_rate;

    return EXIT_SUCCESS;
}

int capture_image()
{
    char file_name[MAX_FILE_NAME];
    char command[MAX_CMD_LENGTH];

    // Zero memory of file name and command
    memset(file_name, 0, sizeof(file_name));
    memset(command, 0, sizeof(command));

    sprintf(file_name, "image_%i.jpg", g_num_images++);

    // Capture image from http image server 
    pthread_mutex_lock(&g_lock);
    sprintf(command, "curl -fs -o %s/%s http://%s:%s/capture", g_file_path, file_name, g_host, g_port);
    pthread_mutex_unlock(&g_lock);

    system(command);

    return g_num_images;
}

int convert_to_video()
{
    // Convert .jpg to .mp4 via ffmpeg utility
    char file_name[MAX_FILE_NAME];
    char command[MAX_CMD_LENGTH];
    char tmp_file_path[MAX_PATH_SIZE];

    memset(tmp_file_path, 0, sizeof(tmp_file_path));

    // Save previous directory for video processing
    memcpy(tmp_file_path, g_file_path, strlen(g_file_path));

    // Create a new timestamp for unique data folders, use UTC time
    time_t timestamp;
    struct tm *info;
    char time_buffer[TIME_BUF_SIZE];
    int time_buf_len = 0;

    memset(time_buffer, 0, TIME_BUF_SIZE);
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(time_buffer, TIME_BUF_SIZE-1, "%m%d%Y_%H%M%S", info);
    time_buf_len = strlen(time_buffer);

    // Create new folder for storing images
    pthread_mutex_lock(&g_lock);

    // Create new data directory for storing image/video data too
    memcpy(&g_file_path[g_base_dir_len - time_buf_len], time_buffer, time_buf_len);
    mkdir(g_file_path, S_IRWXU | S_IRWXG | S_IRWXO);

    pthread_mutex_unlock(&g_lock);

    // Zero memory of file name and command
    memset(file_name, 0, sizeof(file_name));
    memset(command, 0, sizeof(command));

    sprintf(file_name, "video_%i.mp4", g_num_videos++);

    // Capture image from http image server
    sprintf(command, "ffmpeg -hide_banner -loglevel panic -framerate %i -r %i -pattern_type glob -i \'%s/*.jpg\' -c:v libx264 \'%s/%s\'", 
        g_frame_rate, g_frame_rate, tmp_file_path, tmp_file_path, file_name);

    system(command);
    return g_num_videos;
}

int new_folder()
{
    // Create a new timestamp for unique data folders, use UTC time
    time_t timestamp;
    struct tm *info;
    char time_buffer[TIME_BUF_SIZE];
    int time_buf_len = 0;

    memset(time_buffer, 0, TIME_BUF_SIZE);
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(time_buffer, TIME_BUF_SIZE-1, "%m%d%Y_%H%M%S", info);
    time_buf_len = strlen(time_buffer);

    // Create new folder for storing images
    pthread_mutex_lock(&g_lock);

    // Create new data directory for storing image/video data too
    memcpy(&g_file_path[g_base_dir_len - time_buf_len], time_buffer, time_buf_len);
    mkdir(g_file_path, S_IRWXU | S_IRWXG | S_IRWXO);

    pthread_mutex_unlock(&g_lock);

    g_num_folders++;

    return g_num_folders;
}

void destroy_mutex()
{
    if (g_mutex_active) {
        pthread_mutex_destroy(&g_lock);
        g_mutex_active = false;
    }
    return;
}
