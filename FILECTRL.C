/* filectrl.c
 * funciones de filelocking del CBBS
 * Modificaciones
 * 17:57 05/10/89   Version inicial (APR)
 * 18:13 23/03/90   Se modifica para que compile solo
 */

#ifndef CBBSMAIN
#   include <fcntl.h>
#   include "cbbs.h"
#endif

void t_fclose(int number) {
   if(FP != NULL) {
      RQ;
      fclose(FP);
      FP = NULL;
      HANDLE = 0;
      RL;
   }
}

FILE *t_fopen (int number,char *filename, int lmode) {
   FILE *fp;
   char mode[4];
   int access=0, handle;

   pause();
   fp = NULL;

   switch(lmode) {
      case READ  :
      case READ_WAIT :
         access = O_RDONLY | O_DENYWRITE | O_BINARY;
         strcpy(mode,"r+b");
         break;
      case READTEXT :
      case READTEXT_WAIT :
         access = O_RDONLY | O_DENYWRITE | O_TEXT;
         strcpy(mode,"r+t");
         break;
      case APPEND :
         access = O_WRONLY | O_CREAT | O_DENYALL | O_BINARY | O_APPEND;
         strcpy(mode,"a+b");
         break;
      case APPENDTEXT :
         access = O_WRONLY | O_CREAT | O_DENYALL | O_TEXT | O_APPEND;
         strcpy(mode,"a+t");
         break;
      case WRITE :
         access = O_WRONLY | O_CREAT | O_DENYALL | O_BINARY;
         strcpy(mode,"w+b");
         break;
   }

   for(;;) {
      pause();
      RQ;
      handle = open(filename,access,S_IREAD | S_IWRITE);

      if (number >= 0) {
         HANDLE = handle;
      }
      RL;
      if(handle <= 0) {
         if(lmode == READ || lmode == READTEXT) {
            fp = NULL;
            if (number >=0) {
               FP = NULL;
               HANDLE = 0;
            }
            break;
         }
         else if (number >= 0) {
            t_printf(number,"Archivo no accesible. Aguarde, reintentando....\n");
            t_delay(36L);
         }
         else {
            t_delay(36L);
         }
      }
      else {
         RQ;
         fp = fdopen(handle,mode);
         if (number >= 0) {
            FP = fp;
         }
         RL;
         if(fp == NULL) {
            RQ; 
            close(handle); 
            RL;
         }
         else {
            break;
         }
      }
   };

   if(fp != NULL && lmode == APPEND) {
      RQ; 
      fseek(fp,0L,SEEK_END); 
      RL;
   }
   pause();
   return(fp);
}

void safe_unlink (int number, char *filename) {
   FP = t_fopen (number, filename, WRITE);
   if (FP != NULL) {
      t_fclose (number);
      unlink (filename);
   }
   else {
      t_printf (number, "ERROR eliminando archivo '%s'. Por favor tome nota e informe a Sysop\n");
   }
}

void checkfile (char *filename) {
   int handle;

   for(;;) {
      handle = open(filename,
                    O_WRONLY | O_CREAT | O_DENYALL | O_BINARY,
                    S_IREAD | S_IWRITE);
      if (handle > 0) break;
      t_delay (18L);
      tsk_rprintf ("checking '%s'\r\n",filename);
   }
   close(handle);
}
