/* CBBS.H

   17:45 19/09/89   APR   Primer release

   Version Cambio
   --------------
   1.00A   - Primer release funcionante (se cuelga cada 2 o 3 dias)

   1.00B   - Se pusieron RQ y RL (Request y Release) en todas las funciones
             supuestamente no reentrantes (scanf, open, close)
           - Se cambio en t_disconne que en el caso de modem no pusiera
             el REQ_DISCONNE.
           - Se agrego el comando LOG (showlog) en C_OS
           - Se agregan perfiles (ver 'perfil.c' y cambios en 'menues.c'
           - Se cambio en 'editor.c' para que utilizara FP standard
           - Cambio en login.c, se borra newuser.wkr antes de enviar mail a LOG
           - Se modifico la inicializacion del modem
           Nota.- Se sigue colgando

   1.00C   - Se incorporo Watch_modem para ver si se elimina la colgada.
             Como suponemos que se trata de un problema del modem CIGNET
             al bajarle el DTR, colgamos el modem con +++ AT H0

   1.00D   - Comienza a operar con 2 lineas telefonicas (12/01/90)
             Fixed Bug en t_init no ponia tty[xx].user en NULL
             Se agrega comando WHO y QUIENES (identicos)

   1.00E   - Se compila con TC 2.0, se agrega soporte para COM3 (03E8 irq 2)
           - Comienza operacion con 4 usuarios (3 lineas + consola)
           - La variable 'clock' se cambio por '_clock' para no interferir
             con la funcion clock() del TC 2.0
             Los programas tocados fueron : cbbs, c_os, login, terminal
           - Esta version parece que no se cuelga (al menos en una semana)!!

   1.00F   - Nueva opcion de teleconferencias. COMANDO TELECONF
           - Se proteje en CMAIL counter > MAXMSGS (error en LOG)
           - Se instala COM4 en IRQ 7. MAXUSERS=6 !
           - Nueva opcion en CMAIL, comando 'r' recibe todos los mails
           - Se instalan comandos MAS y NOMAS
           - Se patean para disco el help del CMAIL (cmail.hlp)
           - Se saco la mostrada de hora en la consola, no tiene sentido
           - IMPORTANTE: Se puso para probar eliminar las eventuales
                         colagadas un timeout en RQ. OJO VERIFICAR
           - El usuario conectado a tty[0] no genera REQ_DISCONNE cada minuto
           - en C_OS se agrego comando LOGIN para cambiar de codigo de usuario
           - en C_OS se agrego comando WHO
           - se implementa el comando MONITOR
           - A pedido de los usuarios se elimino el NUEVO automatico en el login

   1.00G   - En lugar de incluirse todos los programas, se compilan
             por separado. Se crea cbbs.prj y se compila desde  TC
             en lugar de TCC.
             Es posible compilar desde TCC con MK.BAT que define
             ALL_IN_ONE y es como antes.
             Ahora los stkuser estan en el main stack. No logre
             poner el stkwatch en el main (se cuelga de pique).

             Se soluciona el problema de los negativos en conferencias,
             se pone aviso de tiempo de conexion al salir por la
             opcion 8.
             Se implementan comandos OCU y NOOCU con los correspondientes
             cambios en CMAIL y TELECONF.

           - Se agrego soporte para PACKET

             Nota.- Esta version no se llego a instalar en CompuService

   1.01A   - El sistema se cuelga cada tanto. No se ha podido determinar
             si se trata de un problema de software o de hardware.
             Provisoriamente pasamos a 'cooperative' multitasking en
             lugar de preemptive. Se eliminan por consiguiente los
             RQ y RL. (Se dejan pero se definen en NULL)

   1.01B   - Con 'cooperative' multitasking NO SE CUELGA !
             Instalamos un 5to modem en direccion 03F0 IRQ-6 (Direcciones
             correspondientes a la disketera (que se le saco)

   1.01C   - Se implementa comando OUTDIAL para el Sysop, y acceso a Delphi.
             NEWUSERS ahora envian mail a Sysop. Se le informan tarifas.

   1.01D   - Se implementa Xmodem en editor, c_os, cmail y posibilidad de
             copiar mensajes de conferencias hacia scratchpad.

   1.01E   - Se implementa upload/download de binarios asociados a mails
             y conferencias. Incrementamos el STACKSIZE a 8192 y ponemos
             stklen en 0xFFFF;
----------------------------------------------------------------------------
   2.00A   - Nos pasamos a CTask v 2.0. El cambio mas importante es en
             taskctrl.c, funcion kill, no esperamos mas por TSTAT_IDLE ya
             que se cuelga. (Aparentemente la nueva version no utiliza mas
             esto). Trataremos de volver poco a poco a 'preemptive 
             multitasking' (Sin perder esta 'maravilla' de que no se cuelga).
             En C_OS agregamos el comando 'mv' (move).
----------------------------------------------------------------------------
   2.10A   - Pasamos a disco de 143 Mega. Pretendemos operar as¡ :
             C: mails, conferencias y perfiles, workfiles
             D: binaries
             E: bases de datos
             F: Work para processos batch
             G: DOS, CBBS, mensajes, menues, flash
           - Por ahora operamos as¡:
             C: todo menos binaries
             D: binaries
             G: DOS, CBBS, etc.
   2.10B   - Sacamos 'hostcomm.c', agregamos 'ips.c' en 'hostcomm.c' se
             describe como volver a poner hostcomm. Incluimos por primera
             vez c¢digo C++ (para el manejo de regular expressions)
   2.11A   - Se instala NODO CS1 con CD-ROM. Se incluyen opciones para
             manejar el CD-ROM Player. Compilar con -DCS1 para nodo 1
             Se incluye un campo 'doing (char[31]) en struct STTY.
             Al transmitir y recibir con XModem se actualizan esos campos
             para que el 'who' del Sysop informe que cuernos est  haciendo
             el usuario.
             
----------------------------------------------------------------------------
*/

#include <time.h>
#include <stdio.h>
#include <tsk.h>
#include <sio.h>

#define VERSION     "2.14A"
#ifdef AT_CS
/* Compilaci¢n de prueba para AT en CS */
#define MAXUSERS         2

#define M1PORT    0x02F8   /* Modem Interno */
#define M1IRQ     3
#define M1VECT    M1IRQ+8

#define H1PORT    0x03F8   /* Com1 simulamos IPS */
#define H1IRQ     4
#define H1VECT    H1IRQ+8

#elif defined (TELNET)
#define MAXUSERS         4

#define M1PORT    0x03F8   /* Modem Interno */
#define M1IRQ     4
#define M1VECT    M1IRQ+8

#define M2PORT    0x02F8   /* Modem Interno */
#define M2IRQ     3
#define M2VECT    M2IRQ+8

#define M3PORT    0x03E8   /* Modem Interno */
#define M3IRQ     2
#define M3VECT    M3IRQ+8

#elif defined (CS1)
#define MAXUSERS         6

#define M1PORT    0x03F8   /* Puerta standard COM1 */
#define M1IRQ     4
#define M1VECT    M1IRQ+8

#define M2PORT    0x02F8   /* Puerta standard COM2 */
#define M2IRQ     3
#define M2VECT    M2IRQ+8

#define M3PORT    0x03E8   /* Modem Interno */
#define M3IRQ     5
#define M3VECT    M3IRQ+8

#define M4PORT    0x0368   /* Modem Interno */
#define M4IRQ     2
#define M4VECT    M4IRQ+8

#define M5PORT    0x0370   /* Modem Interno */
#define M5IRQ     7
#define M5VECT    M5IRQ+8

#if 0
#define M6PORT    0x02E8   /* Modem Interno marca ZOOM 2400 */
#define M6IRQ     10
#define M6VECT    M6IRQ+0x68
#endif

#else
/* Puertas sistema de producci¢n Red CS */
#define MAXUSERS         7

#define M1PORT    0x03F8   /* Modem Interno */
#define M1IRQ     4
#define M1VECT    M1IRQ+8

#define M2PORT    0x02F8   /* Modem Interno */
#define M2IRQ     3
#define M2VECT    M2IRQ+8

#define M3PORT    0x03E8   /* Modem Interno */
#define M3IRQ     7
#define M3VECT    M3IRQ+8

#define M4PORT    0x02F0   /* 1¦ puerta placa de 4 */
#define M4IRQ     10
#define M4VECT    M4IRQ+0x68

#define M5PORT    0x02E8   /* 2¦ puerta placa de 4 */
#define M5IRQ     11
#define M5VECT    M5IRQ+0x68

#define M6PORT    0x02E0   /* 3¦ puerta placa de 4 */
#define M6IRQ     12
#define M6VECT    M6IRQ+0x68

#define H1PORT    0x0260   /* 4¦ puerta placa de 4 */
#define H1IRQ     15
#define H1VECT    H1IRQ+0x68
#endif
#define MAXCONFS       100  /* hay aprox al 30/01/91 */
#define COOPERATIVE
/* #define SNAPSHOT */
#define ALLOWOUTDIAL
#ifdef TELNET
#define NOLOG /* No loggea mensajes */
#endif

#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

#define TRUE            ((int)1)
#define FALSE           ((int)0)
#define ON              ((int)1)
#define OFF             ((int)0)
#define equal(a,b)      (!strcmp(a,b))
#define equaln(a,b,n)   (!strncmp(a,b,n))
#define equali(a,b)     (!stricmp(a,b))
#define equalni(a,b,n)  (!strnicmp(a,b,n))
#define nil(type)       ((type *)NULL)
#define echooff(n)      (tty[(n)].echo=FALSE)
#define echoon(n)       (tty[(n)].echo=TRUE)
#define newline         t_putchar(number,'\n')
#define FP              tty[number].fp
#define FPAUX           tty[number].fpaux
#define HANDLE          tty[number].handle[0]
#define HANDLEAUX       tty[number].handle[1]
#define NIVEL           tty[number].nivel
#define ERR(s)          t_printf(number,"%s\n",(s))

#define READ            1
#define READTEXT        2
#define WRITE           3
#define APPEND          4
#define READ_WAIT       5
#define READTEXT_WAIT   6
#define APPENDTEXT      7

/* S_IREAD and S_IWRITE are copied from SYS\STAT.H */
#define S_IREAD         0x0100  /* owner may read  */
#define S_IWRITE        0x0080  /* owner may write */

#define NOTJOINED   "Ud. no esta registrado en ninguna conferencia\n"

#define MAXLOGIN           3
#define MAXLOGINTIME      60L
#define MAXNEWUSERTIME   600L
#define MAXHEADERLINES    10
#define MAXMENUITEMS      30
#define MAXLOCKS          50
#define MAXMSGS          100
#define MAXHANDLES         2
#define MAXTELECONFLINES  20

#define PWDFILE        "password.dat"
#define BILLFILE       "billing.dat"
#define NEWUSERFILE1   "msg/newuser1.msg"
#define NEWUSERFILE2   "msg/newuser2.msg"
#define SCRATCHPATH    "/cbbs/scratch/"
#define PERFILPATH     "/cbbs/perfil/"
#define CINFOPATH      "/cbbs/cinfo/"
#define MAILPATH       "/cbbs/mail/"
#define CONFPATH       "/cbbs/conf/"
#define DBFPATH        "/cbbs/dbf/"

#define UPLOADPATH     "d:/cbbs/loads/"
#define UPSCRATCHPATH  "d:/cbbs/scratch/"

#define CDROMCONF      "1_1000"
#define CDROMINFO      "c:/dod/1_1000.upp"
#define UPLOADSEQ      "/cbbs/scratch/UPLOAD.SEQ"
#define IPSCAPFILE     "/cbbs/scratch/IPS.CAP"     /* Ojo, redefinidas en alg£n *.CPP para no incluir */
#define IPSFILE        "/cbbs/scratch/IPS.FIL"     /* cbbs.h */
#define MSGDELIM       "Msg:"

#define BAUD      1200L    /* Default Modem Baudrate */
#define TXBUFSIZE 128
#define RXBUFSIZE 128
#define STACKSIZE 8192
#define TERMROWS  20

/* tty[x].status */
#define CONNECTED           1
#define DISCONNECTED        2
#define REQ_DISCONNE        3
#define CONNECT_IN_PROGRESS 4
#define WAITING_CALL        5
#define OUTDIAL             6

#define EDIT         0     /* editor parameter */
#define TRANSMITE    1     /* editor parameter */

#define INVALID      0
#define MENU         1
#define TYPE         2
#define MAIL         3
#define CONF         4
#define FIND         5
#define PERFIL       6
#define TELECONF     7
#define DEMODELPHI   8
#define DELPHI       9
#define REUTACCESS  10
#define PED_MYSSA   11
#define SHOWDISK    12
#define DOWNDISK    13
#define QUIT        14

#define S_MENU       "MENU"
#define S_TYPE       "TYPE"
#define S_MAIL       "MAIL"
#define S_CONF       "CONF"
#define S_FIND       "FIND"
#define S_PERFIL     "PERFIL"
#define S_TELECONF   "TELECONF"
#define S_DEMODELPHI "DEMODELP"
#define S_DELPHI     "DELPHI"
#define S_REUTACCESS "REUTACCESS"
#define S_PED_MYSSA  "PED_MYSSA"
#define S_SHOWDISK   "SHOWDISK"
#define S_DOWNDISK   "DOWNDISK"
#define S_QUIT       "QUIT"

#ifdef COOPERATIVE
#   define RQ /**/
#   define RL /**/
#   define pause() schedule()
#else
#   define RQ request_resource(&tclib,0L);
#   define RL release_resource(&tclib)
#   define pause() /**/
#endif

/************ ENUMS **************/

typedef enum {
   CONSOLE,
   MODEM,
   PACKET,
   LOCALTERM,
   NOTHING
   } termtype;

typedef enum {
   CONEXION,
   PAGO,
   RETRIBUCION,
   INVITACION,
   INSCRIPCION
   } billtype;


/*********** STRUCTURES ***********/

typedef struct {
   farptr   xx;
   int      number;
   mailbox  mbox;
   sioptr   siop;
   char     user[9];
   char     mini[13]; /* Menu inicial */
   char     echo;
   long     con_time;
   termtype ttype;
   int      status;
   char     *filename;
   char     *mode;
   int      lmode;
   FILE     *fp;
   FILE     *fpaux;
   int      handle[MAXHANDLES];
   int      nivel;
   int      issysop;
   int      tconf;
   int      more;
   int      ocu;
   int      outdial;
   char     doing [31];
   } STTY;

typedef struct {
   char item;
   char command;
   char param[80];
   } SMENU;

typedef struct {
   int  msg;
   int  deleted;
   long offset;
   } MSGPTR;

typedef struct {
   int rowini;
   int colini;
   int rowfin;
   int colfin;
   int col;
   } WINDOW;

typedef struct {
   char     user[9];
   long     time;
   billtype type;
   int      units;
   } SBILL;

typedef struct {
   char confname[9];
   int  lastmsg;
   } CNFINFO;

/**************** DEFINICION DE VARIABLES ***********************/

#ifdef CBBSMAIN
#   define xtrn /* */
#else
#   define xtrn extern
#endif

/*********** GLOBAL VARIABLES ************/

xtrn struct tm *st;
xtrn STTY tty[10];
xtrn CNFINFO conf_messages [MAXCONFS];

xtrn tcb  user0_tcb,
          user1_tcb,
          user2_tcb,
          user3_tcb,
          user4_tcb,
          user5_tcb,
          user6_tcb,
          host_tcb,
          ips_tcb,
          watchdog_tcb;

xtrn sioptr host_siop;
xtrn sioptr ips_siop;
xtrn resource hostcommsrc;
xtrn resource tclib;
xtrn counter  ipscounter;
xtrn int      parsing_ips;

xtrn long _clock;    /* actualizado por watchdog */
xtrn int  endrun;

xtrn int  mon_channel;
xtrn int  loginseq;
xtrn int  monitor;
xtrn int  anywhere;

xtrn byteptr stkwatch;
xtrn byteptr stkuser0;
xtrn byteptr stkuser1;
xtrn byteptr stkuser2;
xtrn byteptr stkuser3;
xtrn byteptr stkuser4;
xtrn byteptr stkuser5;
xtrn byteptr stkuser6;
xtrn byteptr stkhost;
xtrn byteptr stkips;

xtrn char tconf[MAXTELECONFLINES][81];
xtrn int  tconf_tail;
xtrn int  indemodelphi=FALSE;

/****************** DEFINICIONES NECESARIAS (por el orden de compilacion) *****/
void far usertask(long num);

/* funciones definidas en 'terminal.c' */
xtrn int  connected(int number);
xtrn int  t_kbhit(int number);
xtrn int  confirm(int number, char *prompt);
xtrn int  more(int number);
xtrn int  t_waitfor(int number, char *waitstring, int timeout);
xtrn char t_getc(int number);
xtrn void wait_for_connection(int number);
xtrn void t_init(int number);
xtrn void t_putchar(int number, char c);
xtrn void t_puts(int number,char *string);
xtrn void t_printf(int number, const char *format, ...);
xtrn void t_gets(int number,char *string,int len);
xtrn void t_disconnect(int number);
xtrn void _t_disconne(int number);
xtrn void espacio(int number);
xtrn void typefile(int number, char *filename);
xtrn void init_modem(int number, long baud);


/* funciones definidas en 'filectrl.c' */
xtrn void safe_unlink (int number, char *filename);
xtrn void checkfile (char *filename);
xtrn void t_fclose(int number);
xtrn FILE *t_fopen (int number,char *filename, int lmode);

/* funciones definidas en 'taskctrl.c' */
xtrn void create(int number);
xtrn void start(int number);
xtrn void kill(int number);

/* funciones definidas en 'rfc822.c' */
xtrn char *secondtoken(char *s);
xtrn char *f_gets(char *s,int len,FILE *fp);
xtrn int  loadheader(FILE *fp,int n,...);
xtrn int  nextmsg(FILE *fp, char *delim);

/* funciones definidas en 'login.c' */
xtrn char *validuser(int number, char *username, char *password);
xtrn char *login(int number);
xtrn int  already_in(char *user, int *n);
xtrn int  isuser(int number, char *username);
xtrn long locateuser(int number, char *username, char *password);
xtrn void show_who(int number);
xtrn void newuser(int number);
xtrn void chgpassword(int number);
xtrn void adduser(int number);
xtrn void writebill(char *user, billtype type, int units);

/* funciones definidas en 'lib.c' */
xtrn char *makefilename(char *filename, char *path, char *name, char *ext);
xtrn int existfile(int number,char *file);
xtrn int within(char *str1, char *str2);
xtrn int hardhandler(int errval, int ax, int bp,  int si);
xtrn int is_in_list(int number, char *file, int def);
xtrn void copymsg(long start, long end, FILE *fpsrc, FILE *fpdestin);
xtrn void typemsg(int number, long start, long end);
xtrn void strtrim (char *s, int len);

/* funciones definidas en 'editor.c' */
xtrn void editor(int number, int param, char *user);

/* funciones definidas en 'perfil.c' */
xtrn void showlastcon(int number, char *user);
xtrn void showperfil(int number);
xtrn void loadperfil(int number, char *user);

/* funciones definidas en 'cmail.c' */
int seqmsg(char *filename);
int showmails(int number, MSGPTR *msgp);
void checkformail(int number);
void sendmail(int number,char *username,char *user,char *subject,long xflag,char *name);
void cmail(int number, char *param, char *username);

/* funciones definidas en 'c_os.c' */
xtrn void datetime(int number, int mode);
xtrn void sconf(int number);
xtrn void logbill(int number);
xtrn void showbill(int number);
xtrn void showlog(int number);
xtrn void ls(int number, char *path);
xtrn void cat(int number, char *file1, char *file2, int isappend);
xtrn void c_os(int number);
xtrn void kickuser(int number);
xtrn void writelog(int number,char *filename,char *s);
xtrn void showdisk(int number,char *msgno);
xtrn void downdisk(int number,char *msgno);
/* funciones definidas en 'outdial.c' */
xtrn int find_line(int number);
xtrn void outdial(int number);
xtrn void gateway(int number, int out, int timeout);
xtrn int call_system(int number, int baud, int bits, int parity, int stops, int retries, char **telephone);
xtrn void delphi(int number);
xtrn void demodelphi(int number);

/* funciones definidas en 'cfind.c' */
xtrn void cfind(int number, char *param);

/* funciones definidas en 'teleconf.c' */
xtrn void send_to_conf(char *user, char *buf);
xtrn void secretmsg(int number);
xtrn void calluser(int number);
xtrn void read_from_conf(int number, int *thead);
xtrn void teleconf(int number);

/* funciones definidas en 'menues.c' */
xtrn int dispmenu(int number,char *currentmenu,SMENU *menu);
xtrn int validfname(char *name);
xtrn int menu(int number,char *currentmenu);

/* funciones definidas en 'cconf.c' */
xtrn int i_addmsg(char *confname, long offset);
xtrn long i_seekmsg(char *confname, int msgno);
xtrn long i_modimsg(char *confname, int msgno, long newoffset);
xtrn int i_maxmsg(char *confname);
xtrn int update_cnfinfo(int number, char *user, char *conf, int maxmsg);
xtrn void update_cnfstat(int number, char *conf, int minutes);
xtrn int new_messages(char *conf, int lastmsg);
xtrn void shownew(int number, char *user);
xtrn void showstat(int number, char *conf);
xtrn void showconf(int number,char *confname, int msgini, int msgfin, char *search);
xtrn void typeconf(int number, char *confname, int msg);
xtrn void saveconf(int number, char *confname, int msg);
xtrn void sendconf(int number, char *from, char *to, char *subject, long xflag, char *name);
xtrn int checkconf(int number, char *confname, int *readonly, int *ismoderator);
xtrn void cconf(int number, char *param);
xtrn void load_conf_messages (void);

/* funciones definidas en 'cbbs.c' */
xtrn void far usertask(long num);
xtrn void far watchdog(void);

/* funciones definidas en 'xmodem.c' */
xtrn long xm_receive_file   (int number, char *filename, char *lname);
xtrn int  xm_transmite_file (int number, char *filename, char *lname);

#if 0
/* funciones definidas en 'rxreuter.c' */
xtrn void rxreuter (int number);
xtrn void reutaccess (int number, char *servicio, char *currentmenu);
#endif

/* funciones definiedas en 'cbbssnap.c' */
void cbbs_snapshot (int number);

/* funciones definidas en 'myssa.c' */
void ped_myssa (int number);

/* funciones definidas en 'bipart.c' */
int bipart (char *s, char *key, FILE *fp, long offset, int reclen, int keylen, int keypos);

