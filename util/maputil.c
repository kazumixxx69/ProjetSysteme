#define _XOPEN_SOURCE 500

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "maputil.h"

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
      status = fd;
      return status;
   }

   lseek(fd, mode * sizeof(int), SEEK_SET);
   status = read(fd, &buf, sizeof(int));
   if (status < 0){
      return status;
   }
   fprintf(stdout, "%s: %d\n", property, buf);

   close(fd);

   return EXIT_SUCCESS;
}

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

int set_width(char* filename, int value){
   int buf = value;
   int old_value;
   int nb_objects;
   int status;

   int fd = open(filename, O_RDWR);

   //dup2(fd, backup);

   status = lseek(fd, 0 * sizeof(int), SEEK_SET);
   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
   status = read(fd, &old_value, sizeof(int));
   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
   status = lseek(fd, (off_t) (-1) * sizeof(int), SEEK_CUR);
   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
   status = write(fd, &buf, sizeof(int));
   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
   status = lseek(fd, 2 * sizeof(int), SEEK_SET);
   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
   status = read(fd, &nb_objects, sizeof(int));
   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
   for (int i = 0; i < nb_objects; i++){
      int obj_name_length;
      status = read(fd, &obj_name_length, sizeof(int));
      if (status < 0){
         //dup2(backup, fd);
         //close(backup);
         close(fd);
         return status;
      }
      status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
      if (status < 0){
         //dup2(backup, fd);
         //close(backup);
         close(fd);
         return status;
      }
      status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
      if (status < 0){
         //dup2(backup, fd);
         // close(backup);
         close(fd);
         return status;
      }
   }

   fprintf(stderr, "old val %d\n", old_value);
   fprintf(stderr, "val %d\n", value);

   if((old_value - value) > 0){
      while((status = read(fd, &buf, sizeof(int))) > 0){
	fprintf(stderr, "x: %d\n", buf);
	lseek(fd, 2 * sizeof(int), SEEK_CUR);         
	if (buf >= value){
            status = lseek(fd, (off_t) -3 * sizeof(int), SEEK_CUR);
            if (status < 0){
               //dup2(backup, fd);
               //close(backup);
               close(fd);
               return status;
            }
            off_t cur_position = (off_t) status;
            fprintf(stderr, "curpos: %d\n", (int) cur_position);
            status = lseek(fd, 0, SEEK_END);
            if (status < 0){
               //dup2(backup, fd);
               //close(backup);
               close(fd);
               return status;
            }
            off_t file_end = (off_t) status;
	    fprintf(stderr, "filend: %d\n", (int) file_end);
            char buffer[file_end - cur_position - 3 * sizeof(int)];
	    fprintf(stderr, "sizeofint: %d\n", (int) sizeof(char));
            lseek(fd, cur_position + 3 * sizeof(int), SEEK_SET);
            status = read(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
            if (status < 0){
               //dup2(backup, fd);
               //close(backup);
               close(fd);
               return status;
	    }
            lseek(fd, cur_position, SEEK_SET);
            write(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
            ftruncate(fd, file_end - 3 * sizeof(int));
            lseek(fd, cur_position, SEEK_SET);

         }
      }
   }

   if (status < 0){
      //dup2(backup, fd);
      //close(backup);
      close(fd);
      return status;
   }
  
   //close(backup);
   close(fd);

   return EXIT_SUCCESS;

}

int set_height(char* filename, int value){
   int buf = value;
   int old_value;
   int nb_objects;
   int status;

   int fd = open(filename, O_RDWR);

   status = lseek(fd, 1 * sizeof(int), SEEK_SET);
   status = read(fd, &old_value, sizeof(int));
   status = lseek(fd, (off_t) (-1) * sizeof(int), SEEK_CUR);
   status = write(fd, &buf, sizeof(int));
   //status = lseek(fd, 1 * sizeof(int), SEEK_SET);
   status = read(fd, &nb_objects, sizeof(int));
   for (int i = 0; i < nb_objects; i++){
      int obj_name_length;
      status = read(fd, &obj_name_length, sizeof(int));
      status = lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
      status = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
   }
  
   fprintf(stderr, "old val %d\n", old_value);
   fprintf(stderr, "val %d\n", value);

   //lseek(fd, 1 * sizeof(int), SEEK_CUR);

   while((status = read(fd, &buf, sizeof(int))) > 0){
	fprintf(stderr, "x: %d\n", buf);
        read(fd, &buf, sizeof(int));
        int new_height = buf-(old_value-value);
        lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
	write(fd, &new_height, sizeof(int));
	lseek(fd, sizeof(int), SEEK_CUR);
        if(new_height < 0){
            status = lseek(fd, (off_t) -3 * sizeof(int), SEEK_CUR);
            off_t cur_position = (off_t) status;
            fprintf(stderr, "curpos: %d\n", (int) cur_position);
            status = lseek(fd, 0, SEEK_END);
            off_t file_end = (off_t) status;
	    fprintf(stderr, "filend: %d\n", (int) file_end);
            char buffer[file_end - cur_position - 3 * sizeof(int)];
	    fprintf(stderr, "sizeofint: %d\n", (int) sizeof(char));
            lseek(fd, cur_position + 3 * sizeof(int), SEEK_SET);
            status = read(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
            lseek(fd, cur_position, SEEK_SET);
            write(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
            ftruncate(fd, file_end - 3 * sizeof(int));
            lseek(fd, cur_position, SEEK_SET);
        }
   }
   if (status < 0){
      close(fd);
      return status;
   }
   close(fd);
   return EXIT_SUCCESS;
}

int set_objects(char* filename, int argc, char** objects_list){
   int nb_objects_old;
   int nb_objects_new = (argc)/(NB_PROPERTIES + 1);

   if((nb_objects_new <= 0)||(((argc)%(NB_PROPERTIES + 1))!=0)){
      fprintf(stderr, "Error args\n");
      return EXIT_FAILURE;
   }
   
   
   int fd = open(filename, O_RDWR);

   //int backup = open(".", O_RDWR | O_TMPFILE, S_IRUSR | S_IWUSR);

   lseek(fd, 2 * sizeof(int), SEEK_SET);
   read(fd, &nb_objects_old, sizeof(int));

   if(nb_objects_new < nb_objects_old){
      fprintf(stderr, "Error must be more or as many objects\n");
      return EXIT_FAILURE;
   }

   lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
   write(fd, &nb_objects_new, sizeof(int));

   char* names_old[nb_objects_old];
   char* names_new[nb_objects_new];

   int pos = 0;
   while(objects_list[pos * (NB_PROPERTIES + 1)]!=NULL){
      //fprintf(stderr, "bonjour\n");
      names_new[pos] = (char*) malloc((strlen(objects_list[pos * (NB_PROPERTIES + 1)])) * sizeof(char));
      strcpy(names_new[pos], objects_list[pos * (NB_PROPERTIES + 1)]);
      //fprintf(stderr, "%s\n", objects_list[pos * (NB_PROPERTIES + 1)]);
      //fprintf(stderr, "%s\n", names_new[pos]);
      //fprintf(stderr, "%d\n", strcmp(objects_list[pos * (NB_PROPERTIES + 1)], names_new[pos]));
      pos++;
   }

   //fprintf(stderr, "au revoir\n");

   int obj_name_length;
   
   for(pos = 0; pos < nb_objects_old; pos++){
      read(fd, &obj_name_length, sizeof(int));
      //fprintf(stderr, "%d\n", obj_name_length);
      char buffer_name;
      names_old[pos] = (char*) malloc((obj_name_length + 1) * sizeof(char));
      for(int i = 0; i < obj_name_length; i++){
         read(fd, &buffer_name, sizeof(char));
         names_old[pos][i] = buffer_name;
      }
      names_old[pos][obj_name_length] = '\0';
      //strcpy(names_old[pos], buffer_name);
      //fprintf(stderr, "%s\n", buffer_name);
      //fprintf(stderr, "%s %s\n", names_old[pos], names_old[0]);
      //fprintf(stderr, "%d\n", strcmp(buffer_name, names_old[pos]));
      lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
   }

   int replace[nb_objects_old];
   for (int i = 0; i < nb_objects_old; i++){
      replace[i] = -1;
   }

   for(int i = 0; i < nb_objects_old; i++){
      for(int j = 0; j < nb_objects_new; j++){
         if(strcmp(names_old[i], names_new[j]) == 0){
            replace[i] = j;
            //fprintf(stderr, "%s %d et %s %d\n", names_old[i], i, names_new[j], j);
            //fprintf(stderr, "%d\n", replace[i]);
         }
      }
   }

   int status;
   off_t cur_position;
   off_t file_end;

   cur_position = lseek(fd, 0, SEEK_CUR);
   file_end = lseek(fd, 0, SEEK_END);
   char array[file_end - cur_position];
   lseek(fd, cur_position, SEEK_SET);
   read(fd, &array, file_end - cur_position);

   lseek(fd, 3 * sizeof(int), SEEK_SET);
   for(int i = 0; i < nb_objects_new; i++){
      obj_name_length = (int) strlen(objects_list[i * (NB_PROPERTIES + 1)]);
      write(fd, &obj_name_length, sizeof(int));
      write(fd, (objects_list[i * (NB_PROPERTIES + 1)]), obj_name_length * sizeof(char));

      int nb_frames = atoi(objects_list[i * (NB_PROPERTIES + 1) + 1]);
      write(fd, &nb_frames, sizeof(int));

      int prop;
      
      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "air") == 0){
         prop = MAP_OBJECT_AIR;
         write(fd, &prop, sizeof(int));
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "semi-solid") == 0){
            prop = MAP_OBJECT_SEMI_SOLID;
            write(fd, &prop, sizeof(int));
         }
         else{
            if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "solid") == 0){
               prop = MAP_OBJECT_SOLID;
               write(fd, &prop, sizeof(int));
            }
            else{
               if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 2], "liquid") == 0){
                  prop = MAP_OBJECT_LIQUID;
                  write(fd, &prop, sizeof(int));
               }
               else{
                  return EXIT_FAILURE;
               }
            }
         }
      }

      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 3], "destructible") == 0){
         prop = 1;
         write(fd, &prop, sizeof(int));
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 3], "not-destructible") == 0){
            prop = 0;
            write(fd, &prop, sizeof(int));
         }
         else{
            return EXIT_FAILURE;
         }
      }

      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 4], "collectible") == 0){
         prop = 1;
         write(fd, &prop, sizeof(int));
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 4], "not-collectible") == 0){
            prop = 0;
            write(fd, &prop, sizeof(int));
         }
         else{
            return EXIT_FAILURE;
         }
      }

      if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 5], "generator") == 0){
         prop = 1;
         write(fd, &prop, sizeof(int));
      }
      else{
         if(strcmp(objects_list[i * (NB_PROPERTIES + 1) + 5], "not-generator") == 0){
            prop = 0;
            write(fd, &prop, sizeof(int));
         }
         else{
            return EXIT_FAILURE;
         }
      }
   }

   for (int i = 0; i < nb_objects_old; i++){
      free(names_old[i]);
   }
   for (int j = 0; j < nb_objects_new; j++){
      free(names_new[j]);
   }

   write(fd, &array, file_end - cur_position);
   status = lseek(fd, 0, SEEK_CUR);
   ftruncate(fd, status);
   
   lseek(fd, (off_t) -1 * (file_end - cur_position), SEEK_CUR);

   
   int buf;

   while((read(fd, &buf, sizeof(int))) > 0){
      //fprintf(stderr, "x: %d\n", buf);
      lseek(fd, 1 * sizeof(int), SEEK_CUR);
      read(fd, &buf, sizeof(int));
      //fprintf(stderr, "%d, %d\n", buf, replace[buf]);
      if (replace[buf] != -1){
         lseek(fd, (off_t) -1 * sizeof(int), SEEK_CUR);
         write(fd, &(replace[buf]), sizeof(int));
      }
      else{      
         status = lseek(fd, (off_t) -3 * sizeof(int), SEEK_CUR);

         cur_position = (off_t) status;
         //fprintf(stderr, "curpos: %d\n", (int) cur_position);
         status = lseek(fd, 0, SEEK_END);
         file_end = (off_t) status;
         //fprintf(stderr, "filend: %d\n", (int) file_end);
         char buffer[file_end - cur_position - 3 * sizeof(int)];
         //fprintf(stderr, "sizeofint: %d\n", (int) sizeof(char));
         lseek(fd, cur_position + 3 * sizeof(int), SEEK_SET);
         read(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));

         lseek(fd, cur_position, SEEK_SET);
         write(fd, &buffer, (file_end - cur_position - 3 * sizeof(int)));
         ftruncate(fd, file_end - 3 * sizeof(int));
         lseek(fd, cur_position, SEEK_SET);
         
      }
   }   
   
   return EXIT_SUCCESS;
   
}

int prune_objects(char* filename){
   int nb_objects;
   int buf;
   int status;
   
   
   int fd = open(filename, O_RDWR);

   lseek(fd, 2 * sizeof(int), SEEK_SET);
   read(fd, &nb_objects, sizeof(int));
   for (int i = 0; i < nb_objects; i++){
      int obj_name_length;
      read(fd, &obj_name_length, sizeof(int));
      lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
      lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
   }

   int presence[nb_objects];
   for (int i = 0; i < nb_objects; i++){
      presence[i] = 0;
   }
   
   while((status = read(fd, &buf, sizeof(int))) > 0){
	fprintf(stderr, "x: %d\n", buf);
        lseek(fd, sizeof(int), SEEK_CUR);
	read(fd, &buf, sizeof(int));
        presence[buf]++;
   }
  
   int nb_objects_new = nb_objects;
   
   lseek(fd, 3 * sizeof(int), SEEK_SET);
   for(int i = 0; i < nb_objects; i++){
      if(presence[i] == 0){
         nb_objects_new--;
         off_t cur_position = lseek(fd, 0, SEEK_CUR);
         off_t new_position;
         int obj_name_length;
         read(fd, &obj_name_length, sizeof(int));
         lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
         new_position = lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
         off_t file_end = lseek(fd, 0, SEEK_END);
         char buffer[file_end - new_position];
         lseek(fd, new_position, SEEK_SET);
         read(fd, &buffer, file_end - new_position);
         lseek(fd, cur_position, SEEK_SET);
         write(fd, &buffer, file_end - new_position);
         status = lseek(fd, 0, SEEK_CUR);
         ftruncate(fd, status);
         lseek(fd, cur_position, SEEK_SET);
      }
      else{
         int obj_name_length;
         read(fd, &obj_name_length, sizeof(int));
         lseek(fd, obj_name_length * sizeof(char), SEEK_CUR);
         lseek(fd, NB_PROPERTIES * sizeof(int), SEEK_CUR);
      }
   }

   lseek(fd, 2 * sizeof(int), SEEK_SET);
   write(fd, &nb_objects_new, sizeof(int));
   
   
   close(fd);
   
   return EXIT_SUCCESS;
}

int main(int argc, char* argv[]){
   char* filename = argv[1];
   if(strcmp(argv[2], "--getwidth") == 0){
	get_width(filename);
   }
   if(strcmp(argv[2], "--getheight") == 0){
	get_height(filename);
   }
   if(strcmp(argv[2], "--getobjects") == 0){
	get_objects(filename);
   }
   if(strcmp(argv[2], "--getinfo") == 0){
	get_info(filename);
   }
   if(strcmp(argv[2], "--setwidth") == 0){
	set_width(filename, atoi(argv[3]));
   }
   if(strcmp(argv[2], "--setheight") == 0){
	set_height(filename, atoi(argv[3]));
   }
   if(strcmp(argv[2], "--setobjects") == 0){
	set_height(filename, argc - 3, argv + 3);
   }
   if(strcmp(argv[2], "--pruneobjects") == 0){
	set_height(filename);
   }
   return EXIT_SUCCESS;
}
