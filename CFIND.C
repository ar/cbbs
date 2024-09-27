/* cfind.c
 * CompuService Find Program
 *
 * Emula el FIND de dos per no case sensitive
 *
 * Modificaciones
 * 20:42 11/10/89   Version inicial (APR)
 * 22:52 24/03/90   Se modifica para que compile solo
 */

#ifndef CBBSMAIN
#   include <dir.h>
#   include "cbbs.h"
#endif

void cfind(int number, char *param) {
   char command[41], fname[MAXPATH];
   char *buffer;
   int lines;

   buffer = tsk_alloc(1024);
   if(param != NULL && param[0] != '.' && buffer != NULL) {
      makefilename(fname,DBFPATH,param,"MSG");
      typefile(number,fname);
      for(;;) {
         t_puts(number,"Ingrese palabra a buscar : ");
         t_gets(number,command,40);
         strupr(command);
         if(strlen(command) > 0) {
            t_printf(number,"Buscando '%s'...\n",command);
            makefilename(fname,DBFPATH,param,"HDR");
            typefile(number,fname);
            makefilename(fname,DBFPATH,param,"TXT");
            t_fopen(number,fname,READ);
            lines = 0;
            while(f_gets(buffer,1023,FP) != NULL) {
	       strupr(buffer);
               if(within(command,buffer)) {
                  t_printf(number,"%s\n",buffer);
                  lines++;
                  if(lines > TERMROWS) {
                     if(!more(number)) break;
                     else lines=0;
                  }
               }
            }
            t_fclose(number);
         }
         else break;
      }
   }
   else {
      t_puts(number,"Base de datos no accesible\n");
   }
   tsk_free(buffer);
}
