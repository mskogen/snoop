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
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include "http_client.h"

// TOREMOVE: only for sleep to temporarily test log
#include <unistd.h>

#define LOG_FILE_BASE   ("snoop_log")
#define MAX_PATH_SIZE   (256)
#define MAX_LINE_SIZE   (2048)

int main(int argc, char**argv)
{
    // Verify proper number of arguments passed in
    if (argc != 2) {
        printf("ERROR: Invalid number of arguments %i\n", argc);
        printf("Usage: ./snoop /path/to/storage/drive\n");
        return 1;
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

    // Open connection to system logger with default LOG_USER facility
    openlog("snoop", LOG_CONS, LOG_USER);

    // First argument is a path to directory on filesystem
    char write_file[MAX_PATH_SIZE];
    snprintf(write_file, sizeof(write_file), "%s/%s_%s", 
                argv[1], LOG_FILE_BASE, time_buffer);

    // printf("Write to file '%s'\n", write_file);
    // closelog();
    // exit(0);

    /* 
     * Open a connection to the file, create new file if it doesn't exist and
     * overwrite file if it does exist.
     */
    FILE *p_file;
    int ret_val = 0;
    p_file = fopen(write_file, "w");

    if (p_file == NULL) {
        syslog(LOG_ERR, "Failed to open %s: %m\n", write_file);
        return 1;
    }

    // File opened succesfully, write to file
    snprintf(write_str, sizeof(write_str), "%s - %s\n", 
                "Starting Snoop", time_buffer);
    ret_val = fprintf(p_file, "%s", write_str);

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

    // TOREMOVE: Sleep for 10 seconds
    sleep(10);

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
    ret_val = fprintf(p_file, "%s", write_str);

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

    // Close out connection to the file descriptor if file was opened
    if (p_file) {
        fclose(p_file);
    }

    // Close out connection to system logger
    closelog();

    return 0;
}