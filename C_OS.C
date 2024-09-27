/* c_os.c
 *
 * Permite al sysop acceder al sistema operativo.
 * Modificaciones
 * 23:28 09/10/89   Version inicial (APR)
 *
 */

#ifndef CBBSMAIN
#   include <dir.h>
#   include <dos.h>
#   include <mem.h>
#   include "cbbs.h"
#endif

void datetime(int number, int mode) {
   struct date DATE;
   struct time TIME;
   char line[80];
   int  dia,mes,ano,hh,mm,ss;

   RQ;
   getdate(&DATE);
   gettime(&TIME);
   t_printf(number,"Conexion : %d minuto(s)\n",(_clock+59-tty[number].con_time)/60);
   RL;
   t_printf(number,"Fecha    : %02d/%02d/%02d %02d:%02d:%02d\n",
                                   DATE.da_day,
                                   DATE.da_mon,
                                   DATE.da_year-1900,
                                   TIME.ti_hour,
                                   TIME.ti_min,
                                   TIME.ti_sec);

   if(mode) {
      t_puts(number,"Nueva    : ");
      t_gets(number,line,80);
      if(strlen(line) > 0) {
         sscanf(line,"%d/%d/%d %d:%d:%d",&dia,&mes,&ano,&hh,&mm,&ss);
         t_printf(number,"           %02d/%02d/%02d %02d:%02d:%02d",dia,mes,ano,hh,mm,ss);
         if(confirm(number," Ok ? ")) {
            DATE.da_year = ano+1900;
            DATE.da_mon  = (char) mes;
            DATE.da_day  = (char) dia;

            TIME.ti_min  = (char) mm;
            TIME.ti_hour = (char) hh;
            TIME.ti_sec  = (char) ss;
            TIME.ti_hund = 0;

            preempt_off();
            setdate(&DATE);
            settime(&TIME);
#ifndef COOPERATIVE
	    preempt_on();
#endif
         }
      }
   }
}

void sconf(int number) {
   char conf[9], user[9], subject[41];
   t_puts(number,"Conferencia : ");
   t_gets(number,conf,8);
   t_puts(number,"Usuario     : ");
   t_gets(number,user,8);
   t_puts(number,"Tema        : ");
   t_gets(number,subject,40);
   t_printf(number,"Conf '%s' Usuario '%s' Tema '%s' ",conf,user,subject);
   if(confirm(number,"Ok ? ")) {
      sendconf(number,user,conf,subject,0L,"");
   }
}

void logbill(int number) {
   char user[9];
   char buf[11];
   int  tipo, mins;

   tipo = mins = -1;
   t_puts(number,"Movimiento de cuenta corriente\n");
   t_puts(number,"Usuario : ");
   t_gets(number,user,8);

   if(isuser(number,user)) {
      while(tipo < 0) {
         t_puts(number,"1=PAGO, 2=RETRIBUCION, 3=INVITACION, 4=EXTRATARIFADO -> ");
         t_gets(number,buf,10);
         tipo = buf[0]-'0';
      }
      while(mins < 0) {
         t_puts(number,"Minutos : ");
         t_gets(number,buf,10);
         mins = atoi(buf);
      }
      if(confirm(number,"Confirma ? ")) writebill(user,tipo,mins);
   }
}

void showbill(int number) {
   char fechahora[16];
   char *concepto;
   char user[9];
   SBILL rec;
   int saldo, conex, pagos, retribuc, lines;
   long mins;

   if(tty[number].issysop) {
      t_puts(number,"Usuario : ");
      t_gets(number,user,8);
      t_puts(number,"\n");
   }
   else {
      strcpy(user,tty[number].user);
   }

   saldo = conex = pagos = retribuc = lines = 0;
   mins  = 0L;

   t_fopen(number,BILLFILE,READ);
   if(FP != NULL) {
      t_printf(number,"Usuario  Fecha   Hora   Concepto     Mins. Saldo\n");
      t_printf(number,"------------------------------------------------\n");
      while(fread(&(rec.user),sizeof(SBILL),1,FP) == 1) {
         pause(); pause(); pause();
         if(equali(rec.user,user)) {
            RQ;
            st=localtime(&rec.time);
            sprintf(fechahora,"%02d/%02d %02d:%02d:%02d ",
                    st->tm_mday,(st->tm_mon+1),
                    st->tm_hour,st->tm_min,st->tm_sec);
            RL;
            switch(rec.type) {
               case CONEXION    : concepto = "CONEXION";
                                  saldo   -= rec.units;
                                  mins    += rec.units;
                                  conex++;
                                  break;
               case PAGO        : concepto = "SU PAGO";
                                  saldo   += rec.units;
                                  pagos   += rec.units;
                                  break;
               case RETRIBUCION : concepto = "COLABORACION";
                                  saldo   += rec.units;
                                  retribuc+= rec.units;
                                  break;
               case INVITACION  : concepto = "INVITACION";
                                  saldo   += rec.units;
                                  break;
               case INSCRIPCION : concepto = "SERV.EXTRA";
                                  saldo   -= rec.units;
                                  break;
               default          : concepto = "ERROR";
                                  break;
            }
            t_printf(number,"%-8s %s%-12s %5d %5d\n",rec.user,
                                                     fechahora,
                                                     concepto,
                                                     rec.units,
                                                     saldo);
            lines++;
            if(lines > TERMROWS) {
               if(!more(number)) break;
               else lines=0;
            }
         }
      }
      t_printf(number,"\nTOTALES\n-------\n");
      t_printf(number,"Cantidad de conexiones  : %d\n" ,conex);
      t_printf(number,"Minutos  de conexion    : %ld\n",mins);
      t_printf(number,"Pagos realizados        : %d minutos\n" ,pagos);
      t_printf(number,"Retribuciones recibidas : %d minutos\n" ,retribuc);
      t_printf(number,"Su balance actual es    : %d minutos %s\n",
                                                 abs(saldo),
                                                 saldo >= 0 ? "a favor" : "en contra");
      t_fclose(number);
      more(number);
   }
   else {
      t_printf(number,"'%s' no accesible",BILLFILE);
   }
}

void showlog(int number) {
   long fpos;
   char fechahora[16];
   int  inloop, lines = 0, conex = 0;
   SBILL rec;

   t_fopen(number,BILLFILE,READ);
   if(FP != NULL) {
      RQ;
      fseek(FP,0L,SEEK_END);
      fpos   = ftell(FP)-sizeof(SBILL);
      RL;
      inloop = TRUE && (fpos > 0L);
      if(inloop) fseek(FP,fpos,SEEK_SET);
      do {
         RQ;
         fread(&(rec.user),sizeof(SBILL),1,FP);
         fpos-=sizeof(SBILL);
         fseek(FP,fpos,SEEK_SET);
         inloop = (fpos > 0L);

         st=localtime(&rec.time);
         sprintf(fechahora,"%02d/%02d %02d:%02d:%02d ",
                 st->tm_mday,(st->tm_mon+1),
                 st->tm_hour,st->tm_min,st->tm_sec);
         RL;
         t_printf(number,"%-8s %s %1d %5d %5d\n",rec.user,
                                                 fechahora,
                                                 rec.type,
                                                 rec.units,
                                                 ++conex);
         lines++;
         if(lines > TERMROWS) {
            if(!more(number)) break;
            else lines=0;
         }
      } while(inloop);
      t_fclose(number);
   }
   else {
      t_printf(number,"'%s' no accesible",BILLFILE);
   }
}

void ls (int number, char *path) {
   struct dfree dfree;
   struct ffblk ffblk;
   long total_size = 0L, free;
   int done;

   if (tty[number].issysop)
      t_printf(number,"Directory of %s\n\n",path);
   RQ;
   done = findfirst(path,&ffblk,0);
   while (!done) {
      t_printf(number,"  %-12s %8ld\n",ffblk.ff_name,  ffblk.ff_fsize);
      total_size += ffblk.ff_fsize;
      done = findnext(&ffblk);
   }
   getdfree (0, &dfree);
   RL;
   free = (long) dfree.df_avail * dfree.df_bsec * dfree.df_sclus;
   t_puts  (number,"-----------------------\n");
   t_printf(number,"Total          %8ld  bytes\n",total_size);
   if (tty[number].issysop)
      t_printf(number,"Disponible     %8ld  bytes\n",free);
}

void cat(int number, char *file1, char *file2, int isappend) {
   char c;
   if(file1[0] == '\0') {
      t_puts(number,"invalid file name\n");
   }
   else {
      if(file2[0] == '\0') {
         t_printf(number,"Typing file '%s'\n",file1);
         typefile(number,file1);
      }
      else {
         if(isappend) {
            t_printf(number,"Appending file '%s' into '%s'\n",file1,file2);
            t_fopen(number,file1,READ);
            RQ;
            FPAUX = fopen(file2,"ab");
            RL;
         }
         else {
            t_printf(number,"Copying file '%s' into '%s'\n",file1,file2);
            t_fopen(number,file1,READ);
            RQ;
            FPAUX = fopen(file2,"wb");
            RL;
         }
         RQ;
         for (;;) {
            c=fgetc(FP);
            if (feof (FP)) break;
            fputc(c,FPAUX);
            pause();
         }
         RL;
         t_fclose(number);
         RQ;
         fclose(FPAUX);
         FPAUX = NULL;
         RL;
      }
   }
}

void c_os(int number) {
   char command[80], file1[MAXPATH], file2[MAXPATH], username[9];
   int isappend, i;

   t_printf(number,"\nCBBS VERSION %s\n",VERSION);
   for(;;) {
      t_puts(number,"$ ");
      setmem (command, 80, '\0');
      t_gets(number,command,80);
      if(equalni(command,"cat",3)) {
         file1[0] = file2[0] = '\0';
         if(strstr(command,">>") != NULL) {
            RQ; sscanf(command+3,"%s >> %s",file1,file2); RL;
            isappend = TRUE;
         }
         else {
            RQ; sscanf(command+3,"%s > %s",file1,file2); RL;
            isappend = FALSE;
         }
         cat(number, file1, file2, isappend);
      }
      else if(equalni(command,"mv",2)) {
         file1[0] = file2[0] = '\0';
         sscanf (command+3,"%s %s",file1,file2);
         if (rename (file1, file2) != 0) {
            t_puts(number, "Error\n");
         }
         else {
            t_puts(number, "Ok\n");
         }
      }
      else if(equalni(command,"rm",2)) {
         file1[0] = '\0';
         RQ; sscanf(command+2,"%s",file1); RL;
         if(file1[0] != '\0') {
            t_printf(number,"File %s\n",unlink(file1) ? "not deleted" : "deleted");
         }
      }
      else if(equalni(command,"ls",2)) {
         strcpy(file1,"*.*");
         RQ; sscanf(command+2,"%s",file1); RL;
         ls(number,file1);
      }
      else if(equalni(command,"shutdown",8)) {
         anywhere = confirm(number,"Desea tomar el control remoto ? ");
         for(i=0; i<MAXUSERS; i++) {
            if(i != number && connected(i) && tty[i].user[0] != '\0') {
               _t_disconne(i);
            }
         }
         wake_task(NULL);
         pause();
      }
      else if(equalni(command,"mail",4)) {
         RQ; sscanf(command+4,"%s",username); RL;
         strupr(username);
         if(isuser(number,username)) {
            t_printf(number,"Ok '%s'\n",username);
            cmail(number,"",username);
         }
      }
      else if(equalni(command,"sendconf",8)) sconf(number);
      else if(equalni(command,"showdisk",8)) showdisk (number, command+9);
      else if(equalni(command,"downdisk",8)) downdisk (number, command+9);
      else if(equalni(command,"bill"    ,4)) logbill(number);
      else if(equalni(command,"adduser" ,7)) adduser(number);
      else if(equalni(command,"date"    ,4)) datetime(number,TRUE);
      else if(equalni(command,"who"     ,3)) show_who(number);
      else if(equalni(command,"xd"      ,2)) {
         if (strlen(command) > 3) {
            xm_transmite_file (number,command+3,command+3);
         }
      }
      else if(equalni(command,"xu"      ,2)) {
         if (strlen(command) > 3) {
            xm_receive_file (number,command+3,command+3);
         }
      }
      else if(equalni(command,"monitor" ,7)) {
         show_who(number);
         t_puts(number,"\nChannel : ");
         t_gets(number,command,10);
         t_printf(number,"Monitor channel %d",atoi(command));
         if(confirm(number," ? ")) {
            monitor = atoi(command);
            mon_channel = monitor == 0 ? 0 : number;
         }
      }
      else if(equalni(command,"login"   ,5)) {
         t_puts(number,"Usuario : ");
         t_gets(number,username,8);
         strncpy(tty[number].user,username,8);
         tty[number].user[8] = '\0';
         strupr(tty[number].user);
         t_printf(number,"Nuevo username : '%s'\n",tty[number].user);
      }
      else if(equalni(command,"log"     ,3)) showlog(number);
#ifdef SNAPSHOT
      else if(equalni(command,"snap",4)) cbbs_snapshot(number);
#endif
#ifdef ALLOWOUTDIAL
      else if(equalni(command,"outdial" ,7)) outdial(number);
      else if(equalni(command,"delphi",  6)) {
         t_puts(number,"Conectando Delphi\n");
         delphi(number);
      }
#endif
      else if(equalni(command,"kickuser",8)) kickuser(number);
      else if(equalni(command,"f"       ,1)) break;
   }
}

void kickuser(int number) {
   int  i;
   char username[9];
   t_puts(number,"Usuario : "); t_gets(number,username,8);
   if (already_in(username,&i)) {
      _t_disconne(i);
      t_printf(number,"%s en ttyx%d out !\n",username,i);
   }
   else {
      t_printf(number,"Usuario '%s' no conectado\n",username);
   }
}

void writelog(int number,char *filename,char *s) {
   struct date DATE;
   struct time TIME;
   t_delay(3L);
   getdate(&DATE);
   gettime(&TIME);
   t_fopen(number,filename,APPEND);
   fprintf(FP,"%02d/%02d/%02d %02d:%02d:%02d %s: %s\r\n",
                                   DATE.da_day,
                                   DATE.da_mon,
                                   DATE.da_year-1900,
                                   TIME.ti_hour,
                                   TIME.ti_min,
                                   TIME.ti_sec,
                                   tty[number].user,
                                   s);
   t_fclose(number);
}

char *num2path (char *s, int disk, char *fname) {
   int centena;
   centena = (((disk-1)/100) * 100);
   if (disk < 901)
      sprintf (s, "e:/%03d_%03d/disk%04d/%s", centena+1, centena + 100, disk, fname);
   else if (disk < 1001)
      sprintf (s, "e:/%03d_%04d/disk%04d/%s", centena+1, centena + 100, disk, fname);
   else {
      sprintf (s, "e:/%04d%04d/disk%04d/%s", centena+1, centena + 100, disk, fname);
      s[7] = '_';
   }
   return s;
}

void showdisk (int number, char *dsk) {
   char filename[40];
   char s[256];
   int msg, lines=0;
   long offset=0L;

   msg = atoi (dsk);
   if (msg == 0) {
      t_puts (number, "Ingrese el n£mero de directorio : ");
      t_gets (number, s, 5);
      msg = atoi (s);
   }
   offset = i_seekmsg (CDROMCONF, msg);
   t_fopen (number,CDROMINFO,READTEXT);
   t_puts (number, "\n");
   if (FP != NULL) {
      fseek (FP, offset, SEEK_SET);
      while (fgets (s, 255, FP) != nil (char)) {
         if (atoi (s) != msg)
            break;
         if (!equal (s+5, "PC-SIG VERSION"))
            t_puts (number, s+5);
         lines++;
         if(lines > TERMROWS) {
            if(!more(number)) break;
            else lines=0;
         }
      }
      t_fclose(number);
   }
   else {
      t_puts (number,"Dispositivo de almacenamiento no accesible\n");
   }
   t_printf (number, "\nArchivos disponibles en directorio %04d :\n\n", msg);
   num2path (filename, msg, "*.*");
   ls (number, filename);
   t_puts (number, "\n");
}

void downdisk (int number, char *dsk) {
   char s[6];
   char fname[13];
   char filename[40];
   int msg;

   msg = atoi (dsk);
   if (msg == 0) {
      t_puts (number, "Ingrese n£mero de directorio  : ");
      t_gets (number, s, 5);
      msg = atoi (s);
   }
   t_puts (number, "Ingrese el nombre del archivo : ");
   t_gets (number, fname, 13);
   t_puts (number, "Aguarde por favor...\n");
   num2path (filename, msg, fname);
   if (xm_transmite_file (number, filename, fname))
      writelog (number,"/cbbs/cdrom.log",filename);
}
