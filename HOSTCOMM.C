/*
   hostcomm.c
   Provee comunicaci¢n con el host
*/

#define SOH 0x01
#define EOT 0x04
#define DLE 0x10

#define MSG_TELECONF    '0' /* msg entre SOH...EOT                       */
#define MSG_LOGINREQ    '1' /* SOH + CHANNEL + USERNAME + PASSWORD + EOT */
#define MSG_LOGINOK     '2' /* SOH + CHANNEL + [1 o 2] + EOT             */
#define MSG_LOGIN       '3' /* SOH + CHANNEL + USERNAME + EOT            */
#define MSG_LOGOFF      '4' /* SOH + CHANNEL + EOT                       */
#define MSG_IN_TELECONF '5' /* SOH + CHANNEL + EOT                       */
#define MSG_NO_TELECONF '6' /* SOH + CHANNEL + EOT                       */
#define MSG_CALL        '7' /* SOH + MYUSERNAME + CALLEDUSERNAME + EOT   */

void far hostcomm (void) {
   char buffer[129], username[9], password[9], fromuser[9];
   char msg_type;
   register int i;
   int l;
   for (;;) {
      while (v24_receive (host_siop, 0L) != SOH) ;
      msg_type = v24_receive (host_siop, 0L);
      switch (msg_type) {
         case MSG_TELECONF :  v24_getuntil (host_siop, buffer, 128, EOT);
                              RQ;
                              strcpy (tconf[tconf_tail], buffer);
                              tconf_tail++;
                              tconf_tail%=MAXTELECONFLINES;
                              RL;
                              break;
         case MSG_LOGINREQ :  i = v24_receive (host_siop, 0L) - '0' + MAXUSERS;
                              if (i>=MAXUSERS && i<10) {
                                 v24_getuntil (host_siop, buffer, 30, EOT);
                                 strncpy (username, buffer, 8);
                                 strtrim (username, 8);
                                 strncpy (password, buffer+8, 8);
                                 strtrim (password, 8);

                                 password[8] = '\0';
                                 /* tsk_rprintf ("Login Request: '%s' '%s'\r\n", username, password); */
                                 if (already_in (username, &l)) {
                                    request_resource (&hostcommsrc, 0L);
                                    v24_printf(host_siop, "%c%c%c%c%c", SOH, MSG_LOGINOK, i + '0' - MAXUSERS, '2', EOT);
                                    release_resource (&hostcommsrc);
                                 }
                                 else if (validuser(i, username, password) != NULL) {
                                    /* tty[i].user[0] = '\0'; */
                                    request_resource (&hostcommsrc, 0L);
                                    v24_printf(host_siop, "%c%c%c%c%c", SOH, MSG_LOGINOK, i + '0' - MAXUSERS, '1', EOT);
                                    release_resource (&hostcommsrc);
                                 }
                                 else {
                                    request_resource (&hostcommsrc, 0L);
                                    v24_printf(host_siop, "%c%c%c%c%c", SOH, MSG_LOGINOK, i + '0' - MAXUSERS, '0', EOT);
                                    release_resource (&hostcommsrc);
                                 }
                              }
                              if (i >= 0 && i < MAXUSERS)
                                 tty[i].nivel = v24_receive (host_siop, 0L);
                              break;
         case MSG_LOGIN    :  i = v24_receive (host_siop, 0L) - '0' + MAXUSERS;
                              v24_getuntil (host_siop, buffer, 8, EOT);
                              if (i>=MAXUSERS && i<10) {
                                 strtrim (buffer,8);
                                 strcpy (tty[i].user, buffer);
                                 tty[i].tconf = TRUE;
                              }
                              break;
         case MSG_LOGOFF   :  i = v24_receive (host_siop, 0L) - '0' + MAXUSERS;
                              if (i>=MAXUSERS && i<10) {
                                 tty[i].user[0] = '\0';
                                 tty[i].tconf   = 0;
                              }
                              break;
         case MSG_CALL     :  
                              v24_getuntil (host_siop, buffer, 30, EOT);
                              strncpy (fromuser, buffer, 8);
                              strtrim (fromuser, 8);
                              strncpy (username, buffer+8, 8);
                              strtrim (username, 8);
                              if (already_in (username, &l)) {
                                 if (!tty[l].ocu)
                                    t_printf(l,
                                    "\n\n========>El usuario %s lo espera en TELECONFerencias (canal %d)\n\n",
                                    fromuser, 1);
                              }
                              break;

      }
   }
}
