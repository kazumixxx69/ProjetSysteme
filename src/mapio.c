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
  map_new(map_width(), map_height());

  int fd = open("../maps/saved.map", O_WRONLY | O_CREAT, S_IWRITE);
  write(fd, map_width(), sizeof(int)); //écrit la largeur
  write(fd, map_height(), sizeof(int)); //écrit la hauteur
  write(fd, map_objects(), sizeof(int)); //écrit le nombre d'objets qu'elle contient
  
  for(int i = 0; i<map_width(); i++){
	for(int j=0; j<map_height(); j++){
		write(fd, map_get(i, j), sizeof(int)); //on renvoie le int du type de l'objet
	}
  }

  //tableaux temp en attendant de trouver mieux
  char[NB_TYPES] *nbNames = {"images/ground.png", "images/wall.png", "images/grass.png", "images/marble.png", "images/herb.png", "images/floor.png", "images/flower.png", "images/coin.png"};
  int[NB_TYPES] nbSprites = {1, 1, 1, 1, 1, 1, 1, 20};
  char[NB_TYPES] *nbSolidity = {MAP_OBJECT_SOLID, MAP_OBJECT_SOLID, MAP_OBJECT_SEMI_SOLID, MAP_OBJECT_SOLID, MAP_OBJECT_AIR, MAP_OBJECT_SEMI_SOLID, MAP_OBJECT_AIR, MAP_OBJECT_AIR};
  int[NB_TYPES] nbDesructible = {0, 0, 0, 1, 0, 0, 0, 0};
  int[NB_TYPES] nbCollectible = {0, 0, 0, 0, 0, 0, 0, 1};
  int[NB_TYPES] nbGenerator = {0, 0, 0, 0, 0, 0, 0, 0};

  char* tmp;

  for(int i = 0; i<NB_TYPES; i++){
	//caractéristiques de chaque type : nom/chemin, nombre de sprites, propriétés
	*tmp = nbNames[i] + itoa(nbSprites[i]) + nbSolidity[i] + itoa(nbDestructible[i]) + itoa(nbCollectible[i]) + itoa(nbGenerator[i]); //concaténer mieux
	write(fd, tmp, strlen(tmp));
  }

  close(fd);
  
}

void map_load (char *filename)
{
  // TODO
 // exit_with_error ("Map load is not yet implemented\n");
	
    int fd = open("../maps/saved.map", O_RDONLY, S_IREAD);
    char* buffer;
    
    read(fd, buffer, sizeof(int));//largeur
    int width = atoi(buffer);
    read(fd, buffer, sizeof(int));//hauteur
    int height = atoi(buffer);
	
    map_new(width, height);
	
    read(fd, buffer, sizeof(int));//nombre d'objets
    int nbObjects = atoi(buffer);
    
    int matrixObjects[width][height];
    for(int i = 0; i<width(); i++){
	  for(int j=0; j<height(); j++){
		read(fd, buffer, sizeof(int)); //on renvoie le int du type de l'objet
		matrixObjects[i][j] = buffer;
	  }
    }
    
    //caractéristiques des types d'objet (nom/chemin, nbsprites, propriétés)
    char[NB_TYPES] *nbNames;
    int[NB_TYPES] nbSprites;
    char[NB_TYPES] *nbSolidity;
    int[NB_TYPES] nbDesructible;
    int[NB_TYPES] nbCollectible;
    int[NB_TYPES] nbGenerator;
    
    for(int i = 0; i<NB_TYPES; i++){
        for(int j = 0; j<6; j++){
            while(!strchr(' ', *buffer)){//sans séparateur, nécessité d'une autre condition ?
                read(fd, buff, sizeof(char));//lseek pour chercher la valeur voulue ?
            }
            switch(j){
                case 0:
                    nbNames[i] = buffer; break;
                case 1:
                    nbSprites[i] = buffer; break;
                case 2:
                    nbSolidity[i] = buffer; break;
                case 3:
                    nbDestructible[i] = buffer; break;
                case 4:
                    nbCollectible[i] = buffer; break;
                case 5:
                    nbGenerator[i] = buffer; break;
            }
        }
    }
    
    close(fd);
}

#endif
