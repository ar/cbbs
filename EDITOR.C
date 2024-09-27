/* editor.c
 * Mantenimiento del Scratch-pad del usuario
 * Modificaciones
 * 12:01 08/10/89   Version inicial (APR)
 * 19:53 24/03/90   Se modifica para que compile solo
 */

#ifndef CBBSMAIN
#   include <dir.h>
#   include "cbbs.h"
#endif

void editor(int number, int param, char *user) {
   char workfile[MAXPATH], command[81];
   int  inloop=TRUE, mode;

   t_printf(number,"\nEditor\n");
   t_printf(number,"------\n");
   makefilename(workfile,SCRATCHPATH,user,"WKR");
   if(param == TRANSMITE) {
      t_fopen(number,workfile,READ);
      if(FP != NULL) {
         t_puts(number,"Existen datos en su area de trabajo. Verifique\n");
         param = EDIT;
      }
      t_fclose(number);
   }
   while(inloop) {
      if(param == EDIT) {
         t_puts(number,"Editor>");
         t_gets(number,command,40);
      }
      else {
         command[0] = 't';
         inloop     = FALSE;
      }
      mode = WRITE;
      switch(tolower(command[0])) {
         case 'f' :
            inloop = FALSE;
            break;
         case 'd' :
            typefile(number,workfile);
            break;
         case 'x' :
            switch(tolower(command[1])) {
               case 'u' :
                  xm_receive_file(number,workfile,workfile);
                  break;
               case 'd' :
                  xm_transmite_file(number,workfile,workfile);
                  break;
               default  :
                  t_puts(number,"Error : xd - Download\n");
                  t_puts(number,"        xu - Upload\n");
                  break;
            }
            break;
         case 'a' :
            mode = APPEND;
         case 't' :
            t_fopen(number,workfile,mode);
            if(FP != NULL) {
               t_printf(number,"Ingrese texto ('/FIN' finaliza)\n");
               for(;;) {
                  t_gets(number,command,79);
                  if(equalni(command,"/fin",4)) {
                     t_fclose(number);
                     break;
                  }
                  RQ; fprintf(FP,"%s\r\n",command); RL;
               }
            }
            else {
               t_printf(number,"Error de Entrada/Salida. Comunique al Sysop\n");
            }
            break;
         case 'b' :
            if(confirm(number,"Elimina archivo ? ")) {
               RQ; unlink(workfile); RL;
            }
            break;
         case '?' :
            t_puts(number,"Comandos disponibles :\n");
            t_puts(number,"   ? - Ayuda\n");
            t_puts(number,"   B - Borrar     archivo\n");
            t_puts(number,"   T - Transmitir archivo\n");
            t_puts(number,"   A - Agregar a  archivo\n");
            t_puts(number,"   D - Desplegar  archivo\n");
            t_puts(number,"  XU - Xmodem Upload\n");
            t_puts(number,"  XD - Xmodem Download\n");
            t_puts(number,"   F - Finalizar\n");
            break;
      }
   }
}
