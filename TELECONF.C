/* teleconf.c
 * soporte para conferencias online
 *
 * primera version 14:16 29/01/90   (APR)
 *
 */

#ifndef CBBSMAIN
#   include "cbbs.h"
#endif

void send_to_conf(char *user, char *buf) {
#ifdef NODE1$UP
   char *s;
#endif
   RQ;
   sprintf(tconf[tconf_tail],"%-8s:%s",user,buf);
   tconf_tail++;
   tconf_tail%=MAXTELECONFLINES;
   RL;

#ifdef NODE1$UP
   s = buf;
   while (*s != '\0') {
      if (*s == SOH || *s == DLE || *s == EOT)
         *s = ' ';
      s++;
   }
   if (host_siop) {
      request_resource (&hostcommsrc, 0L);
      v24_printf(host_siop, "%c%c%-8s:%s%c", SOH, MSG_TELECONF, user, buf, EOT);
      release_resource (&hostcommsrc);
   }
#endif
}

void secretmsg(int number) {
   int l;
   char username[10];
   char buf[71];

   t_puts(number,"Secreto a usuario : ");
   t_gets(number,username,9);
   if(already_in(username,&l) && tty[l].tconf == tty[number].tconf) {
      t_putchar(number,'*');
      sprintf(buf,"*%-8s ",username);
      t_gets(number,buf+10,60);
      send_to_conf(tty[number].user,buf);
   }
   else {
      t_printf(number,"Error: usuario '%s' no conectado\n",username);
   }
}

void calluser(int number) {
   int l;
   char username[10];

   t_puts(number,"Llamar a usuario : ");
   t_gets(number,username,9);
   if(already_in(username,&l)) {
      if(tty[l].tconf == tty[number].tconf) {
         t_printf(number,"%s ya conectado a este canal de TELECONFerencia\n",username);
      }
      else {
#ifdef NODE1$UP
         if (l>4) {
            /* usuario en otro nodo, debiera estar en TELECONF ¨? */
         }
         else {
#endif
            if(tty[l].ocu) {
               t_printf(number,"Usuario '%s' ocupado. Envie Email.\n",username);
            }
            else {
               t_printf(l,"\n\n========>El usuario %s lo espera en TELECONFerencias (canal %d)\n\n",tty[number].user,tty[number].tconf);
            }
#ifdef NODE1$UP
         }
#endif
      }
   }
   else {
      t_printf(number,"Error: usuario '%s' no conectado\n",username);
   }
}

void read_from_conf(int number, int *thead) {
   char touser[71];
   for(;;) {
      RQ;
      if(*thead == tconf_tail) break;
      /* verificamos que no sea un mensaje privado */
      if(tconf[*thead][9] == '*') {
         sscanf(&(tconf[*thead][10]),"%s",touser);
         if(equali(touser,tty[number].user)) {
            t_printf(number,"%s\n",tconf[*thead]);
         }
      }
      else {
         t_printf(number,"%s\n",tconf[*thead]);
      }
      (*thead)++;
      (*thead)%=MAXTELECONFLINES;
      RL;
      t_delay(9L);
   }
   RL;
}

void teleconf(int number) {
   int tconf_head;
   char buf[71];
   long starttime;

   starttime = _clock;
   tty[number].tconf = TRUE;
   send_to_conf("CONEXION\a",tty[number].user);
   t_puts(number,"TeleConferencias\n");
   t_puts(number,"----------------\n");
   RQ; tconf_head = tconf_tail; RL;
#ifdef NODE1$UP
   if (host_siop) {
      request_resource (&hostcommsrc, 0L);
      v24_printf(host_siop, "%c%c%c%c", SOH, MSG_IN_TELECONF, (char) number + '0', EOT);
      release_resource (&hostcommsrc);
   }
#endif

   for(;;) {
      if(t_kbhit(number)) {
         t_putchar(number,'>');
         t_gets(number,buf,-70); /* el menos es un flag para las teleconfs */
         if     (equalni(buf,"/FIN"    ,4)) { newline; break; }
         else if(equalni(buf,"/WHO"    ,4)) { newline; show_who(number);   }
         else if(equalni(buf,"/QUIENES",8)) { newline; show_who(number);   }
         else if(equalni(buf,"/PERFIL" ,7)) { newline; showperfil(number); }
         else if(equalni(buf,"/CALL"   ,5)) { newline; calluser(number);   }
         else if(equalni(buf,"/LLAMAR" ,7)) { newline; calluser(number);   }
         else if(equalni(buf,"/SECRET" ,7)) { newline; secretmsg(number);  }
         else if(equalni(buf,"/HELP"   ,5)) { typefile(number,"teleconf.hlp"); }
         else if(equalni(buf,"/?"      ,2)) { typefile(number,"teleconf.hlp"); }
         else if(equalni(buf,"/AYUDA"  ,6)) { typefile(number,"teleconf.hlp"); }
         else if(buf[0] != '\0')        send_to_conf(tty[number].user,buf);
         else newline;
      }
      else t_delay(3L);
      read_from_conf(number,&tconf_head);
   }
   read_from_conf(number,&tconf_head);
   send_to_conf("DESCONEX\a",tty[number].user);
   tty[number].tconf = FALSE;
#ifdef NODE1$UP
   if (host_siop) {
      request_resource (&hostcommsrc, 0L);
      v24_printf(host_siop, "%c%c%c%c", SOH, MSG_NO_TELECONF, (char) number + '0', EOT);
      release_resource (&hostcommsrc);
   }
#endif
   update_cnfstat(number,"teleconf",(int) (_clock-starttime)/60);
}
