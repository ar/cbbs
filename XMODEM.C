/* xmodem.c
   Implementa las funciones XUPLOAD y XDOWNLOAD.
   Escrito por A.Revilla

   Version 1.00A se larga con release 1.00F del CBBS
*/

#ifndef CBBSMAIN
#   include <stdio.h>
#endif

typedef struct {
   unsigned char soh;
   unsigned char rec;
   unsigned char notrec;
   unsigned char data[128];
   unsigned char chksum;
   } XMODEM;

#define SOH 0x01
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18
#define EOT 0x04
#define OOS 0xFF  /* out of sequence */

#define XM_RETRY  10
#define XM_TIMOUT 180L

int  xm_waitack(sioptr siop);
int  rx_xmodem(sioptr siop,XMODEM *x);
void tx_xmodem(sioptr siop,XMODEM *x);
void xm_cancel(sioptr siop);
void xm_eot(sioptr siop);

void xm_cancel (sioptr siop) {
   register int i;
   for (i=0; i<8; i++) v24_send (siop,CAN,0L);
   for (i=0; i<8; i++) v24_send (siop,(char)8,0L);
}

void xm_eot (sioptr siop) {
   int retries;
   for (retries = 0; retries < XM_RETRY; retries++) {
      v24_send (siop,EOT,0L);
      if (xm_waitack(siop) == ACK) {
         retries = 0;
         break;
      }
   }
}

int xm_waitack(sioptr siop) {
   int retries, c;
   for (retries = 0; retries < XM_RETRY ; retries++) {
      c = v24_receive(siop,XM_TIMOUT);
      switch (c) {
         case ACK :
         case NAK :
         case CAN :
            return (c);
      }
   }
   return (0);
}

long xm_receive_file(int number, char *filename, char *lname) {
   long flen;
   int retries = 0, c, ocu_save;
   XMODEM x;

   if (!number) {
      t_puts(number,"XMODEM no disponible en ttyx00\n");
      return(128L);
   }

   unlink (filename);
   FP = t_fopen(number,filename,WRITE);
   if (FP == NULL) {
      t_printf(number,"Error accediendo archivo '%s'\n",lname);
      return(0L);
   }

   ocu_save = tty[number].ocu;
   tty[number].ocu = TRUE;
   t_printf(number,"Listo para recibir '%s' con XModem.\n",lname);
   sprintf (tty[number].doing, "RX:%-16.16s", lname);
   v24_protocol (tty[number].siop, 0x00, 40, 60);

   x.soh = SOH;
   x.rec = 1;
   v24_send(tty[number].siop,NAK,0L);
   flen = 0L;
   while (retries < XM_RETRY) {
      c = rx_xmodem(tty[number].siop,&x);
      switch (c) {
         case ACK :
            x.rec++;
            x.rec&=0xFF;
            v24_send(tty[number].siop,ACK,0L);
            fwrite(&x.data[0],128,1,FP);
            flen += 128;
            retries = 0;
            break;
         case NAK :
            v24_send(tty[number].siop,NAK,0L);
         case OOS :
            retries++;
            break;
         case CAN :
            v24_send(tty[number].siop,ACK,0L);
            retries = XM_RETRY;
            break;
         case EOT :
            v24_send(tty[number].siop,ACK,0L);
            retries = XM_RETRY;
            t_puts(number,"\nOK\n");
            break;
      }
      pause();
   }
   v24_protocol (tty[number].siop, XONXOFF, 40, 60);
   t_fclose(number);
   if (c != EOT)  {
      t_puts(number,"ERROR\n");
      unlink(filename);
   }
   tty[number].ocu = ocu_save;
   tty[number].doing[0] = '\0';
   return (c == EOT ? flen : 0L);
}

int rx_xmodem(sioptr siop,XMODEM *x) {
   register chksum;
   int i, c, seq;

   for (;;) {
      pause();
      c = v24_receive(siop,XM_TIMOUT);
      if (c == SOH) break;
      else if (c == CAN || c == EOT) return (c);
      else if (c == -1) return (NAK);
   }
   if (c != SOH) return(NAK);
   if ((seq = v24_receive(siop,XM_TIMOUT)) == -1) return(NAK);
   if ((c = v24_receive(siop,XM_TIMOUT)) == -1) return(NAK);
   if ((c^0xFF) != seq) return(NAK);
   for (i=0,chksum=0; i<128; i++) {
      if ((c = v24_receive(siop,XM_TIMOUT)) == -1) return(NAK);
      x->data[i] = (char) c;
      chksum += c;
   }
   chksum &= 0xFF;
   x->chksum = (unsigned char) v24_receive(siop,XM_TIMOUT);
   if (seq != x->rec) return(OOS);
   if ((unsigned char) chksum == x->chksum) return (ACK);
   else return(NAK);
}

void tx_xmodem(sioptr siop,XMODEM *x) {
   int i;
   v24_send (siop,x->soh,0L);
   v24_send (siop,x->rec,0L);
   v24_send (siop,x->notrec,0L);
   for (i=0; i<128; i++) {
      v24_send (siop,x->data[i],0L);
      pause();
   }
   v24_send (siop,x->chksum,0L);
}

int xm_transmite_file (int number, char *filename, char *lname) {
   XMODEM x;
   char c;
   int l, i, retries, ocu_save;

   x.soh = SOH;
   x.rec = 1;

   if (!number) {
      t_puts(number,"XMODEM no disponible en ttyx00\n");
      return(TRUE);
   }

   FP = t_fopen(number,filename,READ);
   if (FP == NULL) {
      t_printf(number,"ERROR: Archivo '%s' no accesible\n",lname);
      return (FALSE);
   }
   v24_protocol (tty[number].siop, 0x00, 40, 60);
   ocu_save = tty[number].ocu;
   tty[number].ocu = TRUE;
   t_printf(number,"Listo para transmitir '%s' con XModem.\n",lname);

   if (xm_waitack (tty[number].siop) != NAK) {
      /* Error, cancelamos transmision */
      xm_cancel (tty[number].siop);
      tty[number].ocu = ocu_save;
      return (FALSE);
   }

   sprintf (tty[number].doing, "TX:%-16.16s", lname);
   while ((l=fread(&x.data[0],1,128,FP)) > 0) {
      x.notrec = (x.rec ^ 0xFF);
      while (l < 128) x.data[l++] = '\0';
      for (i=0, x.chksum = 0; i < 128; i++) x.chksum += x.data[i];
      x.chksum &= 0xFF;
      for (retries = 0; retries < XM_RETRY; retries++) {
         tx_xmodem (tty[number].siop,&x);
         c = xm_waitack(tty[number].siop);
         if (c == ACK) {
            retries = 0;
            break;
         }
         else if (c != NAK) {
            retries = XM_RETRY;
         }
      }
      if (retries == XM_RETRY) {
         xm_cancel (tty[number].siop);
         t_fclose(number);
         t_puts (number,"ERROR\n");
         tty[number].ocu = ocu_save;
         tty[number].doing[0] = '\0';
         return (FALSE);
      }
      x.rec++;
      x.rec &= 0xFF;
   }
   xm_eot (tty[number].siop);
   t_fclose (number);
   v24_protocol (tty[number].siop, XONXOFF, 40, 60);
   tty[number].ocu = ocu_save;
   tty[number].doing[0] = '\0';
   return (TRUE);
}
