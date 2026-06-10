/****************************************************************************/
/*     File: HOT.C                                                          */
/*   Author: Richard Iglar                                                  */
/*  Started: April 28, 1996                                                 */
/* Modified: 2026 - Open-source transition for The Major BBS v10            */
/*  Purpose: Code file for DialSoft HotKeys                                 */
/****************************************************************************/

#include "hot.h"

/****************************************************************************/
/* Variable Declarations                                                    */
/****************************************************************************/

CHAR verinfo[]="HotKeys 2.7";

// System Functions
VOID   EXPORT init__dialhot(VOID);
RETVAL ulogon(VOID);
VOID   ulogof(VOID);
RETVAL uinput(VOID);
VOID shtdwn(VOID);
VOID udeltd(CHAR *uid);
INT  glogbl(VOID);

static VOID goback(VOID);
static VOID show_menu(VOID);
static VOID main_menu(VOID);
static VOID runtime(VOID);
static CHAR chrprc(INT chan, INT c);

static VOID poll_personal(VOID);

static VOID coptions(VOID);
static VOID cfinish(FINRET save);

static VOID keyfinish(FINRET save);
static VOID pkeyfinish(FINRET save);

static VOID enablehk(VOID);

CHAR *tvar_listcmd(VOID) {return config.listcmd;}
CHAR *tvar_helpcmd(VOID) {return config.helpcmd;}
CHAR *tvar_enablecmd(VOID) {return config.enablecmd;}
CHAR *tvar_disablecmd(VOID) {return config.disablecmd;}

static
CHAR tmpstr[256];

static
INT hotstt;

#define hotusr hotlst[usrnum]

HMCVFILE hotmb;
DFAFILE *hotbt;

/****************************************************************************/
/* Module Interface Block                                                   */
/****************************************************************************/

struct module block={         // module interface block
     "",                      // name used to refer to this module
     ulogon,                  // user logon supplemental routine
     uinput,                  // input routine if selected
     NULL,                    // status-input routine if selected
     NULL,                    // "injoth" routine for this module
     NULL,                    // user logoff supplemental routine
     ulogof,                  // hangup (lost carrier) routine
     NULL,                    // midnight cleanup routine
     udeltd,                  // delete-account routine
     shtdwn                   // finish-up (sys shutdown) routine
};

/****************************************************************************/
/* Initialize Module                                                        */
/****************************************************************************/

VOID EXPORT init__dialhot(VOID)
{ 
  stzcpy(block.descrp,gmdnam("DialHot.MDF"),MNMSIZ);
  hotstt=register_module(&block);
  hotmb=opnmsg("DIALHOT.MCV");
  hotbt=dfaOpen("DIALHOT.DAT",sizeof(struct hotrec),NULL);

  dclvda(sizeof(struct vdahot));
  dclvda(fsdroom(CNF12,cfsp,0));
  dclvda(fsdroom(CNF21,keyfsp,0));
  dclvda(fsdroom(CNF30,pkeyfsp,0));

  hotlst=(struct hotinf *)alczer(nterms*sizeof(struct hotinf));

  /* Load configuration, or insert defaults on first run */
  usrnum=-1;
  dfaSetBlk(genbb);
  setmem(&config,sizeof(struct configrec),0);
  stzcpy(config.userid,"Sysop",UIDSIZ);
  stzcpy(config.modnam,"DIALHOT",MNMSIZ);
  if (dfaAcqEQ(&config,&config,0))
    {shocst("HotKeys: Loaded",verinfo);
    }
  else
    {config.flags=SHOWLGNMSG;
     strcpy(config.syskey,"SYSOP");
     strcpy(config.hotkey,"NORMAL");
     strcpy(config.clrkey,"NORMAL");
     strcpy(config.pkey,"NORMAL");
     setmem(&config.desc[0],780,0);
     strcpy(config.desc[0],"Chat/Teleconference");
     strcpy(config.command[0],"/GO TELE");
     strcpy(config.desc[9],"TOP Menu");
     strcpy(config.command[9],"/GO TOP");
     strcpy(config.desc[12],"Who's Online");
     strcpy(config.command[12],"/#");
     strcpy(config.desc[13],"Exit the system");
     strcpy(config.command[13],"/GO EXIT");
     strcpy(config.enablecmd,"/HOTON");
     strcpy(config.disablecmd,"/HOTOFF");
     strcpy(config.listcmd,"/HOTLIST");
     strcpy(config.helpcmd,"/HOTKEYS");
     dfaInsertV(&config,sizeof(struct configrec));
     shocst("HotKeys: Initializing defaults",verinfo);
    }

  /* For updating purposes */
  if (!(*config.helpcmd))
    {strcpy(config.enablecmd,"/HOTON");
     strcpy(config.disablecmd,"/HOTOFF");
     strcpy(config.listcmd,"/HOTLIST");
     strcpy(config.helpcmd,"/HOTKEYS");
     strcpy(config.pkey,"NORMAL");
     dfaUpdateV(&config,sizeof(struct configrec));
    }
  dfaRstBlk();

  register_textvar("DIA_HOTENABLE",tvar_enablecmd);
  register_textvar("DIA_HOTDISABLE",tvar_disablecmd);
  register_textvar("DIA_HOTHELP",tvar_helpcmd);
  register_textvar("DIA_HOTLIST",tvar_listcmd);

  globalcmd(glogbl);  //Set global command handler
  rtkick(5,runtime);
}

/****************************************************************************/
/* Run Time rtkick routines                                                 */
/****************************************************************************/

/****************************************************************************/
/* User Logon Supplemental Routine                                          */
/****************************************************************************/

RETVAL ulogon(VOID)
{
  setmem(&hotusr,sizeof(struct hotinf),0);
  if (config.flags&SHOWLGNMSG && haskey(config.hotkey))
    {setmbk(hotmb);
     prfmsg(HOTKEYAD);
     outprf(usrnum);
     rstmbk();
    }
  return(0);
}

/****************************************************************************/
/* Input Routine                                                            */
/****************************************************************************/

RETVAL uinput(VOID)
{
  setmbk(hotmb);
  dfaSetBlk(hotbt);
  if (margc > 0 && (sameas("x",margv[0]) || sameas("exit",margv[0])))
    {
     clrxrf();
     switch (usrptr->substt)
       {case MAINPRM1: return(0);

        default: goback();
                 break;
       }
    }
  else
    {
     do {
        bgncnc();
        switch (usrptr->substt)
          {
             case 0: //user just got in
                     cncchr();
                     show_menu();
                     usrptr->substt=MAINPRM1;
                     break;

              case MAINPRM1: if (sameas(margv[0],"config") && usrptr->flags&MASTER)
                              {cncall();
                               coptions();
                              }
                            else main_menu();
                            break;

           }
        } while (!endcnc());
    }
  outprf(usrnum);
  return(1);
}

/****************************************************************************/
/* Status-Input Routine                                                     */
/****************************************************************************/

/****************************************************************************/
/* "injoth" Routine                                                         */
/****************************************************************************/

/****************************************************************************/
/* User Logoff Supplemental Routine                                         */
/****************************************************************************/

/****************************************************************************/
/* Hangup Routine                                                           */
/****************************************************************************/

VOID ulogof(VOID)
{
  btuchi(usrnum,NULL);
  setmem(&hotusr,sizeof(struct hotinf),0);
}

/****************************************************************************/
/* Midnight Clean-up Routine                                                */
/****************************************************************************/

/****************************************************************************/
/* Delete Account Routine                                                   */
/****************************************************************************/

VOID udeltd(CHAR *uid)
{
  dfaSetBlk(hotbt);
  if (dfaAcqEQ(NULL,uid,0)) dfaDelete();
  dfaRstBlk();
}

/****************************************************************************/
/* System Shutdown Routine                                                  */
/****************************************************************************/

VOID shtdwn(VOID)
{
  clsmsg(hotmb);
  dfaClose(hotbt);
}

/****************************************************************************/
/* Global Command Handler                                                   */
/****************************************************************************/

INT glogbl(VOID)
{
  CHAR phks[]="!@#$%^&*()";
  static CHAR *hk="CDEGKNOPQTUVWXYZ";
  INT c,i,syskey;

  if (!margc) return(0);

  setmbk(hotmb);
  dfaSetBlk(hotbt);
  if (sameas(margv[0],config.enablecmd) && haskey(config.hotkey))
    {enablehk();
     prfmsg(HOTON);
     outprf(usrnum);
     return(1);
    }
  else if (sameas(margv[0],config.disablecmd))
    {btuchi(usrnum,NULL);
     prfmsg(HOTOFF);
     outprf(usrnum);
     return(1);
    }
  else if (sameas(margv[0],config.listcmd))
    {if (dfaAcqEQ(NULL,usaptr->userid,0))
       {prfmsg(PKEYHDR);
        for (i=0; i < 10; i++) prfmsg(PKEYLIN,(i < 9) ? i+1:0,phks[i],hotbuf->command[i]);
        prfmsg(PKEYFTR);
        outprf(usrnum);
        return(-1);
       }
     prfmsg(NOTDEF2);
     outprf(usrnum);
     return(1);
    }
  else if (sameas(margv[0],config.helpcmd))
    {syskey=haskey(config.syskey);
     btucli(usrnum);
     btupbc(usrnum,0);
     prfmsg(HKHELP);
     i=0;
     for (c=0; c < 16; c++)
       {
        if (*config.command[c])
          {if (*config.desc[c] == '@' && syskey)
             {i++;
              prfmsg(HKHLP2H,hk[c],&config.desc[c][1]);
              if (!(i%2)) prf("\n");
             }
           else if (*config.command[c] != '@')
             {i++;
              prfmsg(HKHLP2,hk[c],config.desc[c]);
              if (!(i%2)) prf("\n");
             }
          }
       }
     prfmsg(HKHLPFTR);
     outprf(usrnum);
     clrprf();
     btupbc(usrnum,20);
     return(-1);
    }

  rstmbk();
  dfaRstBlk();
  return(0);
}

/****************************************************************************/
/* Text Variable Functions                                                  */
/****************************************************************************/

/****************************************************************************/
/* Procedures & Functions                                                   */
/****************************************************************************/


/*
  Shows the main menu
*/
static VOID show_menu(VOID)
{
  prfmsg(MAIN10);
  if (haskey(syskey)) prfmsg(MAIN10S);
  prfmsg(MAINPRM1,verinfo);
}

/*
  Moves to main menu upon 'X'
*/
static VOID goback(VOID)
{
  condex();
  prfmsg(MAINPRM1,verinfo);
  usrptr->state=hotstt;
  usrptr->substt=MAINPRM1;
}

/*
  Main menu input handler
*/
static VOID main_menu(VOID)
{ CHAR command;

  if (!margc)
    {if (usrptr->flags&INJOIP) goback();
     else show_menu();
    }
  else
    {command=cncchr();
     switch (command)
       {
        case '?': show_menu();
                  break;

        case 'H': prfmsg(INFORMA);
                  goback();
                  break;

        case 'P': if (haskey(config.pkey))
                    {if (!dfaAcqEQ(NULL,usaptr->userid,0)) setmem(hotbuf,sizeof(struct hotrec),0);
                     fsdroom(CNF30,pkeyfsp,1); //full page edit session
                     sprintf(vdatmp,pkeyfmt,hotbuf->command[0],'\0',
                                            hotbuf->command[1],'\0',
                                            hotbuf->command[2],'\0',
                                            hotbuf->command[3],'\0',
                                            hotbuf->command[4],'\0',
                                            hotbuf->command[5],'\0',
                                            hotbuf->command[6],'\0',
                                            hotbuf->command[7],'\0',
                                            hotbuf->command[8],'\0',
                                            hotbuf->command[9],'\0'
                                           );
                     fsdapr(hotvda->text,4000,vdatmp);
                     fsdbkg(fsdrft());
                     fsdego(vfyadn,pkeyfinish);
                    }
                  else
                    {prfmsg(NOPKEY);
                     goback();
                    }
                  break;

         default: if (haskey(config.syskey))
                    {switch(command)
                       {
                        case 'S': fsdroom(CNF21,keyfsp,1); //full page edit session
                                  sprintf(vdatmp,keyfmt,config.desc[0],'\0',
                                        config.command[0],'\0',
                                        config.desc[1],'\0',
                                        config.command[1],'\0',
                                        config.desc[2],'\0',
                                        config.command[2],'\0',
                                        config.desc[3],'\0',
                                        config.command[3],'\0',
                                        config.desc[4],'\0',
                                        config.command[4],'\0',
                                        config.desc[5],'\0',
                                        config.command[5],'\0',
                                        config.desc[6],'\0',
                                        config.command[6],'\0',
                                        config.desc[7],'\0',
                                        config.command[7],'\0',
                                        config.desc[8],'\0',
                                        config.command[8],'\0',
                                        config.desc[9],'\0',
                                        config.command[9],'\0',
                                        config.desc[10],'\0',
                                        config.command[10],'\0',
                                        config.desc[11],'\0',
                                        config.command[11],'\0',
                                        config.desc[12],'\0',
                                        config.command[12],'\0',
                                        config.desc[13],'\0',
                                        config.command[13],'\0',
                                        config.desc[14],'\0',
                                        config.command[14],'\0'
                                        );
                                  fsdapr(hotvda->text,2048,vdatmp);
                                  fsdbkg(fsdrft());
                                  fsdego(vfyadn,keyfinish);
                                  break;

                         default: prfmsg(NOSUCH);
                                  cncall();
                                  goback();
                                  break;

                       }
                     return;
                    }
                  prfmsg(NOSUCH);
                  cncall();
                  goback();
                  break;
       }
    }
}




#define Hotusr hotlst[chan]

CHAR chrprc(INT chan, INT c)
{ register CHAR num;
  register CHAR *ptr;

  if (Hotusr.lastinp == 27)
    {/* To ignore cursor movements, function keys, etc */
     if (c == '[')
       {Hotusr.lastinp=255;
        return(0);
       }
    }
  /* Also to ignore Cursor movements */
  else if ((INT)Hotusr.lastinp == 255)
    {Hotusr.lastinp=0;
     return(0);
    }
  Hotusr.lastinp=c;

  /* Ignore all high Ascii */
  if (c > 127) return(0);

  if (Hotusr.flags&CHGFOR || Hotusr.flags&CHGBCK) chious(chan,"[s        [u");

  if (!btuibw(chan) && !isalpha(c) && c >= 33 && c <= 94 )
    {switch(c)
       {case '!': Hotusr.pcmd=0;
                  break;
        case '@': Hotusr.pcmd=1;
                  break;
        case '#': Hotusr.pcmd=2;
                  break;
        case '$': Hotusr.pcmd=3;
                  break;
        case '%': Hotusr.pcmd=4;
                  break;
        case '^': Hotusr.pcmd=5;
                  break;
        case '&': Hotusr.pcmd=6;
                  break;
        case '*': Hotusr.pcmd=7;
                  break;
        case '(': Hotusr.pcmd=8;
                  break;
        case ')': Hotusr.pcmd=9;
                  break;
        default:  Hotusr.pcmd=100;
                  break;
       }
     if (Hotusr.pcmd < 10)
       {begin_polling(chan,poll_personal);
        return(c);
       }
    }

  /* All printable characters */
  if (c >= 32 && c != 127 && c != '`')
    {
     if (Hotusr.flags&CHGFOR)
       {Hotusr.flags&=~CHGFOR;
        for (num=0;num < 7;num++) chiinp(chan,fcolors[Hotusr.fcolor][num]);
        Hotusr.clrchg[++Hotusr.clrcnt]=btuibw(chan);
       }
     if (Hotusr.flags&CHGBCK)
       {Hotusr.flags&=~CHGBCK;
        for (num=0;num < 5;num++) chiinp(chan,bcolors[Hotusr.bcolor][num]);
        Hotusr.clrchg[++Hotusr.clrcnt]=-btuibw(chan);
       }
     return(c);
    }

  /* All Control Characters */
  switch(c)
    {
      /* CTRL-C */
      case   3: c=0;
                break;

      /* CTRL-D */
      case   4: c=1;
                break;

      /* CTRL-E */
      case   5: c=2;
                break;

      /* CTRL-G */
      case   7: c=3;
                break;

      /* CTRL-K */
      case  11: c=4;
                break;

      /* CTRL-N */
      case  14: c=5;
                break;

      /* CTRL-O */
      case  15: c=6;
                break;

      /* CTRL-P */
      case  16: c=7;
                break;

      /* CTRL-Q */
      case  17: c=8;
                break;

      /* CTRL-T */
      case  20: c=9;
                break;

      /* CTRL-U */
      case  21: c=10;
                break;

      /* CTRL-V */
      case  22: c=11;
                break;

      /* Control-W */
      case  23: c=12;
                break;

      /* CTRL-X */
      case  24: c=13;
                break;

      /* CTRL-Y */
      case  25: c=14;
                break;


      /* CTRL-Z HELP */
      case  26: btucli(chan);
                for (ptr=config.helpcmd; *ptr; ptr++) chiinp(chan,*ptr);
                return('\r');

      /*
        Pre-Defined Hot Keys
      */

      /* CTRL-H: Backspace  */
      case 127:
      case   8: num=btuibw(chan);
                if (Hotusr.clrcnt && (abs(Hotusr.clrchg[Hotusr.clrcnt]) == num || num == 8 || num == 6))
	          {num=(Hotusr.clrchg[Hotusr.clrcnt] < 0) ? 5:7;
                   for (;num > 0; num--) chiinp(chan,'\b');
                   Hotusr.clrcnt--;
	          }
               Hotusr.flags&=~(CHGFOR+CHGBCK);
	       if (btuibw(chan)) return('\b');
               else return(0);


      /* CTRL-B Background color */
      case   2: if (!(Hotusr.flags&CANCLR) || Hotusr.clrcnt > 29) return(0);
                chious(chan,"[s");
                Hotusr.bcolor = (Hotusr.bcolor % 8)+1;
                chious(chan,bcolors[Hotusr.bcolor]);
                chious(chan,"[u");
	        Hotusr.flags|=CHGBCK;
	        return(0);

      /* CTRL-I : TAB Color change */
      /* CTRL-F Foreground color */
      case   6:
      case   9: if (!(Hotusr.flags&CANCLR) || Hotusr.clrcnt > 29) return(0);
                chious(chan,"[s");
                Hotusr.fcolor = (Hotusr.fcolor % 16)+1;
                chious(chan,fcolors[Hotusr.fcolor]);
                chious(chan,"[u");
	        Hotusr.flags|=CHGFOR;
	        return(0);

      case '`': if (!(Hotusr.flags&CANCLR) || Hotusr.clrcnt > 29) return(0);
                if (Hotusr.fcolor > 1) Hotusr.fcolor--;
                else Hotusr.fcolor = 16;
	        chious(chan,"[s");
                chious(chan,fcolors[Hotusr.fcolor]);
	        chious(chan,"[u");
	        Hotusr.flags|=CHGFOR;
	        return(0);

      /* CTRL-J : Disable HotKeys, calls the global command */
      case  10: btucli(chan);
                for (ptr=config.disablecmd; *ptr; ptr++) chiinp(chan,*ptr);
                return('\r');

      /* CTRL-L : Clear screen */
      case  12: chious(chan,"[0m[2J");

      /* Control-R redraw */
      case  18: if (btuibw(chan) && btuinp(chan,tmpstr) == -1)
	          {chious(chan,"\r\n");
	           chious(chan,tmpstr);
	          }
	        return(0);

      /* CTRL-A or <ESC> Abort */
      case   1:
      case  27: if (!btuibw(chan)) return(0);
	        btucli(chan);
	        chious(chan,"\r\n");
	        chious(chan,"\r\nInput Aborted!\r\n");
                return(0);

      /* ENTER */
     case '\r': Hotusr.flags&=~(CHGFOR+CHGBCK);
		Hotusr.clrcnt=Hotusr.fcolor=Hotusr.bcolor=0;
                //chious(chan,"[0m");
		return(c);

       /* All other characters */
       default: //chious(chan,spr("%d",(int)c));
                return(0);
    }

  /* SysOp Defined Hotkey */
  if (*config.command[c])
    {btucli(chan);
     for (ptr=config.command[c]; *ptr; ptr++) chiinp(chan,*ptr);
     return('\r');
    }
  return(0);
}


/*
  runtime module that enables the HotKeys
*/
static VOID runtime(VOID)
{
  setmbk(hotmb);
  for (usrnum=0;usrnum < nterms; usrnum++)
    {
     curusr(usrnum);
     if (usrptr->usrcls == ACTUSR
         && !(usrptr->flags&0x00800000L)
         && usrptr->state == 0
         && haskey(config.hotkey)
        )
       enablehk();
     else
       hotusr.flags&=~CHRPRC;
    }
  rtkick(1,runtime);
}


/*
  Enables HotKeys on the channel
*/
static VOID enablehk(VOID)
{
  if (!(hotusr.flags&CHRPRC))
    {hotusr.flags|=CHRPRC;
     btuchi(usrnum,chrprc);
    }

  if (haskey(config.clrkey) && usaptr->ansifl&ANSON) hotusr.flags|=CANCLR;
  else hotusr.flags&=~CANCLR;
}


/*
  Personal Hot Key launcher
*/
static VOID poll_personal(VOID)
{
  CHAR *ptr,*cmd=NULL;
  CHAR phks[]="!@#$%^&*()";

  dfaSetBlk(hotbt);
  setmbk(hotmb);

  if (dfaAcqEQ(NULL,usaptr->userid,0) && hotbuf->command[hotusr.pcmd][0] != '\0')
    {btucli(usrnum);
     chious(usrnum,"\b");
     prfmsg(XCUTING,phks[hotusr.pcmd],hotbuf->command[hotusr.pcmd]);
     for (ptr=hotbuf->command[hotusr.pcmd]; *ptr; ptr++)
       {if (*ptr == '~')
          {chiinp(usrnum,'\r');
           cmd=NULL;
          }
        else
          {if (cmd==NULL) cmd=ptr;
           chiinp(usrnum,*ptr);
          }
       }
     /* User didn't add final ENTER, add a space */
     if (cmd != NULL)
       {chious(usrnum,hotbuf->command[hotusr.pcmd]);
        chiinp(usrnum,' ');
        chious(usrnum," ");
       }
     else outprf(usrnum);
    }
  stop_polling(usrnum);
  rstmbk();
  dfaRstBlk();
}


/*
  Kicks off the Full Screen Data Entry session for CNF opts
*/
static VOID coptions(VOID)
{
  fsdroom(CNF12,cfsp,1);  //full page edit session
  sprintf(vdatmp,cfmt,
          "",'\0',
          (config.flags&SHOWLGNMSG) ? "YES":"NO",'\0',
          config.syskey,'\0',
          config.hotkey,'\0',
          config.pkey,'\0',
          config.clrkey,'\0',
          config.helpcmd,'\0',
          config.enablecmd,'\0',
          config.disablecmd,'\0',
          config.listcmd,'\0'
         );
  fsdapr(hotvda->text,4096,vdatmp);
  fsdbkg(fsdrft());
  fsdscb->flddat[ACTCODEFLD].flags|=FFFAVD;  // ACT field is inert in open-source build
  fsdego(vfyadn,cfinish);
}



/*
  Called after FSDE session
*/
static VOID cfinish(FINRET save)
{
  setmbk(hotmb);
  if (fsdscb->chgcnt == 0 || !save) prfmsg(NOCHANG);
  else
    {
     config.flags=0;
     if (fsdord(SHOWLGNMSGFLD)) config.flags|=SHOWLGNMSG;
     fsdfxt(SYSKEYFLD,config.syskey,16);
     fsdfxt(HOTKEYFLD,config.hotkey,16);
     fsdfxt(PKEYFLD,config.pkey,16);
     fsdfxt(CLRKEYFLD,config.clrkey,16);
     fsdfxt(HELPCMDFLD,config.helpcmd,16);
     fsdfxt(ENABLECMDFLD,config.enablecmd,16);
     fsdfxt(DISABLECMDFLD,config.disablecmd,16);
     fsdfxt(LISTCMDFLD,config.listcmd,16);
     dfaSetBlk(genbb);
     if (dfaAcqEQ(vdatmp,&config,0)) dfaUpdateV(&config,sizeof(struct configrec));
     prfmsg(SETSAVD);
    }
  enablehk();
  goback();
}


/*
  Called after FSDE session
*/
static VOID keyfinish(FINRET save)
{ CHAR c;

  setmbk(hotmb);
  if (!save || fsdscb->chgcnt == 0) prfmsg(NOCHANG);
  else
    {
     for (c=0; c < 15; c++)
       {fsdfxt(c*2,config.desc[c],21);
        fsdfxt((c*2)+1,config.command[c],31);
       }
     dfaSetBlk(genbb);
     if (dfaAcqEQ(vdatmp,&config,0)) dfaUpdateV(&config,sizeof(struct configrec));
     prfmsg(SETSAVD);
    }
  enablehk();
  goback();
}





/*
  Called after FSDE session
*/
static VOID pkeyfinish(FINRET save)
{ CHAR c;

  setmbk(hotmb);
  if (!save || fsdscb->chgcnt == 0) prfmsg(NOCHANG);
  else
    {setmem(hotbuf,sizeof(struct hotrec),0);
     strcpy(hotbuf->userid,usaptr->userid);
     for (c=0; c < 10; c++) fsdfxt(c,hotbuf->command[c],61);
     prfmsg(SETSAVD);

     dfaSetBlk(hotbt);
     dfaAcqEQ(vdatmp,usaptr->userid,0) ? dfaUpdate(NULL):dfaInsert(NULL);
     dfaRstBlk();
    }
  enablehk();
  goback();
}

