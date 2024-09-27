/* menues.c
 * funciones de manejo de menues del CompuService Bulleting Board System
 * Modificaciones
 * 17:50 05/10/89   Version inicial (APR)
 * 13:18 31/05/90   Ademas del comando MENU xxx se acepta GO xxx
 */

#ifndef CBBSMAIN
#   include <ctype.h>
#   include "cbbs.h"
#endif

int dispmenu(int number,char *currentmenu,SMENU *menu) {
   char command[10];
   int option=0, i, loop, c;
   t_fopen(number,currentmenu,READ);
   if(FP == NULL) return(-1);
   loop = 1;

   while((c=fgetc(FP)) != EOF && c != '"') {
      if(NIVEL < 2) t_putchar(number,c);
   }
   while(loop) {
      i=0;
      while((c=fgetc(FP)) != EOF && c != '"') {
         if(NIVEL == 0) t_putchar(number,c);
         if(i==0) {
            menu[option].item = toupper(c);
            if(NIVEL == 1) t_putchar(number,c);
            i++;
         }
      }
      if(!NIVEL) t_putchar(number,'\n');
      while((c=fgetc(FP)) != EOF && (c==' ' || c=='\t')) pause();
      command[0]='\0';
      if(c != '\r') {
         /* SCANNING COMMAND */
         command[0]=c;
         for(i=1; (c=fgetc(FP)) != EOF && c!=' ' && c!='\t' && c!='\r' ; i++) {
	    command[i] = c;
	    pause();
         }
         if(c=='\r') fgetc(FP); /* skip LF */
         command[i]='\0';
         if     (equali(command,S_MENU))       menu[option].command = MENU;
         else if(equali(command,S_TYPE))       menu[option].command = TYPE;
         else if(equali(command,S_MAIL))       menu[option].command = MAIL;
         else if(equali(command,S_CONF))       menu[option].command = CONF;
         else if(equali(command,S_FIND))       menu[option].command = FIND;
         else if(equali(command,S_PERFIL))     menu[option].command = PERFIL;
         else if(equali(command,S_TELECONF))   menu[option].command = TELECONF;
         else if(equali(command,S_DELPHI))     menu[option].command = DELPHI;
         else if(equali(command,S_REUTACCESS)) menu[option].command = REUTACCESS;
         else if(equali(command,S_PED_MYSSA))  menu[option].command = PED_MYSSA;
         else if(equali(command,S_DEMODELPHI)) menu[option].command = DEMODELPHI;
         else if(equali(command,S_SHOWDISK))   menu[option].command = SHOWDISK;
         else if(equali(command,S_DOWNDISK))   menu[option].command = DOWNDISK;
         else if(equali(command,S_QUIT))       menu[option].command = QUIT;
         else menu[option].command = INVALID;

      while((c=fgetc(FP)) != EOF && (c==' ' || c=='\t')) pause();
         if(c != '\r') {
            /* SCANNING COMMAND PARAMETER */
            menu[option].param[0]=c;
            for(i=1; (c=fgetc(FP)) != EOF && c!='\r' ; i++) {
               menu[option].param[i] = c;
            }
            menu[option].param[i]='\0';
            if(c=='\r') fgetc(FP);
         }
         else fgetc(FP); /* skip LF */
      }
      else fgetc(FP); /* skip LF */
      c=fgetc(FP);
      if(c!= '"') loop=0;
      if(option < MAXMENUITEMS) option++;
      pause();
   }
   t_fclose(number);
   return(option);
}

int validfname(char *name) {
   int i=0;
   while(name[i] != NULL) {
      if(name[i]=='.'  ||
         name[i]==':'  ||
         name[i]=='\\' ||
         name[i]=='?'  ||
         name[i]=='/'  ||
         name[i]=='*'  ||
         name[i]==' '  ||
         !isprint(name[i])) {
         return(FALSE);
      }
   i++;
   if(i>8) return(FALSE);
   }
   if(i>0) return(TRUE);
   else    return(FALSE);
}

int menu(int number,char *currentmenu) {
   long starttime;
   SMENU menu[MAXMENUITEMS];
   int options, i;
   char input[81], work[81];
   for(;;) {
      options = dispmenu(number,currentmenu,menu);
      if(options == -1) {
         /* error recovery */
         strcpy(currentmenu,"INICIAL.MNU");
         options = dispmenu(number,currentmenu,menu);
      }
      switch(NIVEL) {
         case 0 :
            t_puts(number,"\nDigite opcion ! ");
            break;
         case 1 :
         case 2 :
            t_puts(number,"! ");
            break;
      }
      t_gets(number,input,80);
      if(equalni(input,"MENU",4) || equalni(input,"GO",2)) {
         RQ; sscanf(input,"%*s %s",work); RL;
         if(validfname(work)) {
            strcat(work,".MNU");
            strcpy(currentmenu,work);
         }
      }
#ifdef CS1
      else if(equalni(input,"CORREO"  ,6)) {
         typefile (number, "msg/encs0.msg");
      }
#else
      else if(equalni(input,"CORREO"  ,6)) cmail(number,(char *)NULL,tty[number].user);
#endif
      else if(equalni(input,"EDITOR"  ,6)) editor(number,EDIT,tty[number].user);
      else if(equalni(input,"CONF"    ,4)) cconf(number,(char *)NULL);
      else if(equalni(input,"CUENTA"  ,6)) showbill(number);
      else if(equalni(input,"CLAVE"   ,5)) chgpassword(number);
      else if(equalni(input,"ECO"     ,3)) tty[number].echo = ON;
      else if(equalni(input,"NOECO"   ,5)) tty[number].echo = OFF;
      else if(equalni(input,"MAS"     ,3)) tty[number].more = ON;
      else if(equalni(input,"NOMAS"   ,5)) tty[number].more = OFF;
      else if(equalni(input,"OCU"     ,3)) tty[number].ocu  = ON;
      else if(equalni(input,"NOOCU"   ,5)) tty[number].ocu  = OFF;
      else if(equalni(input,"HORA"    ,4)) datetime(number,FALSE);
      else if(equalni(input,"NUEVO"   ,5)) shownew(number,tty[number].user);
      else if(equalni(input,"PERFIL"  ,6)) showperfil(number);
      else if(equalni(input,"MIPERF"  ,6)) loadperfil(number,tty[number].user);
      else if(equalni(input,"QUIENES" ,7)) show_who(number);
      else if(equalni(input,"WHO"     ,3)) show_who(number);
      else if(equalni(input,"TELECONF",8)) teleconf(number);
      else if(equalni(input,"SYSTEM",6)) {
              if(tty[number].issysop)  c_os(number);
      }
      else if(equalni(input,"?",1)) {
         switch(NIVEL) {
            case 0 :
               typefile(number,"menues.hlp");
               espacio(number);
               break;
            case 1 :
               NIVEL = 0;
               dispmenu(number,currentmenu,menu);
               NIVEL = 1;
               break;
            case 2 :
               NIVEL = 1;
               dispmenu(number,currentmenu,menu);
               NIVEL = 2;
               break;
         }
      }
      else if(equalni(input,"NIVEL" ,5)) {
         i=atoi(input+5);
         if(i>=0 && i<=2) {
            tty[number].nivel = i;
            t_printf(number,"Nivel de ayuda = %d\n",i);
         }
         else {
            t_printf(number,"Niveles validos 0, 1 o 2\n");
         }
      }
      else {
         input[0] = toupper(input[0]);
         for(i=0; i<options; i++) {
            if(menu[i].item == input[0]) break;
         }
         if(i<options) {
            switch(menu[i].command) {
               case MENU :
                  if(menu[i].param[0] != NULL) {
                     strcpy(currentmenu,menu[i].param);
                  }
                  break;
               case TYPE :
                  if(menu[i].param[0] != NULL) {
                     typefile(number,menu[i].param);
                  }
                  break;
               case FIND :
                  if(menu[i].param[0] != NULL) {
                     cfind(number,menu[i].param);
                  }
                  break;
               case MAIL :
                  cmail(number,menu[i].param,tty[number].user);
                  break;
               case CONF :
                  cconf(number,menu[i].param);
                  break;
               case PERFIL :
                  showperfil(number);
                  break;
               case TELECONF :
                  teleconf(number);
                  break;
               case DEMODELPHI :
                  writelog(number,"/cbbs/delphi.log","Req.conn");
                  demodelphi(number);
                  writelog(number,"/cbbs/delphi.log","End.conn");
                  break;
               case PED_MYSSA :
                  ped_myssa (number);
                  break;
               case SHOWDISK :
                  showdisk (number, menu[i].param);
                  update_cnfstat (number, "showdisk", 1);
                  break;
               case DOWNDISK :
                  starttime = _clock;
                  downdisk (number, menu[i].param);
                  update_cnfstat (number, "downdisk",(int) (_clock-starttime)/60);
                  update_cnfstat (number, "downcnt", 1);
                  break;
               case REUTACCESS :
#if 0
                  writelog(number,"/cbbs/reuters.log","Req.conn");
                  reutaccess(number,menu[i].param,currentmenu);
                  writelog(number,"/cbbs/reuters.log","End.conn");
#endif
                  break;
               case DELPHI :
                  writelog(number,"/cbbs/outdial.log","Req.conn");
                  delphi(number);
                  writelog(number,"/cbbs/outdial.log","End.conn");
                  break;
               case QUIT :
                  t_printf(number,"Tiempo de conexion : %d minuto(s)\n",(_clock+59-tty[number].con_time)/60);
                  t_puts(number,"Gracias por su preferencia\n\n\n");
                  t_disconnect(number);
                  t_delay(0L);
                  break;
            }
         }
         else if(!NIVEL) {
            t_printf(number,"\nOpcion no valida. Digite '?' si necesita ayuda\n");
         }
      }
   }
}
