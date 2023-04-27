/*
 * Source: This module was adapted from the httpclient.c application hosted at: 
 * https://www.theinsanetechie.in/2014/02/a-simple-http-client-and-server-in-c.html
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http_client.h"
  
#define BUF_SIZE 1024 

FILE *fileptr;
// char keys[][25] = {"Date: ", "Hostname: ", "Location: ", "Content-Type: "};
char keys[][25] = {"Content-Type: ", "Transfer-Encoding: "};
char status[4] = {0, 0, 0, 0};
char contentFileType[100];
char path[1000];
static int num_files = 0;

int get_request(char *url, char *port) {
    int sockfd;
    // char *ptr;
    // char *host;
    char getrequest[1024];
    struct sockaddr_in addr;

    sprintf(getrequest, "GET /capture HTTP/1.1\r\nHost: %s\n\n", url);
    // if (isValidIP(url)) { //when an IP address is given
    //     sprintf(getrequest, "GET /capture HTTP/1.0\nHOST: %s\n\n", url);
    // } else { //when a host name is given
    //     if ((ptr = strstr(url, "/")) == NULL) {
    //         //when hostname does not contain a slash
    //         sprintf(getrequest, "GET /capture HTTP/1.0\nHOST: %s\n\n", url);
    //     } else {
    //         //when hostname contains a slash, it is a path to file
    //         strcpy(path, ptr);
    //         host = strtok(url, "/");
    //         sprintf(getrequest, "GET %s HTTP/1.0\nHOST: %s\n\n", path, url);
    //     }
    // } 

    // creates a socket to the host
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {  
        printf("Error creating socket!\n");  
        exit(1);  
    }  
    printf("Socket created...\n");

    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_addr.s_addr = inet_addr(url);
    addr.sin_port = htons(atoi(port));

    if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
        printf("Connection Error!\n");
        exit(1);
    }
    printf("Connection successful...\n\n\n");
    // ptr = strtok(path, "/");
    // strcpy(path, ptr);
    //printf("path=%s\n", path); 
    //fileptr = fopen(path, "w");
    //strcpy(fileName, path);
    //sprintf(fileName, "%s", path);

        //int optval = 1;
        //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // writes the HTTP GET Request to the sockfd
    write(sockfd, getrequest, strlen(getrequest));

    return sockfd;
}


int isValidIP(char * ip) {
    struct sockaddr_in addr;
    int valid = inet_pton(AF_INET, ip, &(addr.sin_addr));
    return valid != 0;
}


int parseHeader(char * header) {
    //"Date: %sHostname: %s:%d\nLocation: %s\nContent-Type: %s\n\n"
    char *line, *value;
    // char *key;
    // char temp[100];
    int i = 0, num_hdr_fields = 0;
    line = strtok(header, "\n");
    while (line != NULL) {
        // memset(temp, 0, sizeof(temp));
        printf("parseHeader_line = [%s]\n", line);
        // strcpy(temp, line);
        value = splitKeyValue(line, i);  
        if (i == 1) {   
            strcpy(contentFileType, value);
        }
        printf("value=[%s]\n", value);
        line = strtok(NULL, "\n");
        i++; 
    }
    num_hdr_fields = sizeof(keys) / sizeof(keys[0]);
    printf("num_hdr_fields is %i\n", num_hdr_fields);
    for (i = 0; i < num_hdr_fields; i++) {
        printf("status[%d]=%d\n", i, status[i]);
        if (status[i] == 0) {
            return 1;
        }
    }
    return 0;
}

char* splitKeyValue(char * line, int index) {
    char *temp;
    if ((temp = strstr(line, keys[index])) != NULL) {
        temp = temp + strlen(keys[index]);
        status[index] = 1;
    }
    return temp;
}

// void openFile() {
//     char * temp;
//     char command[100];
//     char fileName[1000];
//     strcpy(fileName, path);
//     //printf("File Name: %s\n", fileName);
//     //printf("Content Type: %s\n", contentFileType);
//     if ((temp = strstr(contentFileType, "text/html")) != NULL) {
//         if ((temp = strstr(fileName, ".txt")) != NULL) {
//             sprintf(command, "gedit %s", fileName);
//         } else {
//             sprintf(command, "firefox %s", fileName);
//         }
//         system(command);
//     } else if ((temp = strstr(contentFileType, "application/pdf")) != NULL) {
//         sprintf(command, "acroread %s", fileName);
//         system(command);
//     } else {
//         printf("The filetype %s is not supported. Failed to open %s!\n", contentFileType, fileName);
//     }
// }

int get_http_jpeg(char *url, char *portNumber, char* baseDir)
{
    // struct sockaddr_in addr, cl_addr;
    int sockfd, ret;
    // struct hostent *server;
    char *temp;
    int path_length;
    char fileName[64];
    char filePath[256];
    char status_ok[] = "OK";
    char buffer[BUF_SIZE];
    // char http_not_found[] = "HTTP/1.0 404 Not Found";
    char http_ok[] = "HTTP/1.1 200 OK";
    // char location[] = "Location: ";
    // char contentType[] = "Content-Type: ";
    // int sPos, ePos;

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

    sockfd = get_request(url, portNumber); 

    // Check that HTTP status
    memset(&buffer, 0, sizeof(buffer));
    ret = recv(sockfd, buffer, sizeof(http_ok)+1, 0);  
    if (ret < 0) {  
        printf("Error receiving HTTP status!\n");    
    } else {
        printf("HTTP status - \n");
        printf("%s\n", buffer);
        if ((temp = strstr(buffer, http_ok)) != NULL) {
            send(sockfd, status_ok, strlen(status_ok), 0);
        } else {
            close(sockfd);
            return 1;
        }
    }

    // Check that HTTP header
    memset(&buffer, 0, sizeof(buffer)); 
    ret = recv(sockfd, buffer, 2*BUF_SIZE, 0);  
    if (ret < 0) {  
        printf("Error receiving HTTP header!\n");    
    } else {
        printf("HTTP header - \n");
        printf("%s\n", buffer);
        if (parseHeader(buffer) == 0) {
            send(sockfd, status_ok, strlen(status_ok), 0);
        } else {
            printf("Error in HTTP header!\n");
            close(sockfd);
            return 1;
        }
    }

    // Put together filename
    printf("putting together filename\n");
    memset(fileName, 0, sizeof(fileName));
    memset(filePath, 0, sizeof(filePath));
    sprintf(fileName, "test_file_%i.jpg", num_files++);
    path_length = strlen(baseDir);
    memcpy(filePath, baseDir, path_length);
    memcpy(&filePath[path_length], fileName, strlen(fileName));
    printf("file: [%s]\n", filePath);

    // Open new file to write data to
    fileptr = fopen(filePath, "w");
    if (fileptr == NULL) {
        printf("Error opening file!\n");
        close(sockfd);
        return 1;
    }

    memset(&buffer, 0, sizeof(buffer));
    int num_pkts = 0;
    int size_pkt = 0;
    while (1) {
        // Receives the file
        size_pkt = recv(sockfd, buffer, BUF_SIZE-1, 0);

        if (size_pkt <= 0) {
            break; // Done receiving
        }
        // if ((strstr(contentFileType, "text/html")) != NULL) {
        //     fprintf(fileptr, "%s", buffer);
        // } else {
        //     fwrite(&buffer, sizeof(buffer), 1, fileptr);
        // }
        printf("[");
        for (int idx = 0; idx < size_pkt; idx++) {
            printf("%hhX", buffer[idx]);
        }
        printf("]\n");

        printf("Packet #%i w/ length %i\n", num_pkts++, size_pkt);
        // fprintf(fileptr, "%s", buffer);
        memset(&buffer, 0, sizeof(buffer));
    }

    fclose(fileptr);
    close(sockfd);
    return 0;
}