/* CompuService Bulletin Board System (multiuser)
 * KEYWORDS: PENDIENTE, DEBUGGING
 * version  time   date   description
 *  1.00A  11:17 09/09/89 primera version
 * ------------------------------------------------
 *
 */

/* Para compilar todo en un solo modulo debe incluirse el
 * switch -DALL_IN_ONE en el tcc
 */

#define CBBSMAIN
#define ALL_IN_ONE
#include "cbbs.h"

#ifdef ALL_IN_ONE
#   include <conio.h>
#   include <string.h>
#   include <ctype.h>
#   include <stdlib.h>
#   include <dir.h>
#   include <io.h>
#   include <fcntl.h>
#   include <stdarg.h>
#   include <dos.h>
#   include <mem.h>
#   include <tskprf.h>

#   include "terminal.c"
#ifdef NODE1$UP
#   include "hostcomm.c"
#endif
#   include "login.c"
#   include "rfc822.c"
#   include "editor.c"
#   include "lib.c"
#   include "c_os.c"
#   include "cmail.c"
#   include "teleconf.c"
#   include "menues.c"
#   include "cfind.c"
#   include "taskctrl.c"
#   include "perfil.c"
#   include "cconf.c"
#ifndef TELNET
#   include "ips.c"
#endif
#   include "filectrl.c"
#   include "outdial.c"
#   include "xmodem.c"
#if 0
#   include "rxreuter.c"
#endif
#   include "bipart.c"
#   include "myssa.c"
#endif

unsigned int _stklen = 0xFFFF;

//-----------------------------------------------------------------------------
#ifdef TELNET
int prime [] = { 5717, 5843, 6481, 6577, 7243, 7451, 8363, 8429, 8539, 9371 };
int validkey = 0;
char key[120];
char actual_key [120];

int read_cifra (int port, int lugar, int c_save) {
   int i, cifra=0, readdata, zeros=0;
   if (lugar == 5) {
      /* caso particular de la 6ta cifra, pin de select */
      outportb (port, 0xFF);
      outportb (port+2, c_save | 2 | 8);
   }
   else {
      outportb (port, 0xFF - (1 << lugar));
   }
   t_delay (2L);
   readdata = inportb (port);
   for (i=0; i<8; i++) {
      outportb (port, (readdata & 31) | (i<<5));
      t_delay (2L);
      if ((inportb (port+1) & 0x40) == 0) {
         cifra = i;
         zeros++;
      }
   }

   if (lugar == 5) {
      outportb (port+2, c_save & 0xF7);
   }
   if (zeros == 1)
      return (cifra);
   else {
      validkey = 0;
      return (8);
   }
}

long read_key (int port) {
   int c, d_save, c_save, i;
   long clave=0L;

   if (port == 0) {
      validkey = 0;
      return 1888888L;
   }

   d_save = inportb (port);
   c = c_save = inportb (port+2);

   c &= 0xF7;
   c |= 2;
   outportb (port+2, c);

   validkey = 1;
   for (i=0; i<6; i++) {
      clave = clave*10 + read_cifra (port, i, c_save);
   }

   outportb (port,   d_save);
   outportb (port+2, c_save);
   return (clave + 1000000L);
}

void calc_key (void) {
   int i, *port;
   long lkey;
   char s[10];
   validkey = 0;
   port = (int *) MK_FP (0x40, 0x08);
   if (*port >= 0x0278)
      lkey = read_key (*port);
   if (!validkey) {
      port = (int *) MK_FP (0x40, 0x0A);
      if (*port >= 0x0278)
         lkey = read_key (*port);
   }
   actual_key[0] = '\0';
   for (i=0; i<10; i++) {
      sprintf (s, "%X", lkey % prime[i]);
      strcat (actual_key, s);
   }
}
#endif
//-----------------------------------------------------------------------------


/*********** TASKS ***********/

void far usertask(long num) {
   int number;
   char currentmenu[40];
   char filename[40];
   char *menuinicial;
   number = (int)num;
   while(!endrun) {
      tty[number].status   = WAITING_CALL;
      wait_for_connection(number);
      tty[number].status   = CONNECTED;
      tty[number].con_time = _clock;
      tty[number].user[0]  = '\0';
      menuinicial=login(number);
      if(menuinicial != NULL) {
         strcpy(currentmenu,menuinicial);
         t_printf(number,"\nBienvenido '%s'.\n",tty[number].user);
         checkformail(number);
         shownew(number,tty[number].user);
         makefilename(filename,PERFILPATH,tty[number].user,"OUT");
         if (existfile (number, filename)) {
            typefile (number, filename);
            t_disconnect (number);
         }
         typefile(number,"flash.msg");
         menu(number,currentmenu);
      }
      else {
         t_printf(number,"Acceso no autorizado\n");
         t_disconnect(number);
         pause();
      }
   }
}

void far watchdog(void) {
   char msg[81];
   int i;
#ifdef CS1
   long clockant=0L;
#endif

#ifdef AT_CS /* Enviamos un mensaje DUMMY de prueba en AT_CS */
   RQ;
   time(&_clock);
   st=localtime(&_clock);
   RL;
   v24_printf (ips_siop, "ZCZC Mensaje de prueba\r\n");
   v24_printf (ips_siop, "Este es el tema del mensaje.\r\n");
   v24_printf (ips_siop, "Este es el primer parrafo del mensaje. Realmente se trata de un parrafo bien largo ya que necesitamos que sea de mas de 80 caracteres para ");
   v24_printf (ips_siop, "comprobar que no se come ning£n caracter. Es una l stima que hasta ahora se venga comiendo caracteres. Ahora si, este es el fin del primer ");
   v24_printf (ips_siop, "p rrafo.-\r\n");
   v24_printf (ips_siop, "Esto va \aENTRE COMILLAS\a\r\n");
   v24_printf (ips_siop, "NNNN\r\n");
   v24_printf (ips_siop, "ZCZC Segundo Mensaje de prueba para deportes\r\n");
   v24_printf (ips_siop, "Este es el tema del segundo mensaje\r\n");
   v24_printf (ips_siop, "Este es la l¡nea del segundo mensaje\r\n");
   v24_printf (ips_siop, "(SIGUE/XX/XX/XX/SP).\r\n");
   v24_printf (ips_siop, "NNNN\r\n");
   v24_printf (ips_siop, "ZCZC Tercer Mensaje de prueba para deportes\r\n");
   v24_printf (ips_siop, "Este es el tema del tercer mensaje\r\n");
   v24_printf (ips_siop, "Este es la l¡nea del tercer mensaje\r\n");
   v24_printf (ips_siop, "Este va a parar a 'deportes' (SP)\r\n");
   v24_printf (ips_siop, "(FIN/XX/XX/XX/SP).\r\n");
   v24_printf (ips_siop, "NNNN\r\n");
#endif

   while(!endrun) {
      RQ;
      time(&_clock);
      st=localtime(&_clock);
      RL;
      t_delay(18L);
#ifdef CS1
      if (_clock - clockant > 15L) {
         inc_counter (&ipscounter);
         clockant = _clock;
      }
#endif
     /*
      * Se supervisan las comunicaciones
      */
      for(i=0; i<MAXUSERS; i++) {
         t_delay(3L);
         switch(tty[i].status) {
            case CONNECTED    :
               if(!connected(i)) {
                  if(tty[i].outdial) {
                     /* cortamos posible outdial */
                     if (indemodelphi) {
                        writelog(i,"/cbbs/delphi.log","WATCHDOG DISCONNECT");
                        indemodelphi = FALSE;
                     }
                     else {
                        writelog(i,"/cbbs/outdial.log","WATCHDOG DISCONNECT");
                     }
                     _t_disconne(tty[i].outdial);
                     t_delay(18L);
                     t_init(tty[i].outdial);
                     tty[i].outdial = 0;
                  }
                  /* se va a REQ_DISCONNE */
               }
               else {
                  if((tty[i].user[0] == '\0') && ((_clock - tty[i].con_time) > MAXLOGINTIME) && (tty[i].ttype == MODEM)) {
                     _t_disconne(i);
                  }
                  else if(equalni(tty[i].user,"NUEVO",5) && ((_clock - tty[i].con_time) > MAXNEWUSERTIME)) {
                     _t_disconne(i);
                  }
                  break;
               }                                             
            case REQ_DISCONNE :
               if(tty[i].user[0] != '\0') {
                  writebill(tty[i].user,CONEXION,(int) (_clock-tty[i].con_time+59)/60);
#ifdef NODE1$UP

                  if (host_siop) {
                     request_resource (&hostcommsrc, 0L);
                     v24_printf(host_siop, "%c%c%c%-8s%c", SOH, MSG_LOGOFF, (char) (i+'0'), tty[i].user, EOT);
                     release_resource (&hostcommsrc);
                  }
#endif
                  if (tty[i].tconf) {
                     strcpy (msg, tty[i].user);
                     strcat (msg, " ");
                     strcat (msg, "(Conexi¢n interrumpida)");
                     send_to_conf("DESCONEX", msg);
                  }
               }
               kill(i);
               t_init(i);
               break;
            case DISCONNECTED :
               if (i==0 || (i && tty[i].siop)) {
                  tty[i].status = CONNECT_IN_PROGRESS;
                  create(i);
                  start(i);
               }
               break;
         }
      }
   }
}

/*********** MAIN ***********/
#ifndef TELNET
extern void parse_ips_data (void);
#endif

int main (void) {
   int i;
#ifdef CS1
   FILE *fp;
   long offset;
#endif
   stkwatch = malloc (STACKSIZE);
   stkuser0 = malloc (STACKSIZE);
   stkuser1 = malloc (STACKSIZE);
   stkuser2 = malloc (STACKSIZE);
   stkuser3 = malloc (STACKSIZE);
#ifndef TELNET
   stkuser4 = malloc (STACKSIZE);
   stkuser5 = malloc (STACKSIZE);
   stkuser6 = malloc (STACKSIZE);
#endif
#ifdef NODE1$UP
   stkhost  = malloc (STACKSIZE);
#endif
#ifndef TELNET
   stkips   = malloc (STACKSIZE);
#endif

   endrun  = mon_channel = loginseq = monitor = anywhere = 0;
   tsk_set_currdis ();
   tsk_setpos (0,0);
   tsk_rprintf ("\f");
   harderr(hardhandler);
   load_conf_messages ();

   install_tasker (0, 0, IFL_DISK, (byteptr) "CBBSMAIN");
#ifndef COOPERATIVE
   create_resource(&tclib,"TCLIB");
#endif
#ifdef NODE1$UP
   create_resource(&hostcommsrc, (byteptr) "HOSTCOMM");
#endif
#ifndef TELNET
   create_counter (&ipscounter, (byteptr) "IPSCOUNT");
   parsing_ips = FALSE;
#endif
   create_task(&watchdog_tcb, watchdog, stkwatch, STACKSIZE, PRI_STD+1, NULL, (byteptr) "WATCHDOG");
#ifdef NODE1$UP
   create_task (&host_tcb,  hostcomm, stkhost,  STACKSIZE, PRI_STD+1, NULL, (byteptr) "HOSTCOMM");
#endif

#ifdef TELNET
   tty[0].siop = NULL; /* tty[0] == CONSOLE */
   tty[1].siop = v24_install(v24_define_port (M1PORT, M1IRQ, M1VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
   tty[2].siop = v24_install(v24_define_port (M2PORT, M2IRQ, M2VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
   tty[3].siop = v24_install(v24_define_port (M3PORT, M3IRQ, M3VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
#else
   create_task (&ips_tcb, ips_receive, stkips, STACKSIZE, PRI_STD+1, NULL, (byteptr) "IPS_RX");
   tty[0].siop = NULL; /* tty[0] == CONSOLE */
   tty[1].siop = v24_install(v24_define_port (M1PORT, M1IRQ, M1VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
#ifndef AT_CS
   tty[2].siop = v24_install(v24_define_port (M2PORT, M2IRQ, M2VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
   tty[3].siop = v24_install(v24_define_port (M3PORT, M3IRQ, M3VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
   tty[4].siop = v24_install(v24_define_port (M4PORT, M4IRQ, M4VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
   tty[5].siop = v24_install(v24_define_port (M5PORT, M5IRQ, M5VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
#ifndef CS1
   tty[6].siop = v24_install(v24_define_port (M6PORT, M6IRQ, M6VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
#endif
#endif
#endif
#ifdef NODE1$UP
   host_siop   = v24_install(v24_define_port (H1PORT, H1IRQ, H1VECT), 1, NULL, RXBUFSIZE, NULL, TXBUFSIZE);
   if (host_siop) {
      tsk_rprintf ("host_port Ok\r\n");
      v24_change_baud (host_siop, 2400L);
      v24_protocol (host_siop, 0, 40, 60); /* RTSCTS */
      v24_change_wordlength(host_siop, 8);
      v24_change_parity(host_siop, PAR_NONE);
      v24_change_stopbits(host_siop, 1);
   }
   else
      tsk_rprintf ("host_port no instalada\r\n");
#endif
#if !defined (TELNET) && !defined (CS1)
   ips_siop   = v24_install(v24_define_port (H1PORT, H1IRQ, H1VECT), 1, NULL, RXBUFSIZE+TXBUFSIZE, NULL, 64);
   /* maximo buffer de recepci¢n, esta puerta no transmite. Ponemos un minimo de 64 por si las moscas ----^  */
   if (ips_siop) {
      tsk_rprintf ("IPS Baudot Port Ok\r\n");
      v24_change_baud (ips_siop, 75L);
      v24_protocol (ips_siop, 0, 40, 60);
      v24_change_wordlength (ips_siop, 5);
      v24_change_parity (ips_siop, PAR_NONE);
      v24_change_stopbits (ips_siop, 1);
#ifdef AT_CS
      v24_change_wordlength (ips_siop, 8);
      v24_change_baud (ips_siop, 2400L);
#endif
   }
#endif
   /* inicializacion requerida por las teleconferencias */
   for(i=0; i<MAXTELECONFLINES; i++) tconf[i][0] = '\0';
   tconf_tail = 0;

   time(&_clock);
   for (i=0; i<10; i++)
      tty[i].user[0] = '\0';

   for(i=0; i<MAXUSERS; i++) {
      t_init(i);
   }
#ifdef NODE1$UP
   if (host_siop) {
      start_task(&host_tcb);
   }
#endif
#if !defined (TELNET) && !defined (CS1)
   if (ips_siop) {
      start_task(&ips_tcb);
   }
#endif
   start_task(&watchdog_tcb);
   set_priority(NULL,PRI_STD-1);

#ifndef COOPERATIVE
   preempt_on();
#endif

#ifdef TELNET
   calc_key ();
   if (strcmp (actual_key, "8E01BC13984FA4FE162520701E4D5A696A") == 0)
      t_delay (0L);
#else
   /* esperamos para procesar datos de IPS */
   for (;;) {
      if (wait_counter_set(&ipscounter, 0L) == WAKE) 
         break;
      else {
#ifdef CS1
         fp = fopen ("e:/dod/1_1000.upp", "rt");
         offset = ((long) random (10240)) * 512L;
         fseek (fp, offset, SEEK_SET);
         if (fread (stkips, 512, 1, fp) == 1 && tty[0].user[0] == '\0') {
            tsk_rprintf ("CD-ROM OK (offset=%ld)\r\n", offset);
         }
         fclose (fp);
#else
         parsing_ips = TRUE;
         parse_ips_data();
         parsing_ips = FALSE;
#endif
      }
   }
#endif
   RQ;
   clrscr();
   tsk_rprintf("Starting shutdown sequence\r\n");
   for(i=0; i<MAXUSERS; i++) {
      /* fclose (tty[i].fp);    */
      /* fclose (tty[i].fpaux); */
      if (i==0 || (i && tty[i].siop)) 
         kill(i);
   }
   preempt_off();
   RL;
   remove_tasker();
   clrscr();
   gotoxy(1,1);
   tsk_rprintf("CBBS VERSION %s - NORMAL SHUT DOWN",VERSION);
   gotoxy(1,1);
   return anywhere;
}
