#define MAP_OBJECT_NONE      -1

#define MAP_OBJECT_AIR           0
#define MAP_OBJECT_SEMI_SOLID    (1UL << 0)
#define MAP_OBJECT_SOLID         (1UL << 1)
#define MAP_OBJECT_LIQUID        (1UL << 2)
// unused slot                   (1UL << 3)
// unused slot                   (1UL << 4)
// unused slot                   (1UL << 5)
#define MAP_OBJECT_DESTRUCTIBLE  (1UL << 6)
#define MAP_OBJECT_COLLECTIBLE   (1UL << 7)
#define MAP_OBJECT_GENERATOR     (1UL << 8)
#define MAP_OBJECT_POWERUP       (1UL << 9)

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
int set_objects(char* filename, int argc, char** objects_list);

int prune_objects(char* filename);

void decal_mat(int fd, int* pstatus);
