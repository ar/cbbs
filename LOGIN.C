/* login.c
 * funciones referentes al login y policy de usuarios en el CBBS
 * Modificaciones
 * 17:49 05/10/89   Version inicial (APR)
 * 19:36 24/03/90   Se modifica para que compile solo
 */

#ifndef CBBSMAIN
#   include <fcntl.h>
#   include <dir.h>
#   include <string.h>
#   include "cbbs.h"
#endif

void show_who(int number) {
   int i;
   t_puts(number,"Terminal  Usuario\n");
   t_puts(number,"-----------------\n");
   for(i=0; i<MAXUSERS; i++) {
      if(tty[number].issysop) {
         t_printf(number,"ttyx%02x  : %-8s %5ld %s %d %d (%s)\n",i,
            tty[i].user[0] != '\0' ? tty[i].user : "-------",
            (_clock-tty[i].con_time)/60,
            tty[i].tconf ? " (TELECONF)" : "",
            tty[i].outdial,
            tty[i].status,
            tty[i].doing);
      }
      else {
        t_printf(number,"ttyx%02x  : %-8s%s\n",i,
		  tty[i].user[0] != '\0' ? tty[i].user : "-------",
		  tty[i].tconf ? " (TELECONF)" : "" );
      }
      pause();
   }
   while (i < 10) {
      t_printf(number,"ttyx%02x  : %-8s%s\n",i,
         tty[i].user[0] != '\0' ? tty[i].user : "-------",
         tty[i].tconf ? " (TELECONF Nodo 01)" : "" );
      i++;
   }
   if(tty[number].issysop) {
      t_printf (number, "Logins  = %d\n",loginseq);
      if (parsing_ips) {
         t_printf (number, "IPS = %d\n",check_counter (&ipscounter));
      }
   }
}

int already_in(char *user, int *n) {
   int i;
   for(i=0; i<10; i++) {
      if(equali(user,tty[i].user)) {
         *n = i;
         return(TRUE);
      }
      pause();
   }
   return(FALSE);
}

int isuser(int number, char *username) {
   char line[81], user[9];
   if(strlen(username)==0) {
      t_printf(number,"Nombre de usuario invalido\n");
      return(FALSE);
   }

   strupr(username);
   t_fopen(number,PWDFILE,READ);
   if(FP == nil(FILE)) {
      t_printf(number,"Archivo de claves no accesible !!!!!!!!!\n");
      t_printf(number,"por favor informe a CompuService por voz\n");
      t_printf(number,"al  telefono 48-24-62. Gracias y disculpe.\n");
      return(FALSE);
   }
   while(f_gets(line,80,FP) != nil(char)) {
      if(*username == *line) {
         RQ; sscanf(line,"%8s",user); RL;
         if(equal(user,username)) {
            t_fclose(number);
            return(TRUE);
         }
      }
      pause(); pause(); pause();
   }
   t_fclose(number);
   return(FALSE);
}

char *validuser(int number, char *username, char *password) {
   int  ttynumber;
   char user[9],
        pass[16],
        line[81];

   t_fopen(number,PWDFILE,READ);
   if(FP == nil(FILE)) {
      t_printf(number,"Archivo de claves no accesible !!!!!!!!!\n");
      t_printf(number,"por favor informe a CompuService por voz\n");
      t_printf(number,"al  telefono 48-24-62. Gracias y disculpe.\n");
      return(NULL);
   }
   strupr(username);
   strupr(password);
   while(f_gets(line,80,FP) != nil(char)) {
      if(*username == *line) {
         RQ; sscanf(line,"%8s %15s %12s",user,pass,tty[number].mini); RL;
         if(equal(username,user) && equal(password,pass)) {
            if(!already_in(user,&ttynumber)) {
               t_fclose(number);
               strncpy(tty[number].user,user,8);
               tty[number].user[8] = '\0';
               return(tty[number].mini);
            }
            else {
               t_printf(number,"Usuario '%s' YA CONECTADO a ttyx%02x\n",tty[ttynumber].user,ttynumber);
            }
         }
      }
      pause(); pause(); pause();
   }
   t_fclose(number);
   return(NULL);
}

long locateuser(int number, char *username, char *password) {
   long offset=0L;
   char user[9],
        pass[16],
        line[81];

   t_fopen(number,PWDFILE,READ);
   if(FP == nil(FILE)) {
      t_printf(number,"Archivo de claves no accesible !!!!!!!!!\n");
      t_printf(number,"por favor informe a CompuService por voz\n");
      t_printf(number,"al  telefono 48-24-62. Gracias y disculpe.\n");
      return(FALSE);
   }
   strupr(username);
   strupr(password);
   while(f_gets(line,80,FP) != nil(char)) {
      if(*username == *line) {
         RQ; sscanf(line,"%8s %15s",user,pass); RL;
         if(equal(username,user) && equal(password,pass)) {
            t_fclose(number);
            return(offset);
         }
      }
      offset = ftell(FP);
      pause(); pause(); pause();
   }
   t_fclose(number);
   return(-1L);
}

void newuser(int number) {
   char nombre[81],direccion[81], telefono[41], scratchfile[MAXPATH], username[9], password[9];
   makefilename(scratchfile,SCRATCHPATH,"NEWUSER","WKR");
   typefile(number,NEWUSERFILE1);

   t_puts(number,"Nombre completo   : "); 
   t_gets(number,nombre,80);
   if (strlen(nombre) < 5) t_disconnect(number);
   
   t_puts(number,"Direccion         : "); 
   t_gets(number,direccion,80);
   if (strlen(direccion) < 5) t_disconnect(number);
   
   t_puts(number,"Telefono(s)       : "); 
   t_gets(number,telefono,40);
   if (strlen(telefono) < 5) t_disconnect(number);

   t_puts(number,"\nDentro del sistema Ud. se identificar  mediante el uso\n");
   t_puts(number,"de un 'codigo de usuario' (de hasta 8 caracteres)\n");
   t_puts(number,"Se sugiere utilizar su apellido, nombre o abreviaturas de los mismos\n");
   t_puts(number,"para mayor claridad. Luego de su 'codigo de usuario'\n");
   t_puts(number,"que todos los demas usuarios conoceran, el sistema solicitara su\n");
   t_puts(number,"clave secreta que Ud. no debera revelar a nadie y es su 'llave' de.\n");
#ifdef TELNET
   t_puts(number,"acceso a la Red TelNet\n\n");
#else
   t_puts(number,"acceso a la Red CompuService\n\n");
#endif
   for(;;) {
      t_puts(number,"Codigo de usuario : "); t_gets(number,username,8);
      if(isuser(number,username)) {
         t_printf(number,"Lamentablemente el codigo '%s' ya ha sido seleccionado\n",username);
         t_puts(number,"por otro usuario. Reintente con otro codigo por favor\n");
      }
      else if(!validfname(username)); /* no action */

      else if(strlen(username) < 3) {
         t_puts(number,"El nombre de usuario debe contener al menos 3 caracteres\n");
      }
      else break;
   }

   t_puts(number,"\nAhora debera seleccionar una clave secreta de acceso al sistema\n");
   t_puts(number,"La misma puede contener hasta 8 caracteres y podra ser cambiada\n");
   t_puts(number,"cuando Ud. lo desee una vez confirmada su afiliacion al sistema\n\n");
   do {
      t_puts(number,"Clave secreta     : "); t_gets(number,password,8);
   } while(!validfname(password));


   t_puts(number,"Aguarde por favor...");
   if(existfile(number,scratchfile)) {
      RQ; unlink(scratchfile); RL;
   }
   t_fopen(number,scratchfile,WRITE);
   RQ;
   fprintf(FP,"Nombre : %s\r\n",nombre);
   fprintf(FP,"Direcc : %s\r\n",direccion);
   fprintf(FP,"Telefs : %s\r\n",telefono);
   fprintf(FP,"User   : %s\r\n",username);
   fprintf(FP,"Pass   : %s\r\n",password);
   RL;
   t_fclose(number);
   sendmail(number,"NEWUSER","SYSOP","NUEVO USUARIO",0L,"");
   typefile(number,NEWUSERFILE2);
   if(number) {
      v24_wait_complete(tty[number].siop,360L);
   }
}

void chgpassword(int number) {
   char currentpass[16], newpass1[16], newpass2[16];
   int  echo_save;
   long offset;
   t_puts(number,"\nCambio de palabra clave\n");
   t_puts(number,"-----------------------\n\n");
   echo_save = tty[number].echo;
   tty[number].echo = OFF;
   t_puts(number,"Ingrese su clave actual  : "); t_gets(number,currentpass,15); t_putchar(number,'\n');
   if((offset=locateuser(number,tty[number].user,currentpass)) >= 0L) {
      t_puts(number,"Ingrese nueva clave      : "); t_gets(number,newpass1,15); t_putchar(number,'\n');
      if (!validfname(newpass1)) {
         t_puts(number,"ERROR: Caracteres no validos en nueva clave\n");
      }
      else {
         t_puts(number,"Re-ingrese nueva clave   : "); t_gets(number,newpass2,15); t_putchar(number,'\n');
         tty[number].echo = echo_save;
         if(equali(newpass1,newpass2)) {
            if(equali(newpass1,currentpass)) {
               t_puts(number,"La nueva clave es igual a la actual. Cambio no realizado\n");
            }
            else {
               if(confirm(number,"Confirma cambio de clave ? ")) {
                  t_puts(number,"Aguarde, actualizando archivo de claves....\n");
                  t_fopen(number,PWDFILE,WRITE);
                  RQ;
                  fseek(FP,offset,SEEK_SET);
                  strupr(newpass1);
                  fprintf(FP,"%-8s %-15s",tty[number].user,newpass1);
                  RL;
                  t_fclose(number);
               }
               else {
                  t_puts(number,"Cambio de clave no realizado. Continua clave actual\n");
               }
            }
         }
         else {
            t_puts(number,"Error al ingresar nueva clave. Cambio no realizado\n");
         }
      }
   }
   else {
      t_puts(number,"Clave no valida\n");
   }
   tty[number].echo = echo_save;
}

void adduser(int number) {
   char username[9], password[16], mini[13], nombre[71];
   t_puts(number,"\nRegistro de nuevo usuario\n");
   t_puts(number,"-------------------------\n\n");
   t_puts(number,"Codigo       : ");
   t_gets(number,username,8);
   if(username[0] == '\0') {
      t_puts(number,"Codigo no valido\n");
   }
   else if(isuser(number,username)) {
      t_puts(number,"Usuario ya registrado\n");
   }
   else {
      t_puts(number,"Password     : "); t_gets(number,password,15);
      t_puts(number,"Menu inicial : "); t_gets(number,mini,12);
      t_puts(number,"Nombre       : "); t_gets(number,nombre,70);
      t_printf(number,"User : '%s'\nPass : '%s'\nMenu : '%s'\nName : '%s'\n",username,password,mini,nombre);
      if(confirm(number,"Confirma ? ")) {
         t_puts(number,"Aguarde, registrando nuevo usuario...\n");
         t_fopen(number,PWDFILE,APPEND);
         strupr(username);
         strupr(password);
         strlwr(mini);
         RQ; fprintf(FP,"%-8s %-15s %-12s\r\n",username,password,mini); RL;
         t_fclose(number);
         if(confirm(number,"Incorpora en lista de usuarios ? ")) {
            t_puts(number,"Agregando a 'dbf/users.txt'\n");
            t_fopen(number,"dbf/users.txt",APPEND);
            strupr(username);
            RQ; fprintf(FP,"%-8s  %s\r\n",username,nombre); RL;
            t_fclose(number);
         }
      }
   }
}

char *login(int number) {
   char username[16];
   char password[16];
#ifndef TELNET
   char empresa[41];
#endif
   char *menuinicial;
   int i;
   RQ; loginseq++; RL;

#ifdef TELNET
   t_printf(number,"\n\nRed TelNet - ttyx%02x\n\n",number);
#elif defined (CS1)
   t_printf(number,"\n\nCompuService - ttyx%02x - Nodo CS1\n\n",number);
#else
   t_printf(number,"\n\nCompuService - ttyx%02x\n\n",number);
#endif
   for(i=0; i<MAXLOGIN; i++) {
      tty[number].con_time = _clock;
      t_puts(number,"Usuario : ");
      t_gets(number,username,15);
      if(equalni(username,"NUEVO",5)) {
         strncpy(tty[number].user,username,8);
         newuser(number);
         t_disconnect(number);
         t_delay(0L);
      }
      t_puts(number,"Clave   : ");
      echooff(number);
      t_gets(number,password,15);
      t_putchar(number,'\n');
      if(tty[number].ttype != PACKET) {
         echoon(number);
      }
#ifndef TELNET
      if(equalni (username,"BSLOADER", 8) && equalni (password, "ABA3718H", 8)) {
         tty[number].ocu = ON;
         strncpy(tty[number].user, username,8);
         t_puts (number,"Empresa : ");
         t_gets (number,empresa, 30);
         if (xm_transmite_file (number, "BSUPDATE.EXE", "BSUPDATE.EXE")) {
            strcat (empresa, " OK");
            writelog(number,"BSLOADER.LOG", empresa);
         }
         else {
            strcat (empresa, " ERROR");
            writelog(number,"BSLOADER.LOG", empresa);
         }
         t_disconnect(number);
         t_delay(0L);
      }
#endif
      menuinicial = validuser(number,username,password);
      if(equalni (username,"APR", 3) && equalni (password, "TELNETSA", 8) ||
         equalni (username,"DFLC",4) && equalni (password, "TELNETSA", 8))
         menuinicial = "INICIAL.MNU";

      if(menuinicial == NULL) {
         t_puts(number,"\nError  en  codigo  de  Usuario  o  palabra  clave\n");
         t_puts(number,  "Si no dispone de codigo de usuario  digite  NUEVO\n");
         t_puts(number,  "En caso de dudas comuniquese por voz al 02-482462\n\n");
      }
      else {
         strncpy(tty[number].user,username,8);
         tty[number].user[8] = '\0';
         strupr(tty[number].user);
         tty[number].con_time = _clock;
#ifdef NODE1$UP
         if (host_siop) {
            request_resource (&hostcommsrc, 0L);
            v24_printf(host_siop, "%c%c%c%-8s%c", SOH, MSG_LOGIN, (char) (number+'0'), username, EOT);
            release_resource (&hostcommsrc);
         }
#endif
         pause();
         if (equalni(tty[number].user,"SYSOP",6) ||
            equalni(tty[number].user,"APR",4)   ||
            equalni(tty[number].user,"DFLC",5)) tty[number].issysop = TRUE;
         else tty[number].issysop = FALSE;
         pause();
#if 0 /* Ejemplo de Login con programa asociado */
         if (equalni(tty[number].user,"REUTERS",7)) {
            rxreuter(number);
            t_disconnect(number);
            t_delay(0L);
         }
#endif

         i=MAXLOGIN;
      }
   }
   return(menuinicial);
}

void writebill(char *user, billtype type, int units) {
   int handle;
   SBILL rec;
   strcpy(rec.user,user);
   rec.type  = type;
   rec.units = units;
   rec.time  = _clock;
   for(;;) {
      RQ;
      handle = open(BILLFILE,O_WRONLY  |
                             O_CREAT   |
                             O_APPEND  |
                             O_BINARY  |
                             O_DENYALL , S_IREAD | S_IWRITE);
      RL;
      if(handle < 0) {
         t_delay(48L);
         t_printf(0,"\nBILLFILE no accesible, reintentando\n");
      }
      else break;
   }
   RQ;
   if(write(handle,&(rec.user),sizeof(SBILL)) != sizeof(SBILL)) {
      t_printf(0,"\nERROR grabando BILLFILE\n");
   }
   close(handle);
   RL;
}
