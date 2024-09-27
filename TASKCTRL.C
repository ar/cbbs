/* taskctrl.c
 * creacion/matanza de tareas
 * Modificaciones
 * 17:50 05/10/89   Version inicial (APR)
 * 18:38 23/03/90   Se modifica para que compile solo
 */

#ifndef CBBSMAIN
#   include "cbbs.h"
#endif

void create(int number) {
   switch(number) {
      case 0 :
         create_task(&user0_tcb,usertask,stkuser0,STACKSIZE,PRI_STD,(farptr) 0L, (byteptr) "USER0   ");
         break;
      case 1 :
         create_task(&user1_tcb,usertask,stkuser1,STACKSIZE,PRI_STD,(farptr) 1L, (byteptr) "USER1   ");
         break;
      case 2 :
         create_task(&user2_tcb,usertask,stkuser2,STACKSIZE,PRI_STD,(farptr) 2L, (byteptr) "USER2   ");
         break;
      case 3 :
         create_task(&user3_tcb,usertask,stkuser3,STACKSIZE,PRI_STD,(farptr) 3L, (byteptr) "USER3   ");
         break;
      case 4 :
         create_task(&user4_tcb,usertask,stkuser4,STACKSIZE,PRI_STD,(farptr) 4L, (byteptr) "USER4   ");
         break;
      case 5 :
         create_task(&user5_tcb,usertask,stkuser5,STACKSIZE,PRI_STD,(farptr) 5L, (byteptr) "USER5   ");
         break;
      case 6 :
         create_task(&user6_tcb,usertask,stkuser6,STACKSIZE,PRI_STD,(farptr) 6L, (byteptr) "USER6   ");
         break;
   }
   pause();
}

void start(int number) {
   switch(number) {
      case 0 :
         start_task(&user0_tcb);
         break;
      case 1 :
         start_task(&user1_tcb);
         break;
      case 2 :
         start_task(&user2_tcb);
         break;
      case 3 :
         start_task(&user3_tcb);
         break;
      case 4 :
         start_task(&user4_tcb);
         break;
      case 5 :
         start_task(&user5_tcb);
         break;
      case 6 :
         start_task(&user6_tcb);
         break;
   }
   pause();
}

void kill(int number) {
   t_fclose(number);
   if(FPAUX != NULL) {
      RQ;
      fclose(FPAUX);
      FPAUX = NULL;
      RL;
   }
   switch(number) {
      case 0 :
         RQ; kill_task(&user0_tcb); RL;
         /* while(user0_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
      case 1 :
         RQ; kill_task(&user1_tcb); RL;
         /* while(user1_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
      case 2 :
         RQ; kill_task(&user2_tcb); RL;
         /* while(user2_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
      case 3 :
         RQ; kill_task(&user3_tcb); RL;
         /* while(user3_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
      case 4 :
         RQ; kill_task(&user4_tcb); RL;
         /* while(user4_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
      case 5 :
         RQ; kill_task(&user5_tcb); RL;
         /* while(user5_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
      case 6 :
         RQ; kill_task(&user6_tcb); RL;
         /* while(user6_tcb.timerq.tstate != TSTAT_IDLE) t_delay(18L); */
         break;
   }
}
