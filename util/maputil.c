#define _XOPEN_SOURCE 500

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "maputil.h"

/* -------------- FONCTIONS GET ---------------- */


//Méthode principale des fonctions "get_"

int get(char* filename, int mode){
   int status;
   int buf;
   char* property;

   switch(mode)
   {
      case MODE_GET_WIDTH:
         property = "Width";
         break;
      case MODE_GET_HEIGHT:
         property = "Height";
         break;
      case MODE_GET_OBJECTS:
         property = "Amount of objects";
         break;
      case MODE_GET_INFO:
         status = get_width(filename);
         if (status < 0){
            return status;
         }
         status = get_height(filename);
         if (status < 0){
            return status;
         }
         status = get_objects(filename);
         if (status < 0){
            return status;
         }
         return EXIT_SUCCESS;
   }

   int fd = open(filename, O_RDONLY);
   if (fd < 0){
      return fd;
   }

   //le mode passé en argument sert à la fois à indiquer quelle propriété afficher et de combien de blocs il faut se déplacer en brut
   status = lseek(fd, mode * sizeof(int), SEEK_SET);
   if (status < 0){
      close(fd);
      return status;
   }
   status = read(fd, &buf, sizeof(int));
   if (status < 0){
      close(fd);
      return status;
   }
   fprintf(stdout, "%s: %d\n", property, buf);

   close(fd);

   return EXIT_SUCCESS;
}




//les fonctions suivantes renvoient sur la get principale en donnant un mode

int get_width(char* filename){
   int status;
   status = get(filename, 0);
   return status;
}


int get_height(char* filename){
   int status;
   status = get(filename, 1);
   return status;
}


int get_objects(char* filename){
   int status;
   status = get(filename, 2);
   return status;
}


int get_info(char* filename){
   int status;
   status = get(filename, 3);
   return status;
}



/* -------------- FONCTIONS SET ---------------- */


int set_width(char* filename, int value){
   int buf = value;
   int old_value;
   int nb_objects;
   int status;

   int fd = open(filename, O_RDWR);
   if(fd < 0){
      return fd;
   }

   //on stocke une backup du fichier à (au moins essayer de) rétablir en cas d'erreur lors des appels système
   int backup_size = lseek(fd, 0, SEEK_END);
   if (backup_size < 0){
      close(fd);
      return backup_size;
   }
   char backup[backup_size];
   status = lseek(fd, 0, SEEK_SET);
   if (status < 0){
      close(fd);
      return status;
   }
   status = read(fd, &backup, backup_size);
   if (status < 0){
      close(fd);
      return status;
   }


   //on se place au début du fichier
   status = lseek(fd, 0 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &old_value, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = lseek(fd, (off_t) (-1) * sizeof(int), SEEK_CUR);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   //on remplace la width de la map
   status = write(fd, &buf, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = lseek(fd, 2 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &nb_objects, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   for (int i = 0; i < nb_objects; i++){
      int obj_name_length;
      status = read(fd, &obj_name_length, sizeof(int));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
   }

   /*si la map a gagné en largeur, on ne touche à rien : les colonnes vides sont créées automatiquement par
   le jeu en chargeant la map*/

   /*si la map a perdu en largeur, on supprime les objets qui n'y rentrent plus, 
   en décalant les objets suivants si nécessaire*/
   if((old_value - value) > 0){
      while((status = read(fd, &buf, sizeof(int))) > 0){
	status = lseek(fd, 2 * sizeof(int), SEEK_CUR);
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }         
	if (buf >= value){
            /*fonction auxiliaire qui décale de 3 entiers en arrière le bloc situé après l'élément à
            supprimer qui sera donc écrasé*/
	    decal_mat(fd, &status);
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
         }
      }
   }

   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }

   close(fd);
   return EXIT_SUCCESS;
}

int set_height(char* filename, int value){
   int buf = value;
   int old_value;
   int nb_objects;
   int status;

   int fd = open(filename, O_RDWR);
   if (fd < 0){
      return fd;
   }

   int backup_size = lseek(fd, 0, SEEK_END);
   if (backup_size < 0){
      close(fd);
      return backup_size;
   }
   char backup[backup_size];
   status = lseek(fd, 0, SEEK_SET);
   if (status < 0){
      close(fd);
      return status;
   }
   status = read(fd, &backup, backup_size);
   if (status < 0){
      close(fd);
      return status;
   }

   //on se positionne pour modifier la height
   status = lseek(fd, 1 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &old_value, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = lseek(fd, (off_t) (-1) * sizeof(int), SEEK_CUR);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   //on modifie la height
   status = write(fd, &buf, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &nb_objects, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   for (int i = 0; i < nb_objects; i++){
      int obj_name_length;
      status = read(fd, &obj_name_length, sizeof(int));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
   }

   /*si la hauteur a diminué, on supprime les objets qui ne sont plus contenus dans la map, 
   et on décale dans tous les cas la hauteur des objets présents sur la map (car agrandissement/
   rétrécissement effectué à l'origine)*/
   while((status = read(fd, &buf, sizeof(int))) > 0){
        status = read(fd, &buf, sizeof(int));
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }
        int new_height = buf-(old_value-value);
        status = lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }
        //on décale la hauteur de l'objet lu
	status = write(fd, &new_height, sizeof(int));
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }
	status = lseek(fd, sizeof(int), SEEK_CUR);
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }
        if(new_height < 0){
            //s'il n'est plus censé être présent sur la map on le supprime avec la fonction auxiliaire
	    decal_mat(fd, &status);
	    if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
        }
   }
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   close(fd);
   return EXIT_SUCCESS;
}


int set_objects(char* filename, int argc, char** objects_list){
   int nb_objects_old;
   int nb_objects_new = (argc)/(NB_PROPERTIES + 1);
   int status;

   //nombre de propriétés invalide
   if((nb_objects_new <= 0)||(((argc)%(NB_PROPERTIES + 1))!=0)){
      fprintf(stderr, "Error: Invalid argument format\n");
      return EXIT_FAILURE;
   }
   
   int fd = open(filename, O_RDWR);
   if (fd < 0){
      return fd;
   }

   int backup_size = lseek(fd, 0, SEEK_END);
   if (backup_size < 0){
      close(fd);
      return backup_size;
   }
   char backup[backup_size];
   status = lseek(fd, 0, SEEK_SET);
   if (status < 0){
      close(fd);
      return status;
   }
   status = read(fd, &backup, backup_size);
   if (status < 0){
      close(fd);
      return status;
   }

   //on se positionne sur le nombre d'objets
   status = lseek(fd, 2 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &nb_objects_old, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }

   //nouveau nombre d'objets inférieur au précédent (contrainte de l'exercice)
   if(nb_objects_new < nb_objects_old){
      fprintf(stderr, "Error: There must be as many or more objects than before\n");
      return EXIT_FAILURE;
   }
   //on remplace le nombre d'objets
   status = lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = write(fd, &nb_objects_new, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }

   /*on stocke le nom des anciens objets et des nouveaux objets pour les comparer et détecter
   d'éventuelles correspondances*/
   char* names_old[nb_objects_old];
   char* names_new[nb_objects_new];
   int pos = 0;
   
   //création de la liste des noms des anciens objets
   while(objects_list[pos * (NB_PROPERTIES + 1)]!=NULL){
      names_new[pos] = (char*) malloc((strlen(objects_list[pos * (NB_PROPERTIES + 1)])) * sizeof(char));
      if(names_new[pos] == NULL){
         for (int j = 0; j < pos; j++){
            free(names_new[j]);
         }
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return -1;
      }
      strcpy(names_new[pos], objects_list[pos * (NB_PROPERTIES + 1)]);
      pos++;
   }

   //création de la liste des noms des nouveaux objets
   int obj_name_length;
   for(pos = 0; pos < nb_objects_old; pos++){
      status = read(fd, &obj_name_length, sizeof(int));
      if (status < 0){
         for (int j = 0; j < nb_objects_new; j++){
            free(names_new[j]);
         }
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      char buffer_name;
      names_old[pos] = (char*) malloc((obj_name_length + 1) * sizeof(char));
      if(names_old[pos] == NULL){
         for (int i = 0; i < pos; i++){
            free(names_old[i]);
         }
         for (int j = 0; j < nb_objects_new; j++){
            free(names_new[j]);
         }
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      for(int i = 0; i < obj_name_length; i++){
         status = read(fd, &buffer_name, sizeof(char));
         if (status < 0){
            for (int i = 0; i <= pos; i++){
               free(names_old[i]);
            }
            for (int j = 0; j < nb_objects_new; j++){
               free(names_new[j]);
            }
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         names_old[pos][i] = buffer_name;
      }
      names_old[pos][obj_name_length] = '\0';
      status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
      if (status < 0){
         for (int i = 0; i <= pos; i++){
            free(names_old[i]);
         }
         for (int j = 0; j < nb_objects_new; j++){
            free(names_new[j]);
         }
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
   }

   //on crée un tableau de correspondance ayant pour taille le nombre des anciens objets (car inférieur ou égal)
   int replace[nb_objects_old];
   for (int i = 0; i < nb_objects_old; i++){
      replace[i] = -1;
   }

   /*pour chaque ancien nom d'objet, s'il y a correspondance avec un nouveau nom, on considère qu'il est
   encore présent et on stocke à la position correspondant à son ID l'ID du nouvel objet correspondant
   pour convertir plus tard les éléments déjà présents. S'il n'y a pas de correspondance, on a -1 et les
   objets correspondant à l'ID de l'ancien objet seront supprimés.*/
   for(int i = 0; i < nb_objects_old; i++){
      for(int j = 0; j < nb_objects_new; j++){
         if(strcmp(names_old[i], names_new[j]) == 0){
            replace[i] = j;
         }
      }
   }

   for (int i = 0; i < nb_objects_old; i++){
      free(names_old[i]);
   }
   for (int j = 0; j < nb_objects_new; j++){
      free(names_new[j]);
   }

   off_t cur_position;
   off_t file_end;

   //on stocke dans un buffer la matrice des objets de la map pour la réécrire plus tard après la liste des nouveaux objets dans le fichier
   cur_position = lseek(fd, 0, SEEK_CUR);
   if (cur_position < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return cur_position;
   }
   file_end = lseek(fd, 0, SEEK_END);
   if (file_end < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return file_end;
   }
   char array[file_end - cur_position];
   status = lseek(fd, cur_position, SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &array, file_end - cur_position);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }


   //on écrit le nom et les propriétés des nouveaux objets dans le fichier en écrasant les données précédentes
   status = lseek(fd, 3 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   for(int i = 0; i < nb_objects_new; i++){
      obj_name_length = (int) strlen(objects_list[i * (NB_PROPERTIES + 1)]);
      status = write(fd, &obj_name_length, sizeof(int));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = write(fd, (objects_list[i * (NB_PROPERTIES + 1)]), obj_name_length * sizeof(char));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }

      int nb_frames = atoi(objects_list[i * (NB_PROPERTIES + 1) + 1]);
      status = write(fd, &nb_frames, sizeof(int));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }

      //pas trouvé d'autres solutions que de comparer les chaînes de caractères pour récupérer les propriétés
      int prop;
      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "air") == 0){
         prop = MAP_OBJECT_AIR;
         status = write(fd, &prop, sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            write(fd, &backup, backup_size);
            close(fd);
            return status;
         }
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "semi-solid") == 0){
            prop = MAP_OBJECT_SEMI_SOLID;
            status = write(fd, &prop, sizeof(int));
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
         }
         else{
            if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "solid") == 0){
               prop = MAP_OBJECT_SOLID;
               status = write(fd, &prop, sizeof(int));
               if (status < 0){
                  lseek(fd, 0, SEEK_SET);
                  write(fd, &backup, backup_size);
                  ftruncate(fd, lseek(fd, 0, SEEK_CUR));
                  close(fd);
                  return status;
               }
            }
            else{
               if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "liquid") == 0){
                  prop = MAP_OBJECT_LIQUID;
                  status = write(fd, &prop, sizeof(int));
                  if (status < 0){
                     lseek(fd, 0, SEEK_SET);
                     write(fd, &backup, backup_size);
                     ftruncate(fd, lseek(fd, 0, SEEK_CUR));
                     close(fd);
                     return status;
                  }
               }
               else{
                  return EXIT_FAILURE;
               }
            }
         }
      }

      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 3], "destructible") == 0){
         prop = 1;
         status = write(fd, &prop, sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 3], "not-destructible") == 0){
            prop = 0;
            status = write(fd, &prop, sizeof(int));
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
         }
         else{
            return EXIT_FAILURE;
         }
      }

      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 4], "collectible") == 0){
         prop = 1;
         status = write(fd, &prop, sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 4], "not-collectible") == 0){
            prop = 0;
            status = write(fd, &prop, sizeof(int));
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
         }
         else{
            return EXIT_FAILURE;
         }
      }

      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 5], "generator") == 0){
         prop = 1;
         status = write(fd, &prop, sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 5], "not-generator") == 0){
            prop = 0;
            status = write(fd, &prop, sizeof(int));
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
         }
         else{
            return EXIT_FAILURE;
         }
      }
   }

   //on réécrit l'ancienne matrice à la suite des nouveaux objets
   status = write(fd, &array, file_end - cur_position);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = lseek(fd, 0, SEEK_CUR);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   int trunc_err;
   trunc_err = ftruncate(fd, status);
   if (trunc_err < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return trunc_err;
   }
   
   status = lseek(fd, (off_t) -1 * (file_end - cur_position), SEEK_CUR);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   

   //on modifie la matrice
   int buf;
   while((status = (read(fd, &buf, sizeof(int)))) > 0){
      status = lseek(fd, 1 * sizeof(int), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = read(fd, &buf, sizeof(int));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      //s'il y a une correspondance d'ID on la change en conséquence
      if (replace[buf] != -1){
         status = lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = write(fd, &(replace[buf]), sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
      }
      //sinon on supprime l'objet
      else{      
         decal_mat(fd, &status);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
      }
   }
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }

   close(fd);
   
   return EXIT_SUCCESS;
}


int prune_objects(char* filename){
   int nb_objects;
   int buf;
   int status;

   int fd = open(filename, O_RDWR);
   if (fd < 0){
      return fd;
   }

   int backup_size = lseek(fd, 0, SEEK_END);
   if (backup_size < 0){
      close(fd);
      return backup_size;
   }
   char backup[backup_size];
   status = lseek(fd, 0, SEEK_SET);
   if (status < 0){
      close(fd);
      return status;
   }
   status = read(fd, &backup, backup_size);
   if (status < 0){
      close(fd);
      return status;
   }

   //on se place directement sur le début de la matrice d'objets
   status = lseek(fd, 2 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = read(fd, &nb_objects, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   for (int i = 0; i < nb_objects; i++){
      int obj_name_length;
      status = read(fd, &obj_name_length, sizeof(int));
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
      status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
      if (status < 0){
         lseek(fd, 0, SEEK_SET);
         write(fd, &backup, backup_size);
         ftruncate(fd, lseek(fd, 0, SEEK_CUR));
         close(fd);
         return status;
      }
   }

   //on crée un tableau de présence
   int presence[nb_objects];
   for (int i = 0; i < nb_objects; i++){
      presence[i] = 0;
   }
   

   //on parcourt tous les objets présents et on incrémente la case du tableau de position l'ID de l'objet
   while((status = read(fd, &buf, sizeof(int))) > 0){
        status = lseek(fd, sizeof(int), SEEK_CUR);
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }
	status = read(fd, &buf, sizeof(int));
        if (status < 0){
           lseek(fd, 0, SEEK_SET);
           write(fd, &backup, backup_size);
           ftruncate(fd, lseek(fd, 0, SEEK_CUR));
           close(fd);
           return status;
        }
        presence[buf]++;
   }
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }

   int nb_objects_new = nb_objects;
   //décalage par rapport aux IDs de base
   int dec = 0;
   

   //on modifie la liste des objets de la map
   status = lseek(fd, 3 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   for(int i = 0; i < nb_objects; i++){
      //si l'objet n'est pas présent, on l'écrase entièrement avec tout ce qu'il y a derrière
      if(presence[i] == 0){
         //un objet en moins
         nb_objects_new--;
         off_t cur_position = lseek(fd, 0, SEEK_CUR);
         if (cur_position < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return cur_position;
         }
         off_t new_position;
         int obj_name_length;
         status = read(fd, &obj_name_length, sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         new_position = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
         if (new_position < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return new_position;
         }
         for(int j = i+1; j < nb_objects; j++){
	    status = read(fd, &obj_name_length, sizeof(int));
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
            status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
            status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
	 }
         //comme on supprime un objet, l'ID des objets situés après diminue de 1 et on doit modifier la matrice pour refléter ce changement
         while((status = (read(fd, &buf, sizeof(int)))) > 0){
	    status = lseek(fd, sizeof(int), SEEK_CUR);
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
	    status = read(fd, &buf, sizeof(int));
            if (status < 0){
               lseek(fd, 0, SEEK_SET);
               write(fd, &backup, backup_size);
               ftruncate(fd, lseek(fd, 0, SEEK_CUR));
               close(fd);
               return status;
            }
	    if(buf > (i - dec)){
	       buf--;
	       status = lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
               if (status < 0){
                  lseek(fd, 0, SEEK_SET);
                  write(fd, &backup, backup_size);
                  ftruncate(fd, lseek(fd, 0, SEEK_CUR));
                  close(fd);
                  return status;
               }
	       status = write(fd, &buf, sizeof(int));
               if (status < 0){
                  lseek(fd, 0, SEEK_SET);
                  write(fd, &backup, backup_size);
                  ftruncate(fd, lseek(fd, 0, SEEK_CUR));
                  close(fd);
                  return status;
               }
	    }
	 }
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         //on écrase l'objet à supprimer
         off_t file_end = lseek(fd, 0, SEEK_END);
         if (file_end < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return file_end;
         }
         char buffer[file_end - new_position];
         status = lseek(fd, new_position, SEEK_SET);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = read(fd, &buffer, file_end - new_position);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = lseek(fd, cur_position, SEEK_SET);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = write(fd, &buffer, file_end - new_position);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = lseek(fd, 0, SEEK_CUR);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
	 int trunc_err;
         trunc_err = ftruncate(fd, status);
         if (trunc_err < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return trunc_err;
         }
         status = lseek(fd, cur_position, SEEK_SET);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
	 dec++;
      }
      //s'il est présent on passe à l'objet suivant
      else{
         int obj_name_length;
         status = read(fd, &obj_name_length, sizeof(int));
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
         status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
         if (status < 0){
            lseek(fd, 0, SEEK_SET);
            write(fd, &backup, backup_size);
            ftruncate(fd, lseek(fd, 0, SEEK_CUR));
            close(fd);
            return status;
         }
      }
   }

   //on écrit le nouveau nombre d'objets
   status = lseek(fd, 2 * sizeof(int), SEEK_SET);
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }
   status = write(fd, &nb_objects_new, sizeof(int));
   if (status < 0){
      lseek(fd, 0, SEEK_SET);
      write(fd, &backup, backup_size);
      ftruncate(fd, lseek(fd, 0, SEEK_CUR));
      close(fd);
      return status;
   }

   close(fd);
   return EXIT_SUCCESS;
}



/* -------------- FONCTIONS AUXILIAIRES ----------------*/


/*recopie tout le bloc situé après la position pointée par le file descriptor 3 blocs de taille int en
arrière, écrasant et supprimant ces derniers. Dans les faits on s'en sert pour supprimer un objet
de la matrice du fichier*/
void decal_mat(int fd, int* pstatus){
   (*pstatus) = lseek(fd, (off_t) -3 * sizeof(int), SEEK_CUR);
   off_t cur_position = (off_t) (*pstatus);
   (*pstatus) = lseek(fd, 0, SEEK_END);
   off_t file_end = (off_t) (*pstatus);
   char buffer[file_end - cur_position - 3 * sizeof(int)];
   lseek(fd, cur_position + 3 * sizeof(int), SEEK_SET);
   (*pstatus) = read(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
   lseek(fd, cur_position, SEEK_SET);
   write(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
   ftruncate(fd, file_end - 3 * sizeof(int));
   lseek(fd, cur_position, SEEK_SET);
}



/* -------------- MAIN ----------------*/


int main(int argc, char* argv[]){
   if(argc <= 2){
      fprintf(stderr, "Usage: %s <filename> <options> [args]\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   char* filename = argv[1];
   int status;

   if(strcmp(argv[2], "--getwidth") == 0){
	status = get_width(filename);
        //en cas d'erreur lors de l'appel de la fonction, on affiche le code d'erreur
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--getheight") == 0){
	status = get_height(filename);
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--getobjects") == 0){
	status = get_objects(filename);
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--getinfo") == 0){
	status = get_info(filename);
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--setwidth") == 0){
	status = set_width(filename, atoi(argv[3]));
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--setheight") == 0){
	status = set_height(filename, atoi(argv[3]));
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--setobjects") == 0){
	status = set_objects(filename, argc - 3, argv + 3);
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }
   if(strcmp(argv[2], "--pruneobjects") == 0){
	status = prune_objects(filename);
        if(status < 0){
           fprintf(stderr, "Error: %s code %d\n", argv[2], status);
        }
        return status;
   }

   //option en argument non reconnue
   fprintf(stderr, "Error: Invalid option\n");

   return EXIT_SUCCESS;
}
