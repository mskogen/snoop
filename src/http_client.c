/*
 * Source: This module was adapted from the httpclient.c application hosted at: 
 * https://www.theinsanetechie.in/2014/02/a-simple-http-client-and-server-in-c.html
 *
 */
#include "http_client.h"
  
#define BUF_SIZE 1024 

FILE * fileptr;
char keys[][25] = {"Date: ", "Hostname: ", "Location: ", "Content-Type: "};
char status[4] = {0, 0, 0, 0};
char contentFileType[100];
char path[1000];

int get_request(char * url, char * port) {
    int sockfd, bindfd;
        char * ptr, * host;
    char getrequest[1024];
        struct sockaddr_in addr;

    if (isValidIP(url)) { //when an IP address is given
        sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", url);
    } else { //when a host name is given
        if ((ptr = strstr(url, "/")) == NULL) {
            //when hostname does not contain a slash
            sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", url);
        } else {
            //when hostname contains a slash, it is a path to file
            strcpy(path, ptr);
                    host = strtok(url, "/");
            sprintf(getrequest, "GET %s HTTP/1.0\nHOST: %s\n\n", path, url);
        }
    } 

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
    ptr = strtok(path, "/");
    strcpy(path, ptr);
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
    char * line, * key, * value;
    char temp[100];
    int i = 0;
    line = strtok(header, "\n");
    while (line != NULL) {
    //printf("%s\n", line);
    strcpy(temp, line);
    value = splitKeyValue(line, i);  
    if (i == 3) {   
    strcpy(contentFileType, value);
    }
    //printf("value=%s\n", value);
    line = strtok(NULL, "\n");
    i++; 
    }
    for (i = 0; i < 4; i++) {
    if (status[i] == 0) return 1;
    //printf("status[%d]=%d\n", i, status[i]);
    }
    return 0;
}

char * splitKeyValue(char * line, int index) {
    char * temp;
    if ((temp = strstr(line, keys[index])) != NULL) {
        temp = temp + strlen(keys[index]);
        status[index] = 1;
    }
    return temp;
}

void openFile() {
    char * temp;
    char command[100];
    char fileName[1000];
    strcpy(fileName, path);
    //printf("File Name: %s\n", fileName);
    //printf("Content Type: %s\n", contentFileType);
    if ((temp = strstr(contentFileType, "text/html")) != NULL) {
        if ((temp = strstr(fileName, ".txt")) != NULL) {
            sprintf(command, "gedit %s", fileName);
        } else {
            sprintf(command, "firefox %s", fileName);
        }
        system(command);
    } else if ((temp = strstr(contentFileType, "application/pdf")) != NULL) {
        sprintf(command, "acroread %s", fileName);
        system(command);
    } else {
        printf("The filetype %s is not supported. Failed to open %s!\n", contentFileType, fileName);
    }
}