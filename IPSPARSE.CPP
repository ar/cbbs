#ifndef TELNET

#include "exprclas.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE    1024     // max line width
#define TMPFILE        "/cbbs/scratch/IPS.$$$"
#define IPSCAPFILE     "/cbbs/scratch/IPS.CAP"     /* Ojo, redefinidas en alg£n *.CPP para no incluir */
#define IPSFILE        "/cbbs/scratch/IPS.FIL"     /* cbbs.h */

extern "C" {
void ips_sconf(char *from, char *fromfile, char *to, char *subject);
void far schedule (void);
}
#define pause() schedule()

struct {
   char tipo[3];
   char conf[9];
} TCONF[] = {
  { "NP", "ips" }, // National Politics
  { "RP", "ips" }, // Regional Politics
  { "IP", "ips" }, // International Politics
  { "AD", "ips" }, // Arms and Disarmaments
  { "NE", "ips" }, // National Economy
  { "RE", "ips" }, // Regional Economy
  { "IE", "ips" }, // International Economy
  { "CD", "ips" }, // Cooperation/Development
  { "TL", "ips" }, // Trade Unions
  { "EO", "ips" }, // Energy/Petroleum
  { "AC", "ips" }, // Agriculture/Raw Materials (Commodities?)
  { "CI", "ips" }, // Communication/Information
  { "ST", "ips" }, // Science/Tecnology
  { "EN", "ips" }, // Enviroment
  { "PR", "ips" }, // Population/Refugees
  { "HE", "ips" }, // Health/Medicine
  { "HR", "ips" }, // Human Rights
  { "CE", "ips" }, // Culture/Education
  { "AE", "ips" }, // Arts/Show Business (?)
  { "SP", "ips" }, // Sport
  { "ND", "ips" }, // Natural Disasters
  { "DW", "ips" }, // Drugs/Narcotics
  { "OT", "ips" }, // Other
  { "#1", "ips" }, // Servicio ASIN - Lo sacamos del tema
  { ""  , ""    }  // Dummy End
};

char *confname (char *tipo) {
   int i;
   for (i=0; TCONF[i].tipo[0] != '\0'; i++)
      if (strcmp (tipo, TCONF[i].tipo) == 0)
         return (TCONF[i].conf);
   return ("ips");
}

FILE *ips_send2conf (FILE *fpout, char *confname, char *subject) {
   fclose (fpout);
   ips_sconf("IPS", TMPFILE, confname, subject);
   unlink (TMPFILE);
   fpout = fopen (TMPFILE, "wt");
   return (fpout);
}

char *ipstrim (char *d, char *s, int len, int cap) {
   int i, j, flag, lastbi, lastbj;
   for (i=j=lastbi=lastbj=0, flag=1; --len && s[i] != '\0'; i++) {
      pause ();
      if (s[i] != ' ') {
         d[j++] = cap ? toupper (s[i]) : s[i];
         flag = cap = 0;
      }
      else if (!flag) {
         flag = !flag;
         lastbi = i;
         lastbj = j;
         d[j++] = s[i];
      }
   }
   if (len) {
      lastbi = i;
      lastbj = j;
   }
   d[lastbj]='\0';
   return lastbi ? s+lastbi : NULL;
}

void write_ips_paragraph (FILE *fpout, char *line) {
   int capitalize=1;
   char s[81];
   if (strlen (line) > 0) {
      strlwr (line);
      do {
         line = ipstrim (s, line, 79, capitalize);
         capitalize=0;
         if (stricmp (s, "NNNN") != 0) 
            fprintf (fpout, "%s\n", s);
      } while (line);
   }
}

extern "C" {
void parse_ips_data (void) {
   Regexpr re;         // instance creation (line,pattern are default)
   FILE *fpin, *fpout;
   char line[LINE],subject [41], tipo[3], zcline [161];
   char *s,*e;
   int state = 0;
   // state:
   //        0 = out of message, waiting ZCZC
   //        1 = got ZCZC, waiting Subject
   //        2 = in message, waiting NNNN

   // open file
   if ((fpin = fopen(IPSFILE,"rt")) == NULL) {
      printf("\nCannot open %s.\n", "IPS.CAP");
      unlink (IPSFILE);
      unlink (TMPFILE);
      return;
   }
   if ((fpout = fopen (TMPFILE, "wt")) == NULL) {
      printf ("\nCannot open %s.\n", "IPS.$$$");
      fclose (fpin);
      unlink (IPSFILE);
      unlink (TMPFILE);
      return;
   }
   subject[0] = tipo[0] = tipo[2] = '\0';
   while (fgets (line, LINE, fpin)) {
      pause (); pause (); pause ();
      line [strlen (line)-1] = '\0';
      pause (); pause (); pause ();
      re.set_line (line);
      re.set_pattern ("ZCZC");
      if (re.find_first ()) { // estamos ante comienzo de mensaje
         re.set_pattern ("/");
         if (re.find_first (&s, &e)) {
            tipo[0] = *(s+1);
            tipo[1] = *(s+2);
         }
         if (state != 0) {    // condicion de error
            if (ftell (fpout) > 0L) {
               fprintf (fpout, "\nERROR: ZCZC en estado no 0\n");
               fpout = ips_send2conf (fpout, "ipserror", line);
            }
         }
         sprintf (zcline, "%s\n(%s)", line, tipo);
         subject[0] = '\0';
         state = 1; // estamos dentro de un mensaje, esperamos tema
         continue;
      }
      if (state == 1) { // estamos esperando tema
         if (line[0] != '\0') {
            strncpy (subject, line, 40);
            subject [40] = '\0';
            if (subject [strlen(subject)-1] == '\n')
               subject [strlen (subject)-1] = '\0';
            if (strstr (subject, "SERVICIO ASIN")) {
               strcpy (tipo, "#1");
               state--;
            }
            state++;
         }
      }
      if (state == 2) {
         re.set_pattern ("^=.[0123456789]*|//.*EMBARGADO");
         if (!re.find_first ()) {
            write_ips_paragraph (fpout, line);
         }
      }
      strupr (line);
      re.set_pattern ("NNNN");
      if (re.find_first ()) { // estamos ante fin de mensaje
         if (ftell (fpout) > 0L) {
            if (state != 2) {    // condicion de error
               fprintf (fpout, "\nERROR: NNNN en estado %d\n", state);
               fpout = ips_send2conf (fpout, "ipserror", line);
            }
            else {
               fprintf (fpout, "%s (#%s)\n", zcline, tipo);
               fpout = ips_send2conf (fpout, confname (tipo), subject);
            }
         }
         state = 0; // finaliza mensaje, esperamos ZCZC
         continue;
      }
   }
   fclose (fpin);
   fclose (fpout);
   unlink (IPSFILE);
   unlink (TMPFILE);
}
} // end extern "C"...

#endif // end #ifndef TELNET
