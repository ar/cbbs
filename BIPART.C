/*
 * Busca por bipartici¢n en archivos secuenciales
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_record (char *s, long recno, FILE *fp, long offset, int reclen);

/* bipart retorna 1 si no la encontr¢. 0 si la encontr¢ */
int bipart (char *s, char *key, FILE *fp, long offset, int reclen, int keylen, int keypos) {
   int cmp;
   long i, bottom, top;

   bottom = 0L;
   fseek (fp, 0L, SEEK_END);
   top = (ftell (fp) + 1 - offset) / reclen; /* camina bien con Ctrl-Z al final */

   while (bottom+1 < top) {
      pause ();
      i = (bottom + top) / 2;
      if (read_record (s, i, fp, offset, reclen)) return (1);
      s[reclen] = '\0';
      pause ();
      if ((cmp = strncmp (s + keypos, key, keylen)) == 0) return (0);
      else if (cmp > 0) top = i;
      else bottom = i;
      pause ();
   }
   return (1);
}

int read_record (char *s, long recno, FILE *fp, long offset, int reclen) {
   if (!fseek (fp, offset + (recno * reclen), SEEK_SET))
      if (fread (s, reclen, 1, fp) == 1)
         return (0);
   return (1);
}
