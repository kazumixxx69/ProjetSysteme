#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"

#define NB_TYPES 8

#ifdef PADAWAN

void map_new (unsigned width, unsigned height)
{
  map_allocate (width, height);

  for (int x = 0; x < width; x++)
    map_set (x, height - 1, 0); // Ground

  for (int y = 0; y < height - 1; y++) {
    map_set (0, y, 1); // Wall
    map_set (width - 1, y, 1); // Wall
  }

  map_object_begin (NB_TYPES);

  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
  // Mur
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
  // Gazon
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Marbre
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
  // Herbe
  map_object_add ("images/herb.png", 1, MAP_OBJECT_AIR);
  // Petit plancher flottant
  map_object_add ("images/floor.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Fleur
  map_object_add("images/flower.png", 1, MAP_OBJECT_AIR);
  // Pièce
  map_object_add("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE);

  map_object_end ();
}

void map_save (char *filename)
{
  // TODO
  //fprintf (stderr, "Sorry: Map save is not yet implemented\n");
  
	int fd = open("maps/saved.map", O_WRONLY | O_CREAT, S_IWRITE);
	write(fd, map_width(), sizeof(int)); //largeur
	write(fd, map_height(), sizeof(int)); //hauteur
	write(fd, map_objects(), sizeof(int));

	for(int i=0; i<map_width(); i++){
		for(int j=0; j<map_height(); j++){
			if(map_get(i,j) != -1){
				write(fd, i, sizeof(int));
				write(fd, j, sizeof(int));
				write(fd, map_get(i,j), sizeof(int));
			}
		}
	}
	
	//stocker propriétés sous forme de int/bool
}

void map_load (char *filename)
{
  // TODO
 // exit_with_error ("Map load is not yet implemented\n");
	
	int fd = open("maps/saved.map", O_RDONLY, S_IREAD);
	char* buffer;

	read(fd, buffer, sizeof(int));
	int width = atoi(buffer);
	read(fd, buffer, sizeof(int));
	int height = atoi(buffer);
	
	map_new(width, height);

	read(fd, buffer, sizeof(int));
	int nbObjects = atoi(buffer);

	for(int i=0; i<nbObjects; i++){
		read(fd, buffer, sizeof(int));
		int x = atoi(buffer);
		read(fd, buffer, sizeof(int));
		int y = atoi(buffer);
		read(fd, buffer, sizeof(int));
		int obj = atoi(buffer);
		map_set(x, y, obj);
	}

	//caractéristiques
}

#endif
