/*
 * Source: This module was adapted from the httpclient.c application hosted at: 
 * https://www.theinsanetechie.in/2014/02/a-simple-http-client-and-server-in-c.html
 *
 */

int get_request(char *url, char *port);
int isValidIP(char *ip);
int parseHeader(char *header);
char * splitKeyValue(char *line, int index);
// void openFile();
int get_http_jpeg(char *url, char *portNumber, char *baseDir);
