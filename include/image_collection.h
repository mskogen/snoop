/**
​*​ ​@file​ image_collection.h
​*​ ​@brief​ Header file for Image Collection module
​*
​*​ ​@author​ ​Matthew Skogen
​*​ ​@date​ April 27 ​2023
​*/
#ifndef __IMAGE_COLLECTION_H__
#define __IMAGE_COLLECTION_H__

#define MUTEX_FAILURE   (2)

// Initialize camera content, creates new data directory to store all data to
// and initializes important global variables
int init_camera(char *base_dir, char *host, char *port, int frame_rate);

// Captures a single .jpeg formatted image via http request
int capture_image();

// Converts .jpg images to .mp4 images via ffmpeg utility
int convert_to_video();

// Creates new folder to log image data to
int new_folder();

// Destroys mutex setup during initialization for proper cleanup
void destroy_mutex();

#endif // __IMAGE_COLLECTION_H__