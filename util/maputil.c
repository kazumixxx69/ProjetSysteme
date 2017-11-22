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



int main(int argc, char* argv[]){
   /*remplir vérification arguments machin*/

   get_width(argv[1]);
   get_height(argv[1]);
   get_objects(argv[1]);
   get_info(argv[1]);
   set_width(argv[1], atoi(argv[2]));
   get_info(argv[1]);


   return EXIT_SUCCESS;

}
