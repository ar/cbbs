/* lib.c
 * funciones usadas por varios programas
 *
 */

#ifndef CBBSMAIN
#   include "cbbs.h"
#endif

#define DISPLAY_STRING 0x09
#define IGNORE 0
#define RETRY  1
#define ABORT  2

void strtrim (char *s, int len) {
   register int i;
   for (i=0; i<len; i++) {
      if (s[i] == ' ') {
         s[i] = '\0';
         break;
      }
   }
   s[i] = '\0';
}

int existfile(int number,char *file) {
   FILE *fp;
   int exist;
   fp = t_fopen(number,file,READ);
   exist = (fp != NULL);
   t_fclose (number);
   return(exist);
}

int within(char *str1, char *str2) {
   /* Retorna TRUE si str1 esta contenida en str2. Ej str1 $ str2 */
   char *s;
   s=str1;
   while(*str2 != '\0') {
      if(*str1 == *str2) {
         str1++;
         str2++;
         if(*str1 == '\0') return(TRUE);
      }
      else if(str1 != s) {
         str1 = s;
      }
      else str2++;
   }
   return(FALSE);
}

int hardhandler(int errval, int ax, int bp,  int si) {
   char msg[25]; int drive;
   bp = si; /* Avoid    */
   si = bp; /* Warnings */

   if(ax<0) {
      sprintf(msg,"\nDEVICE ERROR #%d\n",errval);
   }
   else {
      drive = (ax & 0x00FF);
      sprintf(msg,"\nDISK ERROR #%d in drive %c\n",errval,'A'+drive);
   }
   highvideo();
   tsk_rprintf(msg);
   normvideo();
   hardresume (RETRY);
   return RETRY;  // estaba con IGNORE y no estaba el hardresume ();
}

char *makefilename(char *filename, char *path, char *name, char *ext) {
   strcpy(filename,path);
   strcat(filename,name);
   strcat(filename,".");
   strcat(filename,ext);
   return(filename);
}

void copymsg(long start, long end, FILE *fpsrc, FILE *fpdestin) {
   int  i=FALSE, c;
   fseek(fpsrc,start,SEEK_SET);
   while((c=fgetc(fpsrc)) != EOF && ftell(fpsrc) != end) {
      fputc(c,fpdestin);
      i = TRUE;
      pause();
   }
   if(i && end != -1L) fputc('\n',fpdestin);
}

void typemsg(int number, long start, long end) {
   int lines=0, c;
   RQ; fseek(FP,start,SEEK_SET); RL;
   while((c=fgetc(FP)) != EOF && ftell(FP) != end) {
      if(c != '\r') t_putchar(number,c);
      if(c == '\n') {
         lines++;
         if(lines > TERMROWS) {
            if(!more(number)) break;
            else lines=0;
         }
      }
   }
   t_putchar(number,'\n');
}

int is_in_list(int number, char *file, int def) {
   char line[9];
   int  l;
   l = strlen(tty[number].user);
   t_fopen(number,file,READ);
   if(FP == NULL) {
      return(def);
   }
   while(f_gets(line,8,FP) != nil(char)) {
      if(strnicmp(line,tty[number].user,l)==0) {
         t_fclose(number);
         return(TRUE);
      }
      pause();
   }
   t_fclose(number);
   return(FALSE);
}
