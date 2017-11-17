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
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
	
	//Cas d'erreur lors de l'ouverture du file descriptor
        if (fd < 0){
           exit_with_error("Error while saving your map\n");
        }
	
	int buf; //buffer
        int err;
        
	//Ecriture des paramètres de bases
	buf = map_width();
	err = write(fd, &buf, sizeof(int)); //largeur
        if(err < 0) exit_with_error("Error while saving you map\n");
        
	buf = map_height();
	err = write(fd, &buf, sizeof(int)); //hauteur
        if(err < 0) exit_with_error("Error while saving you map\n");
        
	buf = map_objects();
	err = write(fd, &buf, sizeof(int)); //nb d'objets chargés en mémoire
        if(err < 0) exit_with_error("Error while saving you map\n");

	//Ecriture des objets chargés (avant la matrice de la map -> plus compliqué d'y accéder par la suite mais plus facile d'y ajouter des cases)
	int obj_name_length;	
	for(int obj_id=0; obj_id<map_objects(); obj_id++){
           obj_name_length = (int) strlen(map_get_name(obj_id));
           err = write(fd, &obj_name_length, sizeof(int)); //taille du chemin vers le fichier, sert pour le load avec lseek
           if(err < 0) exit_with_error("Error while saving you map\n");
           
           err = write(fd, map_get_name(obj_id), obj_name_length * sizeof(char)); //chemin du fichier
           if(err < 0) exit_with_error("Error while saving you map\n");

           buf = (int) map_get_frames(obj_id);
           err = write(fd, &buf, sizeof(int));
           if(err < 0) exit_with_error("Error while saving you map\n");
           
           buf = map_get_solidity(obj_id);
           err = write(fd, &buf, sizeof(int));
           if(err < 0) exit_with_error("Error while saving you map\n");
           
           buf = map_is_destructible(obj_id);
           err = write(fd, &buf, sizeof(int));
           if(err < 0) exit_with_error("Error while saving you map\n");
           
           buf = map_is_collectible(obj_id);
           err = write(fd, &buf, sizeof(int));
           if(err < 0) exit_with_error("Error while saving you map\n");
           
           buf = map_is_generator(obj_id);
           err = write(fd, &buf, sizeof(int));
           if(err < 0) exit_with_error("Error while saving you map\n");
	}
	
	/*Stocker un objet avec ses coordonnées évite d'écrire tous les blocs vides 
	de la map et permet une meilleure adaptation en cas de modification de la 
	taille de la map*/
	//Ecriture des blocs présents (matrice)
	for(int x=0; x<map_width(); x++){
		for(int y=0; y<map_height(); y++){
		  if(map_get(x,y) != -1){
				//coordonnees objet
				err = write(fd, &x, sizeof(int));
                                if(err < 0) exit_with_error("Error while saving you map\n");
				err = write(fd, &y, sizeof(int));
                                if(err < 0) exit_with_error("Error while saving you map\n");
				//id objet
				buf = map_get(x,y);
				err = write(fd, &buf, sizeof(int));
                                if(err < 0) exit_with_error("Error while saving you map\n");
			}
		}
	}

  //Fermeture du fichier
	close(fd);
}



void map_load (char *filename)
{
        int fd = open(filename, O_RDONLY);
        
	//Cas d'erreur lors de l'ouverture du file descriptor
        if (fd < 0){
           exit_with_error("Error while loading your map\n");
        }
        
	int buffer;

	read(fd, &buffer, sizeof(int));//on récupère
	int width = buffer;	       //la largeur
	read(fd, &buffer, sizeof(int));//on récupère
	int height = buffer;	       //la hauteur
	
	map_allocate(width, height);
        
	read(fd, &buffer, sizeof(int));
	int nb_objects = buffer;//nombre d'objets différents

        map_object_begin(nb_objects);

        int obj_name_length;
        unsigned obj_frames;
        unsigned obj_solidity;
        unsigned obj_is_destructible;
        unsigned obj_is_collectible;
        unsigned obj_is_generator;
        unsigned obj_properties;
        

        for (int obj_id=0; obj_id<nb_objects; obj_id++){

	   //on récupère la taille du nom, puis le nom du fichier
           read(fd, &buffer, sizeof(int));
           obj_name_length = buffer;
           char buffer_name[obj_name_length+1];
           read(fd, &buffer_name, obj_name_length * sizeof(char));
	   fprintf(stderr, "%s\n", buffer_name);
           
	   read(fd, &buffer, sizeof(int));
           obj_frames = (unsigned) buffer;
           
	   /*les propriétés de l'objet sont stockées au format binaire
	     obj_properties évoluera en fonction des propriétés stockées
	     en accord avec les valeurs définies dans map.h*/
	   obj_properties = 0;
	   read(fd, &buffer, sizeof(int));
           obj_solidity = (unsigned) buffer;
	   obj_properties = obj_properties | obj_solidity;
           read(fd, &buffer, sizeof(int));
           obj_is_destructible = ((unsigned) buffer) * MAP_OBJECT_DESTRUCTIBLE;
	   obj_properties = obj_properties | obj_is_destructible;
           read(fd, &buffer, sizeof(int));
           obj_is_collectible = ((unsigned) buffer) * MAP_OBJECT_COLLECTIBLE;
	   obj_properties = obj_properties | obj_is_collectible;
           read(fd, &buffer, sizeof(int));
           obj_is_generator = ((unsigned) buffer) * MAP_OBJECT_GENERATOR;
	   obj_properties = obj_properties | obj_is_generator;

	   fprintf(stderr, "%d\n", obj_properties);

           //l'objet étant désormais défini, on l'ajoute
           map_object_add(buffer_name, obj_frames, obj_properties);
        }

	map_object_end();
                
        int x;
        int y;
        int obj;
                
	//on place chaque bloc stocké aux coordonnées indiquées
	while(read(fd, &buffer, sizeof(int))>0){
		x = buffer;
		read(fd, &buffer, sizeof(int));
		y = buffer;
		read(fd, &buffer, sizeof(int));
		obj = buffer;
                if((obj < -1)||(obj > nb_objects - 1)){
                   exit_with_error("Error while loading your map\n");
                }
		map_set(x, y, obj);
	}


  close(fd);
}

#endif
