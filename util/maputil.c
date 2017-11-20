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
      case 0:
         property = "Width";
         break;
      case 1:
         property = "Height";
         break;
      case 2:
         property = "Number of objects";
         break;
      default:
         status = getwidth(filename);
         if (status < 0){
            return status;
         }
         status = getheight(filename);
         if (status < 0){
            return status;
         }
         status = getobjects(filename);
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

int getwidth(char* filename){
   int status;
   status = get(filename, 0);
   return status;
}

int getheight(char* filename){
   int status;
   status = get(filename, 1);
   return status;
}

int getobjects(char* filename){
   int status;
   status = get(filename, 2);
   return status;
}

int getinfo(char* filename){
   int status;
   status = get(filename, 3);
   return status;
}

/*int setwidth(char* filename, int value){
   int buf = value;
   int old_value;
   int status;

   int fd = open(filename, O_RDWR);
   char* backup_name = strcat("/backups/", filename);
   int backup = open(backup_name, O_RDWR | O_CREAT | O_TRUNC, 0666);

   dup2(fd, backup);

   lseek(fd, 0 * sizeof(int), SEEK_SET);
   write(fd, &buf, sizeof(int));

   return EXIT_SUCCESS;


   }*/



int main(int argc, char* argv[]){
   /*remplir vérification arguments machin*/

   getwidth(argv[1]);
   getheight(argv[1]);
   getobjects(argv[1]);
   getinfo(argv[1]);


   return EXIT_SUCCESS;

}
