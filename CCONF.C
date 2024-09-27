/* cconf.c
   -- CompuService Mail Program
   --
   -- Funcionamiento de las conferencias :
   -- ----------------------------------
   -- El archivo es similar al de mails con la asistencia de un indice
   -- (un long atras del otro). No se usa el .SEQ ya que el numero se
   -- saca del .IDX.
   --
   --
   -- Modificaciones
   -- 23:28 08/10/89   Version inicial (APR)
   -- 22:38 24/03/90   Se modifica para que compile standalone
   -- 09:23 09/04/90   Se re-modifica para que se compile opcional standalone
   -- 00:50 09/06/90   Se agrega estadistica de uso
   -- 18:17 30/01/91   El comando 'nuevo' actua en memoria sobre conf_messages[MAXCONFS]
*/

#ifndef CBBSMAIN
#   include <dir.h>
#   include <stdlib.h>
#   include "cbbs.h"
#endif

int i_addmsg(char *confname, long offset) {
   FILE *fp;
   int  msgno;
   char fname[MAXPATH];
   makefilename(fname,CONFPATH,confname,"IDX");
   RQ;
   fp = fopen(fname,"a+b");
   fseek(fp,0L,SEEK_END);
   msgno = (int)(ftell(fp)/4)+1;
   fwrite(&offset,sizeof(long),1,fp);
   fclose(fp);
   RL;
   pause();
   return(msgno);
}

long i_seekmsg(char *confname, int msgno) {
   FILE *fp;
   long offset=0L;
   char fname[MAXPATH];
   makefilename(fname,CONFPATH,confname,"IDX");
   RQ;
   fp = fopen(fname,"rb");
   fseek(fp,(msgno-1)*sizeof(long),SEEK_SET);
   fread(&offset,sizeof(long),1,fp);
   fclose(fp);
   RL;
   pause();
   return(offset);
}

long i_modimsg(char *confname, int msgno, long newoffset) {
   FILE *fp;
   long offset;
   char fname[MAXPATH];
   makefilename(fname,CONFPATH,confname,"IDX");
   RQ;
   fp = fopen(fname,"r+b");
   fseek(fp,(msgno-1)*sizeof(long),SEEK_SET);
   fread(&offset,sizeof(long),1,fp);
   fseek(fp,(msgno-1)*sizeof(long),SEEK_SET);
   fwrite(&newoffset,sizeof(long),1,fp);
   fclose(fp);
   RL;
   pause();
   return(offset);
}

int i_maxmsg(char *confname) {
   int i;

   for (i=0; i < MAXCONFS; i++)
      if (equali (confname, conf_messages[i].confname))
         return (conf_messages[i].lastmsg);
   return (-1);
}

void inc_lastmsg (char *confname) {
   int i;
   for (i=0; i < MAXCONFS; i++)
      if (equali (confname, conf_messages[i].confname)) {
         conf_messages[i].lastmsg++;
         break;
      }
}

int maxmsg(char *confname) {
   FILE *fp;
   int msg;
   char fname[MAXPATH];
   makefilename(fname,CONFPATH,confname,"IDX");
   fp = fopen(fname,"rb");
   fseek(fp,0L,SEEK_END);
   msg = (int) ftell(fp)/4;
   fclose(fp);
   return(msg);
}

void load_conf_messages (void) {
   struct ffblk ffblk;
   char path[MAXPATH], filename[14], *s;
   int done, i=0;

   makefilename(path, CONFPATH, "*", "CNF");
   done = findfirst(path,&ffblk,0);
   while (!done) {
      strcpy (filename, ffblk.ff_name);
      s = strchr (filename, '.');
      *s = '\0';
      strcpy (conf_messages[i].confname, filename);
      conf_messages[i].lastmsg = maxmsg (filename);
      i++;
      done = (findnext(&ffblk) || (i == MAXCONFS));
   }
   if (i < MAXCONFS)
      conf_messages[i].confname[0] = '\0';
}

int update_cnfinfo(int number, char *user, char *conf, int maxmsg) {
   int     updated = FALSE, oldmax = 0, len;
   long    offset;
   char    infofile[MAXPATH];
   CNFINFO info;

   len = strlen(conf);
   strlwr(conf);
   makefilename(infofile,CINFOPATH,user,"DAT");
   FP = t_fopen(number,infofile,READ);
   if(FP == NULL) {
      /* archivo no existente, se crea... */
      FP = t_fopen(number,infofile,WRITE);
      t_fclose(number);
      FP = t_fopen(number,infofile,READ);
   }
   if(FP != NULL) {
      offset = 0L;
      while(fread(&info,sizeof(CNFINFO),1,FP) == 1) {
         if(equalni(info.confname,conf,len)) {
            t_fclose(number);
            oldmax = info.lastmsg;
            if(maxmsg != oldmax && maxmsg != 0) {
               FP = t_fopen(number,infofile,WRITE);
               RQ;
               /* t_printf(number,"Actualizando '%s.dat'\n",user); */
               fseek(FP,offset,SEEK_CUR);
               info.lastmsg = maxmsg;
               fwrite(&info,sizeof(CNFINFO),1,FP);
               RL;
               t_fclose(number);
            }
            updated = TRUE;
            break;
         }
         offset = ftell(FP);
         pause();
      }
      t_fclose(number);
      if(!updated) {
         FP = t_fopen(number,infofile,APPEND);
         strcpy(info.confname,conf);
         info.lastmsg = maxmsg;
         RQ; fwrite(&info,sizeof(CNFINFO),1,FP); RL;
         /* t_printf(number,"Actualizando '%s.dat'\n",user); */
         t_fclose(number);
      }
   }
   else {
      t_printf(number,"Error accediendo '%s.dat'. Por favor informe a sysop\n",user);
   }
   return(oldmax);
}

void update_cnfstat(int number, char *conf, int minutes) {
   int     updated = FALSE, len;
   long    offset;
   char    infofile[MAXPATH];
   CNFINFO info;

   if (minutes < 2) return;
   if (tty[number].issysop) return; /* no nos vamos a cobrar a nosotros mismos ! */

   len = strlen(conf);
   strlwr(conf);
   makefilename(infofile,CONFPATH,"CONFSTAT","DAT");
   FP = t_fopen(number,infofile,READ);
   if(FP == NULL) {
      /* archivo no existente, se crea... */
      FP = t_fopen(number,infofile,WRITE);
      t_fclose(number);
      FP = t_fopen(number,infofile,READ);
   }
   if(FP != NULL) {
      offset = 0L;
      while(fread(&info,sizeof(CNFINFO),1,FP) == 1) {
         if(equalni(info.confname,conf,len)) {
            t_fclose(number);
            info.lastmsg += minutes;
            FP = t_fopen(number,infofile,WRITE);
            RQ;
            fseek(FP,offset,SEEK_CUR);
            fwrite(&info,sizeof(CNFINFO),1,FP);
            RL;
            t_fclose(number);
            updated = TRUE;
            break;
         }
	 offset = ftell(FP);
	 pause();
      }
      t_fclose(number);
      if(!updated) {
         FP = t_fopen(number,infofile,APPEND);
         strcpy(info.confname,conf);
         info.lastmsg = minutes;
         RQ; fwrite(&info,sizeof(CNFINFO),1,FP); RL;
         t_fclose(number);
      }
   }
   else {
      t_printf(number,"Error accediendo 'confstat.dat'. Por favor informe a sysop\n");
   }
}

int new_messages(char *conf, int lastmsg) {
   return(max(i_maxmsg(conf) - lastmsg,0));
}

void shownew(int number, char *user) {
   CNFINFO info;
   char    infofile[MAXPATH];
   int     newmsgs, flag=TRUE;

   makefilename(infofile,CINFOPATH,user,"DAT");
   FP = t_fopen(number,infofile,READ);
   if(FP == NULL) {
      t_puts(number,NOTJOINED);
   }
   else {
      while(fread(&info,sizeof(CNFINFO),1,FP) == 1) {
         if(info.lastmsg > 0) {
            newmsgs = new_messages(info.confname,info.lastmsg);
            if(newmsgs && flag) {
               flag = FALSE;
               t_puts(number,"Conferencia  Mensajes\n");
               t_puts(number,"---------------------\n");
            }
            if(newmsgs) {
               t_printf(number,"%-8s        %5d\n",info.confname,newmsgs);
            }
         }
         pause();
      }
      t_fclose(number);
      if(flag) {
         t_puts(number,"No existen nuevos mensajes\n");
      }
   }
}

void showstat(int number, char *conf) {
   CNFINFO info;
   char    infofile[MAXPATH];
   int     len;

   len = strlen(conf);
   strlwr(conf);
   makefilename(infofile,CONFPATH,"confstat","DAT");
   FP = t_fopen(number,infofile,READ);
   t_puts(number,"Conf.    Mins.\n");
   t_puts(number,"--------------\n");
   if(FP == NULL) {
      t_puts(number,"No existe informacion hasta el momento\n");
   }
   else {
      while(fread(&info,sizeof(CNFINFO),1,FP) == 1) {
         if(tty[number].issysop || equalni(info.confname,conf,len)) {
            t_printf(number,"%-8.8s %5d\n",info.confname,info.lastmsg);
            if (!tty[number].issysop) break;
         }
         pause();
      }
      t_fclose(number);
   }
}

void showconf(int number,char *confname, int msgini, int msgfin, char *search) {
   char msg[6], from[9], date[9], time[9], tema[41], name[17], fname[MAXPATH];
   char wtema[41];
   long offset;
   int  maxmsg, lines=0, forward, want_search = FALSE, show_flag, want_bin_search = FALSE;
   makefilename(fname,CONFPATH,confname,"CNF");
   maxmsg = i_maxmsg(confname);
   forward = msgini <= msgfin;

   if (search != NULL && *search != '\0') {
      strlwr (search);
      want_search = TRUE;
      if (*search == '*') {
         search++;
         want_bin_search = TRUE;
      }
   }

   if(forward) {
      if(msgfin <= 0 || msgfin > maxmsg) msgfin = maxmsg;
      if(msgini > maxmsg)                msgini = maxmsg;
      if(msgini <= 0)                    msgini = 1;
   }
   else {
      if(msgini <= 0 || msgini > maxmsg) msgini = maxmsg;
      if(msgfin > maxmsg)                msgfin = maxmsg;
      if(msgfin <= 0)                    msgfin = 1;
   }
   t_printf(number,"Se listan Mensajes del %d al %d %s%s\n",msgini,msgfin,
            want_search ? " - " : "", want_search ? search : "");
   t_printf(number,"Msg.# Usuario   Fecha     Hora    Tema\n");
   t_fopen(number,fname,READ_WAIT);
   while(forward ?  msgini <= msgfin : msgini >= msgfin) {
      offset = i_seekmsg(confname,msgini);
      if(offset >= 0L) {
         fseek(FP,offset,SEEK_SET);
         loadheader(FP,6,MSGDELIM,msg ,5,
                        "De:"    ,from,8,
                        "Fecha:" ,date,8,
                        "Hora:"  ,time,8,
                        "Tema:"  ,tema,40,
                        "Name:"  ,name,16);
         if (want_search) {
            strcpy (wtema, tema);
            strlwr (wtema);
            show_flag = (strstr (wtema, search) != NULL || stricmp (from, search) == 0);
            if (show_flag && want_bin_search && name[0] == '\0')
               show_flag = FALSE;
         }
         else
            show_flag = TRUE;

         if (show_flag) {
            t_printf(number,"%5s %-8s %-8s %-8s %c%s\n",msg,
                                                        from,
                                                        date,
                                                        time,
                                                        name[0] == '\0' ? ' ' : '*',
                                                        tema);
            lines++;
            if(lines > TERMROWS) {
               if(!more(number)) break;
               else lines = 0;
            }
         }
      }
      msgini = forward ? msgini+1 : msgini-1;
   }
   t_fclose(number);
}

void typeconf(int number, char *confname, int msg) {
   char fname[MAXPATH], line[81], file[13], name[17], size[17];   
   int  lines = 0, l, flag;
   long offset;
   offset = i_seekmsg(confname,msg);
   if(offset >=0) {
      makefilename(fname,CONFPATH,confname,"CNF");
      t_fopen(number,fname,READ_WAIT);
      fseek(FP,offset,SEEK_SET);
      l = strlen(MSGDELIM);
      flag = FALSE;
      while(f_gets(line, 80, FP) != nil(char)) {
         if(flag && equaln(line,MSGDELIM,l)) break;
         t_printf(number,"%s\n",line);
         flag = TRUE;
         lines++;
         if(lines > TERMROWS) {
            if(!more(number)) break;
            else lines = 0;
         }
      }
      if (binaryname (number, offset, file, name, size)) {
         t_fclose (number);
         t_printf (number, "Desea recibir el archivo '%s' de %s ? ",name, size);
         if (confirm (number,"")) {
            makefilename(fname,UPLOADPATH,file,"BIN");
            xm_transmite_file (number, fname, name);
         }
      }
      else {
         t_fclose(number);
      }
   }
   else t_printf(number,"Mensaje %d no accesible\n",msg);
}

void saveconf(int number, char *confname, int msg) {
   char fname[MAXPATH], fdest[MAXPATH], line[81];
   int  l, flag;
   long offset;
   offset = i_seekmsg(confname,msg);
   if(offset >=0) {
      makefilename(fname,CONFPATH,confname,"CNF");
      makefilename(fdest,SCRATCHPATH,tty[number].user,"WKR");
      FPAUX = fopen(fdest,"wb");
      t_fopen(number,fname,READ_WAIT);
      fseek(FP,offset,SEEK_SET);
      l = strlen(MSGDELIM);
      flag = FALSE;
      while(f_gets(line,80,FP) != nil(char)) {
         if(flag && equaln(line,MSGDELIM,l)) break;
         fprintf(FPAUX,"%s\r\n",line);
         flag = TRUE;
      }
      t_fclose(number);
      fclose(FPAUX);
   }
   else t_printf(number,"Mensaje %d no accesible\n",msg);
}

void sendconf(int number, char *from, char *to, char *subject, long xflag, char *name) {
   char fromfile[MAXPATH], tofile[MAXPATH], line[81], binaryname[14];
   int  l, msgno;
   makefilename(fromfile,SCRATCHPATH,tty[number].user,"WKR");
   makefilename(tofile  ,CONFPATH   ,to  ,"CNF");
   t_puts(number,"Aguarde por favor....\n");
   t_fopen(number,tofile,APPEND);
   fseek(FP,0L,SEEK_END);
   msgno = i_addmsg(to,ftell(FP));
   RQ;
   fprintf(FP,"%-7s%d\r\n",MSGDELIM,msgno);
   fprintf(FP,"%-7s%s\r\n","De:"   ,from);
   fprintf(FP,"%-7s%02d/%02d/%02d\r\n","Fecha:",st->tm_mday,(st->tm_mon+1),st->tm_year);
   fprintf(FP,"%-7s%02d:%02d:%02d\r\n","Hora:" ,st->tm_hour,st->tm_min,st->tm_sec);
   fprintf(FP,"%-7s%s\r\n","Tema:" ,subject);
   if (xflag) {
      sprintf (binaryname, "BIN%05d",seqmsg (UPLOADSEQ));
      fprintf(FP,"%-7s%s\r\n", "Name:" ,name);
      fprintf(FP,"%-7s%s\r\n", "File:" ,binaryname);
      fprintf(FP,"%-7s%ld bytes\r\n","Size:" ,xflag);
   }
   fprintf(FP,"\r\n");
   FPAUX = fopen(fromfile,"rb");
   RL;
   l     = strlen(MSGDELIM);
   while(f_gets(line,80,FPAUX) != nil(char)) {
      if(equaln(line,MSGDELIM,l)) fputc('>',FP);
      RQ; fprintf(FP,"%s\r\n",line); RL;
   }
   RQ;
   fclose(FPAUX);
   FPAUX = NULL;
   RL;
   t_fclose(number);
   if (xflag) {
      makefilename(fromfile,UPSCRATCHPATH,from,"UP");
      makefilename(tofile, UPLOADPATH,binaryname,"BIN");
      if (rename (fromfile,tofile) != 0) {
         t_puts (number, "ERROR: enviando archivo asociado\n");
      }
   }
   inc_lastmsg (to);
}

int checkconf(int number, char *confname, int *readonly, int *ismoderator) {
   char fname[MAXPATH];

   *readonly    = FALSE;
   *ismoderator = FALSE;
   makefilename(fname,CONFPATH,confname,"CNF");
   t_fopen(number,fname,READ);
   if(FP == NULL) {
      return(FALSE);
   }
   t_fclose(number);
   makefilename(fname,CONFPATH,confname,"RED");
   if(!is_in_list(number,fname,TRUE)) {
      return(FALSE);
   }
   makefilename(fname,CONFPATH,confname,"MOD");
   *ismoderator = is_in_list(number,fname,FALSE);
   if(!(*ismoderator)) {
      makefilename(fname,CONFPATH,confname,"WRT");
      *readonly = !is_in_list(number,fname,TRUE);
   }
   return(TRUE);
}

void cconf(int number, char *param) {
   char command[41], subject[41], scratch[MAXPATH], confname[9], 
        listfile[MAXPATH], file[13], name[17], size[17];
   char *search;
   int  inloop=TRUE, readonly, ismoderator, msg, msgini, msgfin, maxmsg, i;
   long starttime, xflag;

   maxmsg = 0;
   t_printf(number,"\nSistema de Conferencias CompuService\n");
   t_printf(number,"------------------------------------\n");

   makefilename(scratch,SCRATCHPATH,tty[number].user,"WKR");
   makefilename(listfile,CONFPATH,"conflist","msg");
   if(param != NULL && param[0] != '.') {
      strcpy(confname,param);
      inloop = !checkconf(number,confname,&readonly,&ismoderator);
   }
   while(inloop) {
      t_puts(number,"Conferencia : ");
      t_gets(number,confname,8);
      if(confname[0] == '\0') break;
      else if(confname[0] == '?') typefile(number,listfile);
      else if(equalni(confname,"telecon",7)) teleconf(number);
      else {
         inloop = !checkconf(number,confname,&readonly,&ismoderator);
      }
   }
   inloop = confname[0] != '\0';
   if(inloop) {
      if(readonly) {
         t_printf(number,"La conferencia '%s' es solo de lectura\n",confname);
      }
      if(ismoderator) {
         t_printf(number,"Ud. es MODERADOR en esta conferencia\n");
      }
      msgfin = i_maxmsg(confname);
      maxmsg = update_cnfinfo(number,tty[number].user,confname,0);
      if (maxmsg > msgfin) {
         t_printf(number,"Conferencia actualizada. Verifique ultimo mensaje leido\n");
         maxmsg = 0;
      }
      t_printf(number,"Mensajes :%5d\n",msgfin);
      t_printf(number,"Leidos   :%5d\n",maxmsg != -1 ? maxmsg : 0);
      t_printf(number,"Nuevos   :%5d\n",maxmsg != -1 ? msgfin - maxmsg : msgfin);
      starttime = _clock;
   }
   while(inloop) {
      t_fclose(number);
      t_printf(number,"%s>",confname);
      xflag = FALSE;
      t_gets(number,command,40);
      strlwr(command);
      checkconf(number,confname,&readonly,&ismoderator);
      msgfin = i_maxmsg(confname);
      switch(command[0]) {
         case 'f' :
            if (command[1] == 'a')
               maxmsg = i_maxmsg (confname);
            inloop = FALSE;
            break;
         case 'd' :
            if(confirm(number,"Confirma desconexion ? ")) {
               maxmsg = -1; /* se va a 'c' */
            }
            else break;
         case 'c' :
            if (command[1] == 'a')
               maxmsg = i_maxmsg (confname);
            update_cnfstat(number,confname,(int) (_clock-starttime)/60);
            starttime = _clock;
            update_cnfinfo(number,tty[number].user,confname,maxmsg);
            while(inloop) {
               if(command[1] == NULL || (command[1]=='a' && command[2] == NULL)) {
                  t_puts (number,"Conferencia : ");
                  t_gets (number,confname,8);
               }
               else {
                  strncpy(confname,command + (command[1]=='a' ? 3 : 2),8);
                  command[0] = command[1] = confname[8] = '\0';
               }
               if(confname[0] == '\0') break;
               else if(confname[0] == '?') typefile(number,listfile);
               else if(equalni(confname,"teleconf",8)) teleconf(number);
               else
                  inloop = !checkconf(number,confname,&readonly,&ismoderator);
            }
            inloop = confname[0] != '\0';
            if(inloop) {
               if(readonly) {
                  t_printf(number,"La conferencia '%s' es solo de lectura\n",confname);
               }
               if(ismoderator) {
                  t_printf(number,"Ud. es MODERADOR en esta conferencia\n");
               }
               msgfin = i_maxmsg(confname);
               maxmsg = update_cnfinfo(number,tty[number].user,confname,0);
               if (maxmsg > msgfin) {
                  t_printf(number,"Conferencia actualizada. Verifique ultimo mensaje leido\n");
                  maxmsg = 0;
               }
               t_printf(number,"Mensajes :%5d\n",msgfin);
               t_printf(number,"Leidos   :%5d\n",maxmsg != -1 ? maxmsg : 0);
               t_printf(number,"Nuevos   :%5d\n",maxmsg != -1 ? msgfin - maxmsg : msgfin);
            }
            break;
         case 't' :
            editor(number,EDIT,tty[number].user);
            break;
         case 'l' :
            search=NULL;
            /* ubicamos posible search string */
            for (i=1; command[i] != '\0'; i++) {
               if (command[i] == '\"') {
                  if (search == NULL) {
                     command[i] = '\0';
                     if (command[i-1] == ' ')
                        command[i-1] = '\0';
                     search = command + i + 1;
                  }
                  else {
                     command[i] = '\0';
                     break;
                  }
               }
            }
            if(strlen(command) > 1) {
               if(command[1] == 'l') {
                  msgfin = i_maxmsg(confname);
                  msgini = msgfin - atoi(command+2) + 1;
               }
               else if(command[1] == 'n') {
                  msgfin = i_maxmsg(confname);
                  msgini = maxmsg+1;
               }
               else {
                  RQ; sscanf(command,"%*c %d - %d",&msgini,&msgfin); RL;
               }
            }
            else {
               msgini = 1;
            }
            showconf(number,confname,msgini,msgfin, search);
            msgfin = i_maxmsg(confname);
            break;
         case 's' :
            if(strlen(command) > 1) {
               sscanf(command,"%*c %d",&msg);
               if (msg <= msgfin) {
                  t_printf (number,"Copiando mensaje '%d' a area de trabajo. . .\n",msg);
                  saveconf (number,confname,msg);
               }
            }
            else {
               t_puts(number,"Error. Uso : S <n> siendo n el numero de mensaje a salvar en area de trabajo\n");
            }
            break;
         case 'b' :
            if(ismoderator) {
               msg = atoi(command+1);
               if(msg==0) {
                  t_puts(number,"Msg: ");
                  t_gets(number,command,5);
                  msg = atoi(command);
               }
               if(msg>0 && msg <= msgfin) {
                  t_printf(number,"Confirma eliminacion del mensaje %d ? ",msg);
                  if(confirm(number,"")) {
                     xflag = i_modimsg(confname,msg,-1L);
                     t_puts(number,"Mensaje eliminado\n");
                     if (xflag != -1L) {
                        /* verificamos si tiene archivo asociado */
                        makefilename(scratch,CONFPATH,confname,"CNF");
                        t_fopen(number,scratch,READ);
                        if (binaryname (number, xflag, file, name, size)) {
                           t_fclose (number);
                           t_printf (number, "Eliminando '%s'\n",name);
                           makefilename(scratch,UPLOADPATH,file,"BIN");
                           safe_unlink (number,scratch);
                        }
                        else {
                           t_fclose (number);
                        }
                        makefilename(scratch,SCRATCHPATH,tty[number].user,"WKR");
                     }
                  }
               }
               else {
                  t_printf(number,"Mensaje %d fuera de rango\n",msg);
               }
            }
            else {
               t_puts(number,"Comando no autorizado\n");
            }
            break;
         case 'x' :
            if (tolower(command[1]) == 'e') xflag = TRUE;
            else break;
         case 'e' :
            if(!readonly) {
               strupr(tty[number].user);
               t_puts(number,"Tema     : ");
               t_gets(number,subject,40);
               if (xflag) {
                  t_puts(number,"Archivo  : ");
                  t_gets(number,name,16);
               }
               editor(number,TRANSMITE,tty[number].user);
               if (xflag) {
                  makefilename(scratch,UPSCRATCHPATH,tty[number].user,"UP");
                  xflag = xm_receive_file (number, scratch, name);
                  if (xflag == 0L) {
                     t_puts (number, "Error recibiendo archivo asociado\n");
                  }
                  makefilename(scratch,SCRATCHPATH,tty[number].user,"WKR");
               }
               if(confirm(number,"Confirma envio ? ")) {
                  sendconf(number,tty[number].user,confname,subject,xflag,name);
                  sprintf(command,"%-8s%c%-31s",confname,xflag ? '#' : ' ',subject);
#ifndef NOLOG
                  sendmail(number,tty[number].user,"LOG",command,0L,"");
#endif
               }
               if(confirm(number,"Elimina archivo de trabajo ? ")) {
                  RQ; unlink(scratch); RL;
                  t_puts(number,"Archivo eliminado\n");
               }
            }
            else {
               t_puts(number,"Comando no autorizado\n");
            }
            break;
         case 'n' :
            shownew(number,tty[number].user);
            break;
         case '?' :
            t_puts(number,"Comandos disponibles :\n");
            t_puts(number,"   ? - Ayuda\n");
           if(ismoderator) {
            t_puts(number,"   B - Borrar mensaje\n");
            t_puts(number,"   M - Minutos utilizados\n");
           }
            t_puts(number,"   C - Cambia conferencia\n");
           if(!readonly)
            t_puts(number,"   E - Enviar mensaje\n");
            t_puts(number,"  XE - Enviar mensaje con archivo binario asociado\n");
            t_puts(number,"   D - Desconectarse\n");
            t_puts(number,"   L - Lista  mensajes\n");
            t_puts(number,"  LL - Lista  £ltimos mensajes\n");
            t_puts(number,"  LN - Lista  £ltimos mensajes\n");
            t_puts(number,"  Lx \"texto_a_buscar\" permite realizar b£squedas\n");
            t_puts(number,"   N - Nuevos mensajes en el sistema\n");
            t_puts(number,"   P - Perfil de usuario\n");
            t_puts(number,"   S - Salva Msg. en area de trabajo\n");
            t_puts(number,"   T - Area de Trabajo\n");
            t_puts(number,"   n - Despliega Mensaje No. n\n");
            t_puts(number,"   + - Despliega proximo mensaje\n");
            t_puts(number,"   F - Finalizar\n\n");
            break;
         case 'p' :
            showperfil(number);
            break;
         case 'm' :
            if (ismoderator) {
               showstat(number,confname);
            }
            break;
         case 'q' :
         case 'w' :
            show_who(number);
            break;
         case '+' :
            if (maxmsg < msgfin) {
               msg = ++maxmsg;
               typeconf (number,confname,msg);
            }
            else {
               t_printf(number,"Ultimo mensaje = %d\n",msgfin);
            }
            break;
         default :
            msg = atoi(command);
            if(msg) {
               if(msg <= msgfin) {
                  maxmsg = max(maxmsg,msg);
                  typeconf(number,confname,msg);
               }
               else {
                  t_printf(number,"Ultimo mensaje = %d\n",msgfin);
               }
            }
      }
   }
   t_fclose(number);
   if(confname[0] != '\0') {
      update_cnfstat(number,confname,(int) (_clock-starttime)/60);
      update_cnfinfo(number,tty[number].user,confname,maxmsg);
   }
}
