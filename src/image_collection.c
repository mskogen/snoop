#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "image_collection.h"
#include "logging.h"

#define MAX_CMD_LENGTH  (1024)

// Global variables
static char g_file_path[MAX_PATH_SIZE];
static char *g_base_dir = NULL;
static int g_base_dir_len = 0;
static int g_num_images = 0;
static int g_num_videos = 0;
static char *g_host = NULL;
static char *g_port = NULL;

void init_camera(char *base_dir, char *host, char *port)
{
    // Create a new timestamp for unique data folders, use UTC time
    time_t timestamp;
    struct tm *info;
    static char time_buffer[TIME_BUF_SIZE];
    int time_buf_len = 0;

    memset(time_buffer, 0, TIME_BUF_SIZE);
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(time_buffer, TIME_BUF_SIZE-1, "%m%d%Y_%H%M%S", info);
    time_buf_len = strlen(time_buffer);

    // Create data directory for storing image/video data too
    g_base_dir = base_dir;
    g_base_dir_len = strlen(base_dir);
    sprintf(g_file_path, "%s/data_%s/", base_dir, time_buffer);
    g_base_dir_len += 7 + time_buf_len;
    mkdir(g_file_path, S_IRWXU | S_IRWXG | S_IRWXO);

    // Save host and port for camera capture calls
    g_host = host;
    g_port = port;

    return;
}

int capture_image()
{
    char file_name[MAX_FILE_NAME];
    char command[MAX_CMD_LENGTH];

    // Zero memory of file name and command
    memset(file_name, 0, sizeof(file_name));
    memset(command, 0, sizeof(command));
    memset(&g_file_path[g_base_dir_len], 0, MAX_PATH_SIZE-g_base_dir_len);

    sprintf(file_name, "image_%i.jpg", g_num_images++);
    memcpy(&g_file_path[g_base_dir_len], file_name, strlen(file_name));

    // Capture image from http image server 
    sprintf(command, "curl -s http://%s:%s/capture > %s", g_host, g_port, g_file_path);
    // vlc -I dummy http://192.168.0.137:81/stream --sout=file/ts:output9.mp4 ?

    system(command);

    return g_num_images;
}


int convert_to_video()
{
    // Do nothing so far, will read from data dir and convert .jpg to .mp4 via
    // ffmpeg utility
    return g_num_videos;
}