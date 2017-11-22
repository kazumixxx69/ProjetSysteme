#define MAP_OBJECT_NONE      -1

#define MODE_GET_WIDTH        0
#define MODE_GET_HEIGHT       1
#define MODE_GET_OBJECTS      2
#define MODE_GET_INFO         3

#define NB_PROPERTIES         5 //6

int get_width(char* filename);
int get_height(char* filename);
int get_objects(char* filename);
int get_info(char* filename);
int get(char* filename, int mode);

int set_width(char* filename, int value);
int set_height(char* filename, int value);
