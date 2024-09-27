/* terminal.c
 * funciones de manejo de terminal para el CompuService Bulleting Board System
 * Modificaciones
 * 17:48 05/10/89   Version inicial (APR)
 * 16:22 24/03/90   Se prepara para que compile solo. Se definen los
 *                  prototipos en cbbs.h
 */

#ifndef CBBSMAIN
#   include <conio.h>
#   include "cbbs.h"
#endif

int v24_getuntil (sioptr siop, char *buffer, int len, char until_c) {
   register int i;
   for (i=0; i<len; i++) {
      buffer[i] = (char) v24_receive (siop, 0L);
      if (buffer[i] == until_c)
         break;
   }
   buffer[i] = '\0';
   return (i);
}

void v24_puts (sioptr siop, char *string) {
   while(*string != NULL)
      v24_send (siop, *(string++), 0L);
}

void v24_printf(sioptr siop, const char *format, ...) {
   char output[240];
   va_list argptr;
   va_start (argptr, format);
   vsprintf(output, format, argptr);
   va_end (argptr);
   v24_puts(siop, output);
}

int connected(int number) {
   switch(tty[number].ttype) {
      case CONSOLE :
         return(TRUE);
      case PACKET :
      case MODEM :
        return(v24_modem_status(tty[number].siop) & CD);
      case LOCALTERM :
        return(v24_modem_status(tty[number].siop) & CTS);
   }
   return(FALSE);
}

void t_init(int number) {
   int i;
   if(mon_channel == number) monitor = mon_channel = 0;
   tty[number].echo     = ON;
   tty[number].status   = DISCONNECTED;
   tty[number].number   = number;
   tty[number].nivel    = 0;
   tty[number].issysop  = FALSE;
   tty[number].user[0]  = '\0';
   tty[number].tconf    = FALSE;
   tty[number].more     = TRUE;
   tty[number].ocu      = FALSE;
   tty[number].outdial  = 0;
   tty[number].doing[0] = '\0';
   RQ;
   if(FP    != NULL) fclose(FP);
   if(FPAUX != NULL) fclose(FPAUX);
   FP = FPAUX = NULL;
   for(i=0; i<MAXHANDLES; i++) {
      tty[number].handle[i] = 0;
   }
   RL;
   switch(number) {
      case 0 :
         tty[number].ttype = CONSOLE;
         break;
      case 1 :
         tty[number].ttype = MODEM;
         break;
      case 2 :
         tty[number].ttype = MODEM;
         break;
      case 3 :
         tty[number].ttype = MODEM;
         break;
      case 4 :
         tty[number].ttype = MODEM;
         break;
      case 5 :
         tty[number].ttype = MODEM;
         break;
      case 6 :
         tty[number].ttype = MODEM;
         break;
      case 7 :
         tty[number].ttype = MODEM;
         break;
      default :
         tty[number].ttype = NOTHING;
   }
   if(number) { /* no inicializa tty[0] que es la consola */
      if (tty[number].siop == NULL) {
         printf ("t_init: Unable to install driver for tty %d\n",number);
         tty[number].ttype = NOTHING;
      }
      else if (tty[number].siop) {
         flush_pipe(&tty[number].siop->xmit_pipe);
         v24_change_baud (tty[number].siop, BAUD);
         v24_protocol (tty[number].siop, 0x00, 40, 60);
         switch(tty[number].ttype) {
            case MODEM :
               v24_protocol (tty[number].siop, XONXOFF, 40, 60);
               break;
            case PACKET :
               v24_protocol (tty[number].siop, RTSCTS, 40, 60);
               break;
         }
         v24_change_wordlength(tty[number].siop, 8);
         v24_change_parity(tty[number].siop, PAR_NONE);
         v24_change_stopbits(tty[number].siop, 1);
         v24_change_dtr(tty[number].siop,ON);
         if (!v24_modem_status(tty[number].siop) & (CTS+DSR)) {
            t_delay(36L);
         }
      }
   }
}

void t_putchar(int number, char c) {
   if (number < MAXUSERS) {
      if (number) {
         if(c == '\n') {
            v24_send(tty[number].siop,'\r',0L);
         }
         v24_send(tty[number].siop,c,0L);
      }
      else if (number == 0) {
         if(c == '\n') {
            tsk_rprintf("\r\n");
         }
         else {
            tsk_rprintf("%c",c);
         }
      }
      if(monitor && monitor==number) {
         if(mon_channel) {
            if(c == '\n') {
               v24_send(tty[mon_channel].siop,'\r',0L);
            }
            v24_send(tty[mon_channel].siop,c,0L);
         }
         else {
            if(c == '\n') {
               tsk_rprintf("\r\n");
            }
            else {
               tsk_rprintf("%c",c);
            }
         }
      }
      pause();
   }
}

void t_puts(int number,char *string) {
   while(*string != NULL) {
      t_putchar(number,*string);
      string++;
   }
}

void t_printf(int number, const char *format, ...) {
   char output[240];
   va_list argptr;
   va_start (argptr, format);
   vsprintf(output, format, argptr);
   va_end (argptr);
   t_puts(number,output);
}

char t_getc(int number) {
   char c;
   if(number) {
      c=(char) v24_receive(tty[number].siop,0L);
   }
   else {
      c=(char) t_read_key();
   }
   /* if (c == '^') killretn(); */

   return(c);
}

int t_kbhit(int number) {
   if(number) {
      return(v24_check(tty[number].siop) != -1);
   }
   else {
      return(t_keyhit() != 0xFFFF);
   }
}

/* si len es < 0 no agregamos newline (para las teleconf) */
void t_gets(int number,char *string,int len) {
   int i=0, addnewline=TRUE;
   char c;
   if(len < 0) {
      len = -len;
      addnewline = FALSE;
   }
   do {
      c=t_getc(number);
      if (!addnewline) {
         if(c == '\n')   c = '\r';
         if(c == '\x1B') c = '[';
      }
      if(c != 8 && c != 127) {
         if(c != '\r') string[i++] = c;
         if(tty[number].echo) {
            switch(c) {
               case '\r': t_putchar(number,addnewline ? '\n' : '\r');
                          break;
               default  : t_putchar(number,c);
            }
         }
      }
      else {
         if(i>0) {
            i--;
            if(tty[number].echo) {
               t_putchar(number,8);
               t_putchar(number,32);
               t_putchar(number,8);
            }
         }
      }
   } while((c != '\r') && (i < len));
   if(tty[number].echo && i==len && addnewline) t_putchar(number,'\n');
   string[i] = '\0';
   if(!addnewline) {
      t_putchar(number,'\r');
      for(i=0; string[i] != '\0'; i++) t_putchar(number,' ');
      t_putchar(number,' ');
      t_putchar(number,'\r');
   }
}

void _t_disconne(int number) {
   CRITICAL;
   if(tty[number].ttype == MODEM) {
      C_ENTER;
      tty[number].siop->t_xoff = 0;
      C_LEAVE;
      v24_wait_complete(tty[number].siop,360L);
      pause();
      while(v24_receive(tty[number].siop,24L) != -1) pause();
      v24_change_dtr(tty[number].siop,OFF);
   }
}

void t_disconnect(int number) {
   if(tty[number].ttype == MODEM) {
      _t_disconne(number);
   }
   else {
      tty[number].status = REQ_DISCONNE;
   }
   t_delay(0L);
}

int confirm(int number, char *prompt) {
   char resp[4];
   t_puts(number,prompt);
   resp[0] = '\0';
   while(resp[0] != 'n' && resp[0] != 's') {
      t_gets(number,resp,3);
      resp[0] = tolower(resp[0]);
      resp[0] = resp[0]=='y' ? 's' : resp[0];
      if(resp[0] != 'n' && resp[0] != 's') {
         t_puts(number,"Por favor, responda Si o No.\n");
         t_puts(number,prompt);
      }
   }
   return(resp[0] == 's');
}

int more(int number) {
   char resp[4];
   if(tty[number].more) {
      t_puts(number,"\nMas? ");
      t_gets(number,resp,3);
      t_putchar(number,'\n');
   }
   else {
      resp[0] = '\r';
   }
   return((resp[0] != 'N') && (resp[0] != 'n'));
}

void espacio(int number) {
   char resp[2];
   t_puts(number,"\nPresione ESPACIO para continuar....");
   t_gets(number,resp,1);
   t_putchar(number,'\n');
}

void typefile(int number, char *filename) {
   int lines=0, c;
   t_fopen(number,filename,READTEXT);
   if(FP != NULL) {
      while(!feof(FP)) {
         c=fgetc(FP);
         if (c != (char) 0x1A && c != EOF) t_putchar(number,c);
         if(c == '\n') {
            lines++;
            if(lines > TERMROWS) {
               if(!more(number)) break;
               else lines=0;
            }
         }
      }
      t_fclose(number);
   }
   else {
      t_printf(number,"Archivo '%s' no accesible\n",filename);
   }
}

void init_modem(int number, long baud) {
   CRITICAL;
   C_ENTER;
   tty[number].siop->t_xoff = 0;
   tty[number].siop->r_xoff = 0;
   tty[number].echo         = OFF;
   C_LEAVE;
   v24_overrun(tty[number].siop); /* Clears overrun flag */
   v24_wait_complete(tty[number].siop,90L);
   v24_change_baud (tty[number].siop,baud);
   do {
      t_delay(9L);
      while(v24_receive(tty[number].siop,18L) != -1);
      t_puts(number,"AT Z\r");
      while(v24_receive(tty[number].siop,36L) != -1);
      t_puts(number,"AT E0 V0 M0 S0=0 X1\r");
      while(v24_receive(tty[number].siop,18L) != -1);
      t_puts(number,"AT\r");
   }
   while((char) v24_receive(tty[number].siop,36L) != '0');
}

#if 0
void init_packet(int number) {
   CRITICAL;
   C_ENTER;
   tty[number].siop->t_xoff = 0;
   tty[number].siop->r_xoff = 0;
   tty[number].echo         = OFF;
   C_LEAVE;
   v24_overrun(tty[number].siop); /* Clears overrun flag */
   v24_wait_complete(tty[number].siop,90L);
   v24_change_baud (tty[number].siop,BAUD);
   t_delay(54L);
   t_putchar(number,3); /* ^C */
   t_putchar(number,3);
   t_putchar(number,3);
   t_delay(54L);
   t_puts(number,"DISCONNE\r");
   t_delay(54L);
   t_puts(number,"CONMODE CONVERSE\r");
   t_delay(54L);
}
#endif

void wait_for_connection(int number) {
   char c;
   int try=0;
   switch(tty[number].ttype) {
      case CONSOLE :
         break;
      case LOCALTERM :
         while(!connected(number)) t_delay(18L);
         break;
      case PACKET :
         v24_watch_modem(tty[number].siop,0);
         while(!connected(number)) t_delay(54L);
         while(v24_receive(tty[number].siop,36L) != -1);
         v24_watch_modem(tty[number].siop,CTS | DSR | CD);
         tty[number].echo = ON;
#if 0
         init_packet(number);
         do {
            t_gets(number,rx_buf,80);
         } while(strnicmp(rx_buf+1,"*** CONNECTED",13) != 0);
         tty[number].echo = OFF;
         t_putchar(number,3);
         t_delay(36L);
         t_puts(number,"TRANS\r");
         while(v24_receive(tty[number].siop,180L) != -1);
#endif
         break;
      case MODEM :
         v24_watch_modem(tty[number].siop,0);
         init_modem(number,BAUD);
         do {
            c=(char) v24_receive(tty[number].siop,0L);
            switch(c) {
               case '1' : /* connect  300 bps */
                  v24_change_baud (tty[number].siop, 300L);
                  break;
               case '5' : /* connect 1200 bps */
                  break;
               case '2' : /* RING */
                  try = 0;
                  t_delay(9L);
                  t_puts(number,"AT S7=15 A\r");
                  break;
               case '3' : /* no carrier */
                  if(try==0) {
                     t_delay(9L);
                     t_puts(number,"AT S7=15 A\r");
                     try++;
                  }
                  else try=0;
                  break;
            }
         } while(c != '1' && c != '5');
         /* nos comemos posible basura del modem */
         while(v24_receive(tty[number].siop,36L) != -1);
         v24_watch_modem(tty[number].siop,CTS | DSR | CD);
         tty[number].echo = ON;
         break;
   }
   tty[number].status = CONNECTED;
   tty[number].con_time = _clock;
}

int t_waitfor(int number, char *waitstring, int timeout) {
   long maxtime;
   int  i,len;

   maxtime = _clock + timeout;
   for(i=0,len=strlen(waitstring); i<len; ) {
      if(v24_check(tty[number].siop) != -1) {
         if((char)v24_receive(tty[number].siop,180L) == waitstring[i]) i++;
         else i=0;
         pause();
      }
      else t_delay(3L);
      if(_clock > maxtime || !connected(number)) break;
   }
   return(i==len);
}
