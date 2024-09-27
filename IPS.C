/* ips.c
 * Recibe noticias de la agencia IPS en baudot a 75 bps
 *
 */

#define LTRS 0
#define FIGS 1

int moderec = LTRS;
static int baudotrec[] = {
   '_',   'E', 0x0A,  'A',  ' ',  'S',  'I',  'U',  /* 00-07 '_'=Blank */
   0x0D,  'D',  'R',  'J',  'N',  'F',  'C',  'K',  /* 08-0F */
    'T',  'Z',  'L',  'W',  'H',  'Y',  'P',  'Q',  /* 10-17 */
    'O',  'B',  'G', 0xFF,  'M',  'X',  'V', 0xFF,  /* 18-1F */

    'a',  '3', 0x0A,  '-',  ' ', '\'',  '8',  '7',  /* 20-27 El ' ' era un '\t' */
   0x0D,  '*',  '4', '\a',  ',',  'e',  ':',  '(',  /* 28-2F '*'=Cruz de Malta*/
    '5', '\"',  ')',  '2',  'f',  '6',  '0',  '1',  /* 30-37 */
    '9',  '?',  'g', 0xFF,  '.',  '/',  '=', 0xFF,  /* 38-3F */
};

#if 0
static int baudotrec[64];
static int baudot[]={
   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0xFF, 0x05,  /* 00-07 */
   0xFF, 0xFF, 0x02, 0xFF, 0xFF, 0x08, 0xFF, 0xFF,  /* 08-0F */
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 10-17 */
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 18-1F */
   0x04, 0xFF, 0x11, 0x14, 0x09, 0xFF, 0xFF, 0x0B,  /* 20-27 */
   0x0F, 0x12, 0xFF, 0x11, 0x0C, 0x03, 0x1C, 0x1D,  /* 28-2F */
   0x16, 0x17, 0x13, 0x01, 0x0A, 0x10, 0x15, 0x07,  /* 30-37 */
   0x06, 0x18, 0x0E, 0x1E, 0xFF, 0x1E, 0xFF, 0x19,  /* 38-3F */

   0xFF, 0x03, 0x19, 0x0E, 0x09, 0x01, 0x0D, 0x1A,  /* 40-47 */
   0x14, 0x06, 0x0B, 0x0F, 0x12, 0x1C, 0x0C, 0x18,  /* 48-4F */
   0x16, 0x17, 0x0A, 0x05, 0x10, 0x07, 0x1E, 0x13,  /* 50-57 */
   0x1D, 0x15, 0x11, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 58-5F */
   0xFF, 0x03, 0x19, 0x0E, 0x09, 0x01, 0x0D, 0x1A,  /* 60-67 */
   0x14, 0x06, 0x0B, 0x0F, 0x12, 0x1C, 0x0C, 0x18,  /* 68-6F */
   0x16, 0x17, 0x0A, 0x05, 0x10, 0x07, 0x1E, 0x13,  /* 70-77 */
   0x1D, 0x15, 0x11, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF   /* 78-7F */
};

void InitBaudotTables (void) {
   int i;
   for(i=0; i < 64; i++) {
      if(baudot[i] != 0xFF) {
         baudotrec[baudot[i]] = toupper((char)i);
      }
   }
   for(i=64; i < 128; i++) {
      if(baudot[i] != 0xFF) {
         baudotrec[baudot[i]+32] = toupper((char)i);
      }
   }
}
#endif

int ReceiveBaudot (sioptr siop, long timout) {
   int c;
   for (;;) {
      c = v24_receive (siop, timout);
#ifdef AT_CS /* no recibimos Baudot sino ASCII para las pruebas */
   switch (c) {
      case '+' :
      case '\x7' :
         return '\"';
      case '\0' :
         return ' ';
      default :
         return c;
   }
#endif
      c &= 0x1F;
      switch (c) {
         case 0x1B:
            moderec = FIGS;
            break;
         case 0x1F:
            moderec = LTRS;
            break;
         default :
            return (baudotrec[32*moderec+c] & 0xFF);
      }
   }
}

void far ips_receive (void) {
   char fname[13];
   FILE *fp;
   int c, enes=0;
   /* InitBaudotTables (); ##FIXME## sacar este c¢digo */
   unlink (IPSCAPFILE);
   unlink (IPSFILE);
   fp = fopen (IPSCAPFILE, "wb");
   cprintf ("ips_receive: Ok\r\n");
   for (;;) {
      c = ReceiveBaudot (ips_siop, 1800L);
      if (c != -1) {
         if (c == 'N') enes++;
         else enes = 0;
         fprintf (fp, "%c", c);
         pause ();
         if (tty[0].user[0] == '\0' && c != '\a')
            cprintf ("%c", c);
         if (enes == 4) {
            fprintf (fp, "\r\n");
            if (tty[0].user[0] == '\0')
               cprintf ("\r\n");
         }
      }
      if (parsing_ips == 0 && (enes == 4 || (c==-1 && ftell (fp) > 0L) )) {
         /* We've got a message or part of a message */
         fclose (fp);
         sprintf (fname, "%ld.ips", _clock);
         /* rename (IPSCAPFILE, IPSFILE); */
         rename (IPSCAPFILE, fname);
         fp = fopen (IPSCAPFILE, "wb");
         // inc_counter (&ipscounter);
         enes = 0;
      }
   }
}

void ips_sconf(char *from, char *fromfile, char *to, char *subject) {
   FILE *fp, *fpaux;
   char tofile[MAXPATH], line[80];
   int  msgno, i, uppercase, in_paragraph;
   makefilename(tofile  ,CONFPATH   ,to  ,"CNF");
   fp = t_fopen(-1,tofile,APPENDTEXT);
   if (fp == NULL)
      return;
   fseek(fp,0L,SEEK_END);
   msgno = i_addmsg(to,ftell(fp));
   fprintf(fp,"%-7s%d\n",MSGDELIM,msgno); pause();
   fprintf(fp,"%-7s%s\n","De:"   ,from);  pause();
   fprintf(fp,"%-7s%02d/%02d/%02d\n","Fecha:",st->tm_mday,(st->tm_mon+1),st->tm_year); pause();
   fprintf(fp,"%-7s%02d:%02d:%02d\n","Hora:" ,st->tm_hour,st->tm_min,st->tm_sec);      pause();
   strlwr (subject);
   subject[0] = toupper (subject[0]);
   fprintf(fp,"%-7s%s\n","Tema:" ,subject);                                            pause();
   fprintf(fp,"\n");                                                                   pause();
   fpaux = t_fopen(-1,fromfile,READTEXT); pause();
   in_paragraph = uppercase = 0;
   while (fgets(line, 79,fpaux)) {
#if 0
      for (i=0; line[i] != '\0'; i++) {
         line[i] = uppercase ? toupper (line[i]) : tolower (line[i]);
         if (!in_paragraph) {
            /* fprintf(fp,"\n"); */
            line[i] = toupper (line[i]);
            uppercase = 0;
            in_paragraph = !in_paragraph;
         }
         if (line[i] == '\"')
            uppercase = !uppercase;
         if (line[i] == '\n') {
            line[i] = '\0';
            in_paragraph = !in_paragraph;
         }
         pause ();
      }
#endif
      if (stricmp (line, "NNNN") != 0)
         fprintf(fp,"%s\n",line);
      pause();
   }
   fclose(fpaux);
   fclose(fp);
   pause();
   inc_lastmsg (to);
}
