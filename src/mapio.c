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
  
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666); //nom de la sauvegarde pris en argument
	
	// TO DO Cas d'erreur lors de l'ouverture du file descriptor
	
	int buf;
	
	//Ecriture des paramètres de bases
	buf = map_width();
	write(fd, &buf, sizeof(int)); //largeur
	buf = map_height();
	write(fd, &buf, sizeof(int)); //hauteur
	buf = map_objects();
	write(fd, &buf, sizeof(int)); //nb d'objets chargés en mémoire

	//Ecriture des objets chargés (avant la matrice de la map -> plus compliqué d'y accéder par la suite mais plus facile d'y ajouter des cases)
	int obj_name_length;	
	for(int obj_id=0; obj_id<map_objects(); obj_id++){
	  obj_name_length = (int) strlen(map_get_name(obj_id));
		write(fd, &obj_name_length, sizeof(int)); //taille du chemin vers le fichier, sert pour le load avec lseek
		write(fd, map_get_name(obj_id), obj_name_length * sizeof(char)); //chemin du fichier, FONCTIONNE???

		buf = map_get_frames(obj_id);
		write(fd, &buf, sizeof(int));
		buf = map_get_solidity(obj_id);
		write(fd, &buf, sizeof(int));
		buf = map_is_destructible(obj_id);
		write(fd, &buf, sizeof(int));
		buf = map_is_collectible(obj_id);
		write(fd, &buf, sizeof(int));
		buf = map_is_generator(obj_id);
		write(fd, &buf, sizeof(int));		
	}
	
	//Ecriture des blocs présents (matrice)
	for(int x=0; x<map_width(); x++){
		for(int y=0; y<map_height(); y++){
		  if(map_get(x,y) != -1){
				//coordonnees objet
				write(fd, &x, sizeof(int));
				write(fd, &y, sizeof(int));
				//id objet
				buf = map_get(x,y);
				write(fd, &buf, sizeof(int));
			}
		}
	}
	
	close(fd);

}

void map_load (char *filename)
{
  // TODO
 // exit_with_error ("Map load is not yet implemented\n");
	
	int fd = open(filename, O_RDONLY);
	//TO DO Cas d'erreur
	
	char* buffer = NULL;

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
