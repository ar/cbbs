/* perfil.c
 * funciones para el manejo del perfil de los usuarios
 * Modificaciones
 * 18:02 10/12/89   Version inicial (APR)
 * 13:24 31/05/90   showlastcon solo chequea las ultimas 500 conexiones
 */

#ifndef CBBSMAIN
#   include <dir.h>
#   include "cbbs.h"
#endif

void showlastcon(int number, char *user) {
   char fechahora[16];
   int  inloop, l, i;
   SBILL rec;
   long fpos;
   if(already_in(user,&l)) {
      t_printf(number,"Usuario '%s' CONECTADO a ttyx%02x\n",user,l);
   }
   else {
      t_fopen(number,BILLFILE,READ);
      t_printf(number,"Aguarde por favor....");
      if(FP != NULL) {
         RQ;
         fseek(FP,0L,SEEK_END);
         fpos = ftell(FP) - sizeof(SBILL);
         RL;
         inloop = TRUE && (fpos > 0L);
         if(inloop) fseek(FP,fpos,SEEK_SET);
         i = 0;
         do {
            RQ;
            pause();
            fread(&(rec.user),sizeof(SBILL),1,FP);
            pause(); pause(); pause();
            fpos-=sizeof(SBILL);
            fseek(FP,fpos,SEEK_SET);
            pause(); pause();
            inloop = (fpos > 0L);
            RL;
            if(equali(rec.user,user)) {
               st=localtime(&rec.time);
               sprintf(fechahora,"%02d/%02d %02d:%02d:%02d ",
                     st->tm_mday,(st->tm_mon+1),
                     st->tm_hour,st->tm_min,st->tm_sec);
               t_printf(number,"\rUltimo contacto : %s",fechahora);
               inloop = FALSE;
            }
         } while(inloop && i++ < 300);
         t_puts (number,"\n");
         t_fclose(number);
      }
   }
}

void showperfil(int number) {
   char filename[MAXPATH];
   char user[9];

   t_puts(number,"Usuario : ");
   t_gets(number,user,8);
   t_puts(number,"\n");
   if(isuser(number,user)) {
      makefilename(filename,PERFILPATH,user,"PER");
      if(existfile(number,filename)) {
         typefile(number,filename);
         showlastcon(number,user);
      }
      else {
         t_printf(number,"%s no ha registrado su perfil en el sistema\n",user);
         showlastcon(number,user);
      }
   }
   else {
      t_printf(number,"'%s' no registrado en el sistema\n",user);
   }

   makefilename(filename,PERFILPATH,tty[number].user,"PER");
   if (!existfile (number, filename)) {
      t_puts (number, "\nUd. no ha registrado su perfil en el sistema.\n");
      t_puts (number, "Si desea hacerlo, puede utilizar el comando 'MIPERFIL'\n");
      t_puts (number, "Accesible desde cualquier menu.\n\n");
   }
}

void loadperfil(int number, char *user) {
   char filename[MAXPATH], command[81];
   makefilename(filename,PERFILPATH,user,"PER");
   if(existfile(number,filename)) {
      RQ; unlink(filename); RL;
   }
   t_fopen(number,filename,WRITE);
   if(FP != NULL) {
      t_printf(number,"Ingrese texto ('/FIN' finaliza)\n");
      RQ; fprintf(FP,"Usuario...............: %s\r\n",user);     RL;
      t_puts(number, "Nombre................: ");
      t_gets(number,command,79);
      RQ; fprintf(FP,"Nombre................: %s\r\n",command);  RL;
      t_puts(number, "Profesion.............: ");
      t_gets(number,command,79);
      RQ; fprintf(FP,"Profesion.............: %s\r\n",command);  RL;
      t_puts(number, "Ocupacion.............: ");
      t_gets(number,command,79);
      RQ; fprintf(FP,"Ocupacion.............: %s\r\n",command);  RL;
      t_puts(number, "Fecha de nacimiento...: ");
      t_gets(number,command,79);
      RQ; fprintf(FP,"Fecha de nacimiento...: %s\r\n",command);  RL;
      t_puts(number, "Computador............: ");
      t_gets(number,command,79);
      RQ; fprintf(FP,"Computador............: %s\r\n",command);  RL;
      t_puts(number, "Areas de interes......: ");
      t_gets(number,command,79);
      RQ; fprintf(FP,"Areas de interes......: %s\r\n\r\n",command);  RL;
      t_puts(number,"Notas : (/FIN finaliza)\n");
      for(;;) {
         t_gets(number,command,79);
         if(equalni(command,"/fin",4)) {
            t_puts(number,"Aguarde por favor....");
            t_fclose(number);
            break;
         }
         RQ; fprintf(FP,"%s\r\n",command); RL;
      }
      t_puts(number,"\nPerfil registrado. Gracias\n");
   }
   else {
      t_printf(number,"Error de Entrada/Salida. Comunique al Sysop\n");
   }
}
