/* rfc822.c
 * funciones de manejo de mensajes en standard RFC822 para el CBBS
 * Modificaciones
 * 17:48 05/10/89   Version inicial (APR)
 * 19:04 24/03/90   Modificado para que compile solo
 */

#ifndef CBBSMAIN
#   include <stdarg.h>
#   include "cbbs.h"
#endif

char *secondtoken(char *s) {
   char *ptr = nil(char);
   while((*s == ' ') || (*s == '\t')) s++;
   if((*s != '\n') && (*s != '\0')) {
      ptr = s;
   }
   pause();
   return(ptr);
}

char *f_gets(char *s,int len,FILE *fp) {
   int i=0, c;
   while((c=fgetc(fp)) != EOF && c != '\r') {
      if(i<len)
         s[i++] = c;
      else
         break;
      pause();
   }
   if(c == '\r') {
      c=fgetc(fp);
   }
   s[i] = '\0';
   return(c == EOF ? nil(char) : s);
}

int loadheader(FILE *fp,int n,...) {
   char line[81];
   int i, j=0, k=0;
   struct {
      char *text;
      char *data;
      int len;
      int textlen;
   } head[MAXHEADERLINES];
   va_list argptr;

   va_start(argptr,n);
   for(i=0; i<n; i++) {
      head[i].text      = va_arg(argptr,char *);
      head[i].data      = va_arg(argptr,char *);
      head[i].len       = va_arg(argptr,int);
      head[i].textlen   = strlen(head[i].text);
      head[i].data[0] = '\0';
      pause();
   }
   while(f_gets(line,80,fp) != nil(char)) {
      if(*line == '\0') break;
      for(i=0; i<n; i++,j++) {
         j%=n;
         if(equaln(line,head[j].text,head[j].textlen)) {
            strncpy(head[j].data,secondtoken(line+head[j].textlen),head[j].len);
            head[j].data[head[j].len] = '\0';
            k++;
            i=n;
	 }
	 pause();
      }
   }
   return(k);
}

int nextmsg(FILE *fp, char *delim) {
   char line[81];
   int  delimlen;
   long offset;
   delimlen = strlen(delim);
   for(;;) {
      offset = ftell(fp);
      if(f_gets(line,80,fp) == nil(char)) return(FALSE);
      if(equaln(line,delim,delimlen)) {
         RQ; fseek(fp,offset,SEEK_SET); RL;
         return(TRUE);
      }
      pause();
   }
}

