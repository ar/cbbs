/*
 * Comandos asociados a la toma de pedidos de la firma Metzen y Sena
 *
 */

void ped_myssa (int number) {
   int flag = FALSE, i;
   long starttime;
   char scratch[MAXPATH], codigo[14], s[81], cantidad[11];

   starttime = _clock;
   makefilename(scratch,SCRATCHPATH,tty[number].user,"WKR");

   t_puts (number, "\nMetzen y Sena S.A.\n");
   t_puts (number, "Sistema de pedidos en Tiempo Real\n\n");

   unlink (scratch);
   t_fopen(number, scratch, WRITE);
   FPAUX = fopen("/cbbs/dbf/myssa.txt","rb");

   for (;;) {
      t_puts (number, "C¢digo   : ");
      t_gets (number, s, -13);
      if (s[0] == '\0') break;
      sprintf (codigo, "%-13s", s);

      if (!bipart (s, codigo, FPAUX, 0L, 58, 13, 0)) {
         s[56] = '\0';
         t_printf (number, "      %s", s);
         t_puts (number, " Cant: ");
         t_gets (number, cantidad, -5);
         if (cantidad[0] != '\0') {
            t_printf (number, "\r%5s/%s", cantidad, s);
            if (confirm (number," Confirma ? ")) {
               fprintf (FP, "%-8s %5s %-13s\r\n", tty[number].user, cantidad, codigo);
               flag = TRUE;
            }
         }
         else {
            for (i=0; i<78; i++) t_putchar (number, ' ');
            t_putchar (number, '\r');
         }
      }
      else {
         t_printf (number, "Item '%s' no encontrado\n", codigo);
      }
   }
   fclose (FPAUX);
   FPAUX = 0L;
   t_fclose (number);
   if (flag) {
      if (confirm (number, "Confirma el pedido ? ")) {
         t_puts (number, "Aguarde por favor...");
         sendmail(number,tty[number].user, "MYSSA", "Pedido", 0L, "");
         t_puts (number, "\n");
      }
   }
   unlink (scratch);
   update_cnfstat(number,"pedmyssa",(int) (_clock-starttime)/60);
}

