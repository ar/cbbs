/* Outdial.c
 *
 * Agrupamos en este archivo todos los programas correspondientes a
 * llamados salientes ya sea el sysop con el comando OUTDIAL desde el SYSTEM
 * o un simple usuario accediendo a terceros proveedores.
 *
 */

#ifndef CBBSMAIN
#   include "cbbs.h"
#endif

int find_line(int number) {
   int i;
   for(i=0; i<MAXUSERS; i++) {
      if(tty[i].status == WAITING_CALL && tty[i].ttype == MODEM) {
         tty[number].outdial = i;
         tty[i].status = OUTDIAL;
         kill(i);
         break;
      }
   }
   return(i < MAXUSERS ? i : 0);
}

void outdial(int number) {
   int i, echosave;
   char c,command[11];
   show_who(number);
   t_puts(number,"\nOut line : ");
   t_gets(number,command,10);
   i = atoi(command);
   i = (i > MAXUSERS-1 || i < 0) ? 0 : i;
   if (i) {
      tty[number].outdial = i;
      tty[i].status = OUTDIAL;
      t_printf(number,"COM com%d. ~ Finaliza",i);
      kill(i);
      t_puts(number,"\n\n");
      echosave = tty[number].echo;
      tty[number].echo = OFF;
      tty[i].echo      = OFF;

      for(;;) {
         if (t_kbhit(i)) t_putchar(number,t_getc(i));
         else pause();
         if (t_kbhit(number)) {
            c = t_getc(number);
            if (c == '~') break;
            t_putchar(i,c);
         }
         else pause();
      }

      t_puts(number,"\nDesconectando....\n");
      tty[number].echo = echosave;
      _t_disconne(i);
      t_delay(18L);
      t_init(i);
      tty[number].outdial = 0;
   }
}

int call_system(int number, int baud, int bits, int parity, 
                int stops, int retries, char **telephone) 
{
   int out, i;
   char connect_char;
   if (baud != 1200 && baud != 300) {
      t_printf(number,"Error. %d baudios no disponible\n",baud);
      return(0);
   }

error_recovery:

   out = find_line(number);
   if (out) {
      /* init_modem(out,baud); */
      v24_change_wordlength(tty[out].siop, bits);
      v24_change_parity(tty[out].siop, parity);
      v24_change_stopbits(tty[out].siop, stops);
      v24_receive(tty[out].siop,9L); /* Nos comemos supuesto Enter */

      connect_char = (baud == 300 ? '1' : '5');
      for(i=0; i<retries; i++) {
         while(v24_receive(tty[out].siop,18L) != -1); /* Nos comemos posible basura */
         t_printf(out,"AT DT W%s\r",telephone[i]);
         if (connected(out)) {
            _t_disconne(out);
            t_delay(18L);
            t_init(out);
            tty[number].outdial = 0;
            t_delay(36L);
            goto error_recovery;
         }
         t_putchar(number,'.');
         /* t_printf(number,"Dialing '%s'\n",telephone[i]); */
         if (v24_receive(tty[out].siop,720L) == connect_char) {
            /* tty[out].status = CONNECTED; */
            t_puts(number,"Conectado\n");
            return(out);
         }
         else {
            t_puts(out," ");
         }
      }
      _t_disconne(out);
      t_delay(18L);
      t_init(out);
      tty[number].outdial = 0;
      return(0);
   }
   else {
      t_printf(number,"No existen lineas disponibles para realizar conexion\n");
      return(0);
   }
}

int delphilogon(int number, int out, char *user, char *pwd) {
   int tries;

   t_puts(number,"Conectado con Delphi Uruguay\n");
   for(tries=0; connected(out) && tries<3; tries++) {
      t_putchar(out,'\r');
      t_waitfor(out,":",20);
      t_printf(out,"%s\r",user);
      t_waitfor(out,":",20);
      t_printf(out,"%s\r",pwd);
      if(t_waitfor(out,"Hola",10)) {
         t_puts(number,"\n\nHola");
         return(TRUE);
      }
   }
   _t_disconne(out);
   return(FALSE);
}

void gateway(int number, int out, int timeout) {
   long maxtime;
   char c;

   maxtime = _clock + timeout;
   while(connected(out)) {
      if (t_kbhit(out)) t_putchar(number,t_getc(out));
      else pause();
      if (t_kbhit(number)) {
         c = t_getc(number);
         /* if (c == '~') break; */ /* no lo ponemos para aceptar Xmodem */
         t_putchar(out,c);
         if(c == (char)0x0d) {
            t_delay(9L); /* Igual nos demora el servicio que sea */
         }
      }
      else pause();
      if (_clock > maxtime) {
         _t_disconne(out);
         t_puts(number,"\n\n\n\n\n\n\n\n\n\n");
      }
   }
}

void demodelphi(int number) {
   int out, echosave;

   char *phones[] = { "981806", "981702", "981369" };

   if(indemodelphi) {
      t_puts(number,"Existe otro usuario accediendo a Delphi en Demostracion\n");
      t_puts(number,"Reintente luego de algunos minutos\n");
      return;
   }

   typefile(number,"/cbbs/msg/demodelp.msg");
   if(confirm(number,"Confirma acceso ? ")) {
      if(confirm(number,"Esta seguro ?     "));
      else return;
   }
   else return;
   writelog(number,"/cbbs/delphi.log","Calling");
   t_puts(number,"Conectando");
   out = call_system(number,1200,8,PAR_NONE,1,sizeof(*phones),phones);
   t_delay(36L);
   if(out) {
      echosave = tty[number].echo;
      tty[number].echo = tty[out].echo = OFF;
      indemodelphi=TRUE;
      if(delphilogon(number,out,"DEMOURU2","DCPOWER")) {
         writelog(number,"/cbbs/delphi.log","LOGIN");
         gateway(number,out,7200);
         if (!connected(out)) {
            t_puts(number,"\n\nFin de la comunicacion\n");
         }
         else {
            t_puts(number,"\nDesconectando\n");
         }
         writelog(number,"/cbbs/delphi.log","LOGOFF");
      }
      else {
         t_puts(number,"\n\nSistema Delphi fuera de servicio. No acepta logon.\n\n");
      }
      _t_disconne(out);
      t_delay(18L);
      indemodelphi = FALSE;
      t_init(out);
      tty[number].outdial = 0;
      tty[number].echo = echosave;
   }
   else {
      t_puts(number,"Imposible establecer comunicacion\n");
   }
}

void delphi(int number) {
   int out, echosave;

   char *phones[] = { "981806", "981702", "981369" };
   char pwddelphi[MAXPATH], username[16], password[16];

   makefilename (pwddelphi, "/cbbs/mail/", tty[number].user, "MIA");


   FP = t_fopen (number,pwddelphi,READ);
   if (FP == NULL) {
      typefile (number,"/cbbs/delpuser.msg");
      espacio (number);
      return;
   }

   f_gets (username,16,FP);
   f_gets (password,16,FP);
   t_fclose (number);

   writelog(number,"/cbbs/outdial.log","Calling Delphi");
   t_puts(number,"Conectando");
   out = call_system(number,1200,8,PAR_NONE,1,sizeof(*phones),phones);
   t_delay(36L);
   if(out) {
      echosave = tty[number].echo;
      tty[number].echo = tty[out].echo = OFF;
      if(delphilogon(number,out,username,password)) {
         writelog(number,"/cbbs/outdial.log","LOGIN");
         gateway(number,out,10800);
         if (!connected(out)) {
            t_puts(number,"\n\nFin de la comunicacion\n");
         }
         else {
            t_puts(number,"\nDesconectando\n");
         }
         writelog(number,"/cbbs/outdial.log","LOGOFF");
      }
      else {
         t_puts(number,"\n\nSistema Delphi fuera de servicio. No acepta logon.\n\n");
      }
      _t_disconne(out);
      t_delay(18L);
      t_init(out);
      tty[number].outdial = 0;
      tty[number].echo = echosave;
   }
   else {
      t_puts(number,"Imposible establecer comunicacion\n");
      t_puts(number,"Sirvase reintentar luego de algunos minutos\n");
   }
}
