/*
 * Source: This module was adapted from the httpclient.c application hosted at: 
 * https://www.theinsanetechie.in/2014/02/a-simple-http-client-and-server-in-c.html
 *
 */
#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"netdb.h"


int get_request(char * url, char * port);
int isValidIP(char * ip);
int parseHeader(char * header);
char * splitKeyValue(char * line, int index);
void openFile();
