/* cmail.c
 * CompuService Mail Program
 * Modificaciones
 * 20:58 06/10/89   Version inicial (APR)
 * 16:12 20/02/90   Se agrega comando READMAIL
 *
 */

#ifndef CBBSMAIN
#   include <dir.h>
#   include "cbbs.h"
#endif

#include <string.h>
#include <stdio.h>

char *frontToken (char *in, char *out, int len) {
   char *s;
   s = out;
   while (*in != '\0' && *in != ',' && *in != ' ' && --len)
      *(s++) = *(in++);
   *s = '\0';
   return (out);
}

char *restToken (char *s) {
   char *p;
   p = s;
   while (*s != '\0' && *s != ',' && *s != ' ')
      s++;
   while ((*s == ' ' || *s == ',') && *s != '\0')
      s++;

   if (*s == '\0')
      return (NULL);
   else
      return (strcpy (p, s));
}

int binaryname (int number, long offset, char *file, char *name, char *size) {
   file[0] = '\0';
   fseek (FP,offset,SEEK_SET);
   loadheader (FP, 3, "File:", file, 12, 
                      "Name:", name, 16, 
                      "Size:", size, 16);
   return (file[0] != '\0');
}

void checkformail(int number) {
   char mailbox[MAXPATH];
   makefilename(mailbox,MAILPATH,tty[number].user,"MAI");
   if(existfile(number,mailbox)) {
      t_puts(number,"Verifique su correo\n");
   }
   pause();
}

int seqmsg(char *filename) {
   unsigned i;
   FILE *fp;

   RQ;
   fp = fopen(filename,"r");
   if(fp == NULL) i = 1;
   else {
      fscanf(fp,"%d",&i);
   }
   fclose(fp);
   fp = fopen(filename,"w");
   fprintf(fp,"%d\n",i+1);
   fclose(fp);
   RL;
   pause();
   return(i);
}

void sendmail(int number, char *from, char *to, char *subject, long xflag, char *name) {
   char fromfile[MAXPATH], tofile[MAXPATH], binaryname[14], line[81];
   int  l, msgno;
   makefilename(fromfile,SCRATCHPATH,from,"WKR");
   makefilename(tofile  ,MAILPATH   ,to  ,"SEQ");
   msgno = seqmsg(tofile);
   makefilename(tofile  ,MAILPATH   ,to  ,"MAI");
   t_fclose(number);
   t_fopen(number,tofile,APPEND);
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
   l     = strlen(MSGDELIM);
   RL;
   while(f_gets(line,80,FPAUX) != nil(char)) {
      if(equaln(line,MSGDELIM,l)) fputc('>',FP);
      RQ; fprintf(FP,"%s\r\n",line); RL;
      pause();
   }
   RQ;
   fclose(FPAUX);
   FPAUX = NULL;
   RL;
   t_fclose(number);
   if (xflag) {
      makefilename(fromfile,UPSCRATCHPATH,from,"UP");
      makefilename(tofile,  UPLOADPATH,binaryname,"BIN");
      if (rename (fromfile,tofile) != 0) {
         t_puts (number, "ERROR: enviando archivo asociado\n");
      }
   }
   if(already_in(to,&l)) {
      if(tty[l].ocu) {
         t_printf(number,"Usuario '%s' ocupado. No se puede enviar aviso en linea.\n",to);
      }
      else {
         t_printf(number,"Se envia aviso en linea.\n");
         preempt_off();
         t_printf(l,"\n\n\nMensaje %d de '%s' recien recibido\nTema: %s\nVerifique su mailbox\n\n\n",msgno,from,subject);
#ifndef COOPERATIVE
	 preempt_on();
#endif
      }
   }
}

int showmails(int number, MSGPTR *msgp) {
   int  counter=0;
   char msg[6], from[9], date[9], time[9], tema[41], name[17];

   t_printf(number," Msg.# Usuario   Fecha     Hora    Tema\n");
   while(nextmsg(FP,MSGDELIM)) {
      if(counter < MAXMSGS) {
         msgp[counter].offset = ftell(FP);
         loadheader(FP,6,MSGDELIM,msg ,5,
                        "De:"    ,from,8,
                        "Fecha:" ,date,8,
                        "Hora:"  ,time,8,
                        "Tema:"  ,tema,40,
                        "Name:"  ,name,16);

         if(counter < MAXMSGS) msgp[counter].msg = atoi(msg);
         t_printf(number,"%c%5s %-8s %-8s %-8s %c%s\n",
                  msgp[counter].deleted ? '*' : ' ',
                  msg,from,date,time,
                  name[0] == '\0' ? ' ' : '*',
                  tema);
         counter++;
      }
      else {
         t_printf(number,"Supera cantidad maxima de mails\n");
         break;
      }
   }
   return(counter);
}

void cmail(int number, char *param, char *username) {
   long starttime, xflag;
   char mailbox[MAXPATH], mailbag[MAXPATH], scratch[MAXPATH], 
        command[81], user[9], subject[41], file[13], name[17], size[17];
   int  mails=0, inloop=TRUE, i, m, haymails=FALSE, is_mailer;
   MSGPTR msgp[MAXMSGS];

   starttime = _clock;
   *param = '\0'; /* not used yet. avoid Warning */
   t_printf(number,"\nCorreo Electronico CompuService\n");
   t_printf(number,"-------------------------------\n");
   for(i=0; i<MAXMSGS; i++) msgp[i].deleted = FALSE;
   makefilename(mailbox,MAILPATH,username,"MAI");
   makefilename(scratch,SCRATCHPATH,username,"WKR");
   t_fopen(number,mailbox,READ);
   if(FP != NULL) {
      mails = showmails(number,msgp);
      t_printf(number,"\nExisten %d mensajes en su archivo\n",mails);
      t_fclose(number);
   }
   else {
      t_printf(number,"No existen mensajes en su archivo\n");
   }
   while(inloop) {
      t_puts(number,"Correo>");
      t_gets(number,command,70);
      xflag = FALSE;
      switch(tolower(command[0])) {
         case 'z' :
            for(i=0; i<mails; i++) {
               msgp[i].deleted = TRUE;
            }
            t_puts(number,"Mensajes eliminados\n");
            break;
         case 'f' :
            inloop = FALSE;
            break;
         case 'x' :
            if (tolower(command[1]) == 'r') {
               xm_transmite_file(number,mailbox,mailbox);
               break;
            }
            else if (tolower (command[1]) == 'e') {
               if(command[2] == NULL) {
                  t_puts(number,"Enviar a : ");
                  t_gets(number,command,40);
                  strncpy(user,command,8);
                  user[8] = '\0';
               }
               else {
                  strncpy(user,command+3,8);
                  user[8] = '\0';
                  t_printf(number,"Enviar a '%s'\n",user);
               }
               xflag = TRUE;
               /* nos vamos sin break a la opcion 'E' */
            }
            else {
               break;
            }
         case 'e' :
            if (!xflag) {
               if(command[1] == NULL) {
                  t_puts(number,"Enviar a : ");
                  t_gets(number,command,70);
                  frontToken (command, user, 9);
                  user[8] = '\0';
               }
               else {
                  strcpy (command, command+2);
                  frontToken (command, user, 9);
                  t_printf(number,"Enviar a '%s'\n",command);
               }
            }
            strupr(user);
            is_mailer = equalni (user, "MAILER", 6);
            if (is_mailer && xflag)
               t_puts(number, "ERROR: MAILER no acepta archivos asociados\n");
            else if(is_mailer || isuser(number,user)) {
               if (is_mailer) {
                  t_puts(number,"Destino  : ");
                  strcpy (command, user);
               }
               else {
                  t_puts(number,"Tema     : ");
               }
               t_gets(number,subject,40);
               if (xflag) {
                  t_puts(number,"Archivo  : ");
                  t_gets(number,name,16);
               }
               editor(number,TRANSMITE,username);
               if (xflag) {
                  makefilename(scratch,UPSCRATCHPATH,username,"UP");
                  xflag = xm_receive_file (number, scratch, name);
                  if (xflag == 0L) {
                     t_puts (number, "Error recibiendo archivo asociado\n");
                  }
                  makefilename(scratch,SCRATCHPATH,username,"WKR");
                  strcpy (command, user); /* no soporta mails multiples */
               }
               if(confirm(number,"Confirma envio ? ")) {
                  sendmail(number,username,user,subject,xflag,name);
                  while (restToken (command)) {
                     frontToken (command, user, 9);
                     if(isuser(number,user)) {
                        t_printf (number, "Enviando copia a usuario '%s'\n", user);
                        sendmail(number,username,user,subject,0L, "");
                     }
                     else {
                        t_printf (number, "Usuario '%s' no existente. Copia no enviada.\n", user);
                     }
                  }
                  sprintf(command,"%-8s%c%-31s",user,xflag ? '#' : ' ',subject);
#ifndef NOLOG
                  sendmail(number,username,"LOG",command,0L,"");
#endif
               }
               if(confirm(number,"Elimina archivo de trabajo ? ")) {
                  safe_unlink(number,scratch);
                  t_puts(number,"Archivo eliminado\n");
               }
            }
            else if (!is_mailer) {
               t_printf(number,"Usuario '%s' no registrado en CompuService\n",user);
            }
            break;
         case 'b' :
            m = atoi(command+1);
            if(m==0) {
               t_puts(number,"Msg: ");
               t_gets(number,command,5);
               m = atoi(command);
            }
            if(m>0) {
               if(mails > 0) {
                  for(i=0; i<mails; i++) {
                     if(msgp[i].msg == m) {
                        msgp[i].deleted = TRUE;
                        break;
                     }
                  }
                  if(i == mails) {
                     t_puts(number,"Numero(s) valido(s) :");
                     for(i=0; i<mails; i++) {
                        t_printf(number," %d",msgp[i].msg);
                     }
                     t_putchar(number,'\n');
                  }
               }
               else {
                  t_puts(number,"No existen mensajes en su archivo\n");
               }
            }
            break;
         case 't' :
            editor(number,EDIT,username);
            break;
         case 'r' :
            typefile(number,mailbox);
            break;
         case '?' :
            typefile(number,"cmail.hlp");
            break;
         case 'q' :
         case 'w' :
            show_who(number);
            break;
         case 'l' :
            t_fopen(number,mailbox,READ);
            mails = showmails(number,msgp);
            t_fclose(number);
            break;
         case 'p' :
            showperfil(number);
            break;

         default :
            m = atoi(command);
            if(m>0) {
               if(mails > 0) {
                  t_fopen(number,mailbox,READ);
                  for(i=0; i<mails; i++) {
                     if(msgp[i].msg == m) {
                        typemsg(number,msgp[i].offset,(i+1) == mails ? -1L : msgp[i+1].offset);
                        if (binaryname (number, msgp[i].offset, file, name, size)) {
                           t_printf (number, "Desea recibir el archivo '%s' de %s ? ",name, size);
                           if (confirm (number,"")) {
                              makefilename(scratch,UPLOADPATH,file,"BIN");
                              t_fclose(number);
                              xm_transmite_file (number, scratch, name);
                              makefilename(scratch,SCRATCHPATH,username,"WKR");
                           }
                        }
                        msgp[i].deleted = confirm(number,"\nElimina mensaje ? ");
                        break;
                     }
                  }
                  t_fclose(number);
                  if(i == mails) {
                     t_puts(number,"Numero(s) valido(s) :");
                     for(i=0; i<mails; i++) {
                        t_printf(number," %d",msgp[i].msg);
                     }
                     t_putchar(number,'\n');
                  }
               }
               else {
                  t_puts(number,"No existen mensajes en su archivo\n");
               }
            }
      }
   }
   for(i=0; i<mails; i++) {
      if(msgp[i].deleted) { /* hay al menos 1 deleteado */
         t_puts(number,"Aguarde...actualizando archivo\n");
         makefilename(mailbag,MAILPATH,username,"TMP");
         t_fopen(number,mailbox,READ);
         RQ;
         FPAUX = fopen(mailbag,"wb");
         for(i=0; i<mails; i++) {
            if(msgp[i].deleted) {
               if (binaryname (number, msgp[i].offset, file, name, size)) {
                  t_printf (number, "Eliminando '%s'\n",name);
                  makefilename(scratch,UPLOADPATH,file,"BIN");
                  unlink (scratch);
                  makefilename(scratch,SCRATCHPATH,username,"WKR");
               }
            }
            else {
               haymails = TRUE;
               copymsg(msgp[i].offset,
                       (i+1) == mails ? -1L : msgp[i+1].offset,
                       FP,
                       FPAUX);
            }
         }
         RL;
         t_fclose(number);
         t_fopen(number,mailbox,WRITE); /* para que nadie la use */
         if(FP != NULL) {
            RQ;
            fclose(FP);
            FP = NULL;
            HANDLE = 0;
            fclose(FPAUX);
            FPAUX = NULL;
            unlink(mailbox);
            if(haymails) {
               if(rename(mailbag,mailbox) != 0) {
                  t_printf(number,"Error actualizando mailbox\n");
               }
               else {
                  unlink(mailbag);
               }
            }
            RL;
         }
         i=mails;
      }
   }
   t_fclose(number);
   update_cnfstat(number,"correo",(int) (_clock-starttime)/60);
}
