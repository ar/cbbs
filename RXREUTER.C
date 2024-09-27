/* rxreuter.c
   recibe reuters hasta que meten un /FIN
*/

#if !defined (ALL_IN_ONE)
#   include <stdio.h>
#   include <alloc.h>
#   include <stdlib.h>
#   include <dos.h>
#   include <dir.h>
#endif

void reutsconf(char *from, char *fromfile, char *to, char *subject) {
   FILE *fp, *fpaux;
   char tofile[MAXPATH], line[81];
   int  l, msgno;
   makefilename(tofile  ,CONFPATH   ,to  ,"CNF");
   fp = t_fopen(-1,tofile,APPEND);
   fseek(fp,0L,SEEK_END);
   msgno = i_addmsg(to,ftell(fp));
   fprintf(fp,"%-7s%d\r\n",MSGDELIM,msgno); pause();
   fprintf(fp,"%-7s%s\r\n","De:"   ,from);  pause();
   fprintf(fp,"%-7s%02d/%02d/%02d\r\n","Fecha:",st->tm_mday,(st->tm_mon+1),st->tm_year); pause();
   fprintf(fp,"%-7s%02d:%02d:%02d\r\n","Hora:" ,st->tm_hour,st->tm_min,st->tm_sec);      pause();
   fprintf(fp,"%-7s%s\r\n","Tema:" ,subject);                                            pause();
   fprintf(fp,"\r\n");                                                                   pause();
   fpaux = t_fopen(-1,fromfile,READ); pause();
   l     = strlen(MSGDELIM);          pause();
   while(f_gets(line,80,fpaux) != nil(char)) {
      if(equaln(line,MSGDELIM,l)) fputc('>',fp);
      fprintf(fp,"%s\r\n",line);
      pause();
   }
   fclose(fpaux);
   fclose(fp);
   pause();
}

void rxreuter(int number) {
   char c;
   int fin;
   t_fopen(number,"/cbbs/reuters.not",APPEND);
   tty[number].echo = OFF;
   tty[number].ocu  = ON;
   if(FP != NULL) {
      for(fin=0; fin < 4;) {
         c = t_getc(number);
         if (c == '~') fin++;
         else {
            fputc(c, FP);
            fin = 0;
         }
      }
      t_fclose (number);
      inc_counter (&reuters);
      if (number)
         while(v24_receive(tty[number].siop,36L) != -1); /* nos tragamos basura */
   }
}


#define SLSFILE    "REUTERS.TXT"
#define PARSE_INFO "REUTERS.DAT"
#define MAXPAGES 200
#define TEMPDIR "/cbbs/reuters/"

typedef struct {
   char title[41];
   char filename[9];
} PAGE;

PAGE *pag;
int  pag_elems;

int process_line (char *line, char *w_line, int *state) {
   pause(); pause(); pause();
   switch (*state) {
      case 0 :
         if ((line[0] == line[1]) &&
             (line[0] == 'Y' ||
              line[0] == 'U' ||
              line[0] == 'H')) {
            *state = 1;
         }
         return (0);
      case 1 :
         strcpy (w_line, line);
         *state = 2;
         return (1);
      case 2 :
         if (line[0] == (char)0xF2 && line[1] == (char)0x12) {
            strcpy (w_line, line+2);
            line = w_line;
            while (*line != '\0') {
               if (*line == '\xF2') {
                  *line = '\0';
                  break;
               }
               pause();
               line++;
            }
            *state = 3;
            return (2);
         }
         return (0);
      case 3 :
         if (strstr (line, "NNNN") != NULL) {
            *state = 0;
            return (4);
         }
         else if (strstr (line, "\x14\x14\xF2") != NULL) {
            return (0);
         }
         else {
            strcpy (w_line, line);
            return (3);
         }
   }
   return (0);
}

char *sls_gets (char *s, int len, FILE *fp) {
   int i=0, c;

   while((c=fgetc(fp)) != EOF && c != '\r') {
      pause();
      if(i<len) {
         if (c != '\0') {
            switch (c) {
               case '\x15' :
                  c = ' ';
                  break;
               case '\x7' :
                  c = '/';
                  break;
            }
            s[i++] = c;
         }
      }
   }
   while (c == '\r' || c == '\n') {
      pause();
      c = fgetc (fp);
   }
   ungetc (c, fp);
   s[i] = '\0';
   return(c == EOF ? nil(char) : s);
}

int pag_compare(const void *e1, const void *e2) {
   pause();
   return strcmp( ((PAGE*)e1)->title, ((PAGE*)e2)->title);
}

void parse_reuters_data (void) {
   char filename[MAXPATH], line[81], w_line[81], tipo[11], tema[41],
        newfilename[MAXPATH];
   int  filenumber=0, state, ret, i;
   FILE *fp, *fpout, *fperr;
   PAGE *wpag;

   sprintf (filename, "%sREUTERS.ERR", TEMPDIR);
   fperr = t_fopen (-1, filename, APPEND);

   pag = tsk_alloc (MAXPAGES*sizeof (PAGE));
   if (pag == NULL) {
      tsk_rprintf ("PANIC: RXREUTER.C out of memory !\r\n");
      return;
   }

   for (i=0; i < MAXPAGES; i++) {
      pag[i].filename[0] = '\0';
      pag[i].title[0]    = '\0';
      pause();
   }
   fp = t_fopen (-1, PARSE_INFO, READ);
   if (fp != NULL) {

      for (i=0; f_gets (line, 80, fp) && i < MAXPAGES; i++) {
         strncpy (pag[i].filename, line, 8);
         strncpy (pag[i].title, line+9, 40);
         pag[i].filename[8] = '\0';
         pag[i].title[40]   = '\0';
         pause(); pause(); pause();
      }
      fclose (fp);
      pag_elems = i;
   }

   qsort (pag, pag_elems, sizeof (PAGE), pag_compare);

   fp = t_fopen (-1, SLSFILE, READ);
   while (!feof(fp)) {
      sprintf (filename, "%sREU%05d.$$$", TEMPDIR, filenumber++);
      fpout = t_fopen (-1, filename , WRITE);
      state = ret = 0;
      while (!feof(fp) && ret != 4) {
         sls_gets(line,80,fp);
         ret = process_line (line,w_line,&state);
         switch (ret) {
            case 0 :
               break;
            case 1 :
               strncpy (tipo, w_line, 10);
               tipo[10] = '\0';
               /* fprintf (fpout, "%s\r\n",w_line); */
               break;
            case 2 :
               strncpy (tema, w_line, 40);
               tipo[40] = '\0';
               fprintf (fpout, "%s\r\n",w_line);
               break;
            case 3 :
               fprintf (fpout, "%s\r\n",w_line);
               break;
         }
      }
      fclose (fpout);
      pause(); pause(); pause();

      if (strnicmp (tipo, "zr", 2) == 0) {
         /* verificamos si 'filename' nos sirve, y en ese caso la renombramos */
         wpag = bsearch(&tema, pag, pag_elems, sizeof(PAGE), pag_compare);
         if (wpag != NULL) {
            sprintf (newfilename,"%s%s.TXT",TEMPDIR,wpag->filename);
            pause();
            checkfile (newfilename);
            unlink (newfilename);
            rename (filename, newfilename);
         }
         else {
            fprintf (fperr,"xxxxxxxx %s\r\n", tema);
            checkfile (filename);
            unlink (filename);
         }
      }
      else {
         reutsconf(tipo, filename, "NOTICIAS", tema);
         checkfile (filename);
         unlink (filename);
      }
      pause(); pause(); pause();
   }
   fclose (fp);
   fclose (fperr);
   tsk_free (pag);
   sprintf (filename, "%s%08ld.REU", TEMPDIR, _clock);
   rename (SLSFILE, filename);
   pause(); pause(); pause();
}

void reutaccess(int number, char *servicio, char *currentmenu) {
   typefile(number,"/cbbs/msg/reutpant.msg");
   if(confirm(number,"Confirma acceso ? "))
      if(confirm(number,"Esta seguro ?     "));
      else return;
   else return;
   t_puts(number,"Conectando con REUTERS...");
   writelog(number,"/cbbs/reuters.log", servicio);
   t_delay(90L);
   t_puts(number,"\n\nCOM\n\n");
   strcpy(currentmenu, servicio);
}
