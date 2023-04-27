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
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <signal.h>

#include "http_client.h"

#define LOG_FILE_BASE   ("snoop_log")
#define MAX_PATH_SIZE   (256)
#define MAX_LINE_SIZE   (2048)

// Global variables
bool exit_status = false;
bool syslog_open = false;
bool logfile_open = false;
FILE *logfile = NULL;
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

        // if (mutex_active) {
        //     status = pthread_mutex_destroy(&thread_mutex);
        //     if (status != 0) {
        //         syslog(LOG_ERR, "Error pthread_mutex_destroy(): %s\n", strerror(status));
        //     }
        //     mutex_active = false;
        // }

        // if (tmp_file_exists) {
        //     status = remove(TMP_FILE);
        //     if (status != 0) {
        //         syslog(LOG_ERR, "Error remove(): %s\n", strerror(errno));
        //     }
        //     tmp_file_exists = false;
        // }

        // Close out connection to the file descriptor if file was opened
        if (logfile_open) {
            fclose(logfile);
            logfile_open = false;
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

int main(int argc, char**argv)
{
    // Verify proper number of arguments passed in
    if (argc != 2) {
        printf("ERROR: Invalid number of arguments %i\n", argc);
        printf("Usage: ./snoop /path/to/storage/drive\n");
        return EXIT_FAILURE;
    }

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

    // Objects for timestamping application start
    time_t timestamp;
    struct tm *info;
    char time_buffer[80];
    char write_str[MAX_LINE_SIZE];

    // Zero out our buffers
    memset(time_buffer, 0, sizeof(time_buffer));
    memset(write_str, 0, sizeof(write_str));

    // Create timestamp for application start, use UTC time
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(time_buffer, sizeof(time_buffer), "%m%d%Y_%H%M%S", info);

    // First argument is a path to directory on filesystem
    char write_file[MAX_PATH_SIZE];
    snprintf(write_file, sizeof(write_file), "%s/%s_%s", 
                argv[1], LOG_FILE_BASE, time_buffer);

    /* 
     * Open a connection to the file, create new file if it doesn't exist and
     * overwrite file if it does exist.
     */
    int ret_val = 0;
    logfile = fopen(write_file, "w");

    if (logfile == NULL) {
        syslog(LOG_ERR, "Failed to open %s: %m\n", write_file);
        cleanup(true);
        return EXIT_FAILURE;
    } else {
        logfile_open = true;
    }

    // File opened succesfully, write to file
    snprintf(write_str, sizeof(write_str), "%s - %s\n", 
                "Starting Snoop", time_buffer);
    ret_val = fprintf(logfile, "%s", write_str);

    if (ret_val < 0) {
        // Error writing to file, check errno
        syslog(LOG_ERR, "Failed to write to file: %m\n");
    } else if (ret_val != strlen(write_str)) {
        // Failed to write all bytes to file
        syslog(LOG_WARNING, "Only wrote %i bytes to file\n", ret_val);
    } else {
        // Successful write
        syslog(LOG_DEBUG, "Successfully wrote full string to file.\n");
    }

    // struct sockaddr_in addr, cl_addr;  
    // int sockfd, ret; 
    // struct hostent * server;
    // char * url, * temp;
    // int portNumber;
    // char * fileName;
    // char status_ok[] = "OK";
    // char buffer[BUF_SIZE]; 
    // char http_not_found[] = "HTTP/1.0 404 Not Found";
    // char http_ok[] = "HTTP/1.0 200 OK";
    // char location[] = "Location: ";
    // char contentType[] = "Content-Type: ";
    // int sPos, ePos;

    // if (argc < 3) {
    //     printf("usage: [URL] [port number]\n");
    //     exit(1);  
    // }

    // url = argv[1];
    // portNumber = atoi(argv[2]);

    // //checking the protocol specified
    // if ((temp = strstr(url, "http://")) != NULL) {
    //     url = url + 7;
    // } else if ((temp = strstr(url, "https://")) != NULL) {
    //     url = url + 8;
    // }

    // //checking the port number
    // if (portNumber > 65536 || portNumber < 0) {
    //     printf("Invalid Port Number!");
    //     exit(1);
    // }

    // sockfd = get_request(url, argv[2]); 

    // memset(&buffer, 0, sizeof(buffer));
    // ret = recv(sockfd, buffer, BUF_SIZE, 0);  
    // if (ret < 0) {  
    //     printf("Error receiving HTTP status!\n");    
    // } else {
    //     printf("%s\n", buffer);
    //     if ((temp = strstr(buffer, http_ok)) != NULL) {
    //         send(sockfd, status_ok, strlen(status_ok), 0);
    //     } else {
    //         close(sockfd);
    //         return 0;
    //     }
    // }

    // memset(&buffer, 0, sizeof(buffer)); 
    // ret = recv(sockfd, buffer, BUF_SIZE, 0);  
    // if (ret < 0) {  
    //     printf("Error receiving HTTP header!\n");    
    // } else {
    //     printf("%s\n", buffer);
    //     if (parseHeader(buffer) == 0) {
    //         send(sockfd, status_ok, strlen(status_ok), 0);
    //     } else {
    //         printf("Error in HTTP header!\n");
    //         close(sockfd);
    //         return 0;
    //     }
    // } 

    // //printf("file: [%s]\n", fileName);
    // fileptr = fopen(path, "w");
    // if (fileptr == NULL) {
    //     printf("Error opening file!\n");
    //     close(sockfd);
    //     return 0;
    // }

    // memset(&buffer, 0, sizeof(buffer));
    // while (recv(sockfd, buffer, BUF_SIZE, 0) > 0) { //receives the file
    //     if ((strstr(contentFileType, "text/html")) != NULL) {
    //         fprintf(fileptr, "%s", buffer);
    //     } else {
    //         fwrite(&buffer, sizeof(buffer), 1, fileptr);
    //     }
    //     memset(&buffer, 0, sizeof(buffer));
    // }

    // fclose(fileptr);
    // close(sockfd);

    // openFile();

    sleep(60);
    
    // Zero out our buffers
    memset(time_buffer, 0, sizeof(time_buffer));
    memset(write_str, 0, sizeof(write_str));

    // Create timestamp for application end, use UTC time
    time(&timestamp);
    info = gmtime(&timestamp);
    strftime(time_buffer, sizeof(time_buffer), "%m%d%Y_%H%M%S", info);

    // Write to file
    snprintf(write_str, sizeof(write_str), "%s - %s\n", 
                "Closing Snoop", time_buffer);
    ret_val = fprintf(logfile, "%s", write_str);

    if (ret_val < 0) {
        // Error writing to file, check errno
        syslog(LOG_ERR, "Failed to write to file: %m\n");
    } else if (ret_val != strlen(write_str)) {
        // Failed to write all bytes to file
        syslog(LOG_WARNING, "Only wrote %i bytes to file\n", ret_val);
    } else {
        // Successful write
        syslog(LOG_DEBUG, "Successfully wrote full string to file.\n");
    }

    // Cleanup any opened resources
    cleanup(true);

    return EXIT_SUCCESS;
}