
#define MUTEX_FAILURE   (2)

int init_camera(char *base_dir, char *host, char *port, int frame_rate);

int capture_image();

int convert_to_video();

void destroy_mutex();
