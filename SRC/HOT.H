/****************************************************************************/
/*     File: HOT.H                                                          */
/*   Author: Richard Iglar                                                  */
/*  Started: February 16, 1996                                              */
/* Modified: 2026 - Updated includes for The Major BBS v10 SDK              */
/*  Purpose: Header file for DialSoft HotKeys                               */
/****************************************************************************/

#include "gcomm.h"
#include "majorbbs.h"
#include "fsdbbs.h"
#include "DIALHOT.H"

/* RETVAL and FINRET are legacy Borland-era types not defined in the v10 SDK.
   Map them to the v10 equivalents so HOT.C compiles without modification. */
typedef GBOOL RETVAL;
typedef SHORT FINRET;


/*
  Config struct, holds settings for NOMSG
  Size = 1024
  Must be packed to 1-byte alignment to match the original Borland-compiled
  on-disk layout.  The USHORT after modnam[25] sits at byte offset 55 (odd);
  without pack(1) the compiler inserts a pad byte and shifts every subsequent
  field, corrupting reads from the Btrieve NOMSG record.
*/
#pragma pack(push, 1)
struct configrec {
   CHAR   userid[UIDSIZ]; // 30 userid
   CHAR   modnam[MNMSIZ]; // 25 module name
   USHORT reserved1;      //  2 reserved (formerly: day of install)
   USHORT reserved2;      //  2 reserved (formerly: last run date)
   SHORT  reserved3;      //  2 reserved (formerly: days running)
   CHAR   reserved4[8];   //  8 reserved (formerly: activation code)
   SHORT  flags;          //  2 flags for bouncer
   CHAR   syskey[16];     // 16 sysop/webmaster key
   CHAR   hotkey[16];     // 16 key required to use hotkeys
   CHAR   clrkey[16];     // 16 note key
   CHAR   desc[15][21];   // 315 description array
   CHAR   command[15][31];// 465 command array
                        // === 899

   CHAR   pkey[16];       // 16 key required to make personal hotkeys
   CHAR   enablecmd[16];  // 16 global command to enable hotkeys
   CHAR   disablecmd[16]; // 16 disable hotkeys
   CHAR   listcmd[16];    // 16 list personal hotkeys
   CHAR   helpcmd[16];    // 16 help command
                        // === 80
   CHAR   spare[1024-979];
  } config;
#pragma pack(pop)




struct vdahot {
  CHAR text[8000];
};

#define hotptr  ((struct vdahot *)vdaptr)
#define hotvda  ((struct vdahot *)vdaptr)

struct hotinf {
  CHAR  flags;                 //  1 flags
  CHAR  fcolor;                //  1 Current color
  CHAR  bcolor;                //  1 Current color
  CHAR  clrcnt;                //  1 Num of colors
  CHAR  lastinp;               //  1 last input.
  CHAR  pcmd;                  //  1 personal command
  SHORT clrchg[30];            // 60 Array for color changing
} *hotlst;


//Btrieve record = 1000
struct hotrec
{ CHAR userid[30];      //  30 UserId
  CHAR command[10][61]; // 610 command array

  CHAR spare[360];
};

#define hotbuf ((struct hotrec *)hotbt->data)




/* Flags for module */
#define SHOWLGNMSG 0x0001

static CHAR *fcolors[]={"","\x1b[0;30m\x1b[0;37mBLACK",
                        "\x1b[0;34mBLUE",
                        "\x1b[0;32mGREEN",
                        "\x1b[0;36mCYAN",
                        "\x1b[0;31mRED",
                        "\x1b[0;35mMAGENTA",
                        "\x1b[0;33mBROWN",
                        "\x1b[0;37mGRAY",
                        "\x1b[1;30mMD.GRAY",
                        "\x1b[1;34mLT.BLUE",
                        "\x1b[1;32mLT.GREEN",
                        "\x1b[1;36mLT.CYAN",
                        "\x1b[1;31mLT.RED",
                        "\x1b[1;35mPINK",
                        "\x1b[1;33mYELLOW",
                        "\x1b[1;37mWHITE"};

static CHAR *bcolors[]={"","\x1b[40mBLACK   ",
                        "\x1b[44mBLUE    ",
                        "\x1b[42mGREEN   ",
                        "\x1b[46mCYAN    ",
                        "\x1b[41mRED     ",
                        "\x1b[45mMAGENTA ",
                        "\x1b[43mBROWN   ",
                        "\x1b[47mGRAY    "};

/* Flags for users */
#define CHRPRC 0x0001
#define CHGFOR 0x0002
#define CHGBCK 0x0004
#define CANCLR 0x0008
#define OUTPUT 0x0010

#define CTRLCC 0
#define CTRLCD 1
#define CTRLDC 2
#define CTRLDD 3
#define CTRLEC 4
#define CTRLED 5
#define CTRLGC 6
#define CTRLGD 7
#define CTRLKC 8
#define CTRLKD 9
#define CTRLNC 10
#define CTRLND 11
#define CTRLOC 12
#define CTRLOD 13
#define CTRLPC 14
#define CTRLPD 15
#define CTRLQC 16
#define CTRLQD 17
#define CTRLTC 18
#define CTRLTD 19
#define CTRLUC 20
#define CTRLUD 21
#define CTRLVC 22
#define CTRLVD 23
#define CTRLWC 24
#define CTRLWD 25
#define CTRLXC 26
#define CTRLXD 27
#define CTRLYC 28
#define CTRLYD 29

static CHAR keyfmt[]=
     "CC=%s%c"
     "CD=%s%c"
     "DC=%s%c"
     "DD=%s%c"
     "EC=%s%c"
     "ED=%s%c"
     "GC=%s%c"
     "GD=%s%c"
     "KC=%s%c"
     "KD=%s%c"
     "NC=%s%c"
     "ND=%s%c"
     "OC=%s%c"
     "OD=%s%c"
     "PC=%s%c"
     "PD=%s%c"
     "QC=%s%c"
     "QD=%s%c"
     "TC=%s%c"
     "TD=%s%c"
     "UC=%s%c"
     "UD=%s%c"
     "VC=%s%c"
     "VD=%s%c"
     "WC=%s%c"
     "WD=%s%c"
     "XC=%s%c"
     "XD=%s%c"
     "YC=%s%c"
     "YD=%s%c";

static CHAR keyfsp[]=
     "CC "
     "CD "
     "DC "
     "DD "
     "EC "
     "ED "
     "GC "
     "GD "
     "KC "
     "KD "
     "NC "
     "ND "
     "OC "
     "OD "
     "PC "
     "PD "
     "QC "
     "QD "
     "TC "
     "TD "
     "UC "
     "UD "
     "VC "
     "VD "
     "WC "
     "WD "
     "XC "
     "XD "
     "YC "
     "YD "
     "DONE(ALT=SAVE ALT=QUIT ALT=EDIT MULTICHOICE)";


#define ACTCODEFLD    0
#define SHOWLGNMSGFLD 1
#define SYSKEYFLD     2
#define HOTKEYFLD     3
#define PKEYFLD       4
#define CLRKEYFLD     5
#define HELPCMDFLD    6
#define ENABLECMDFLD  7
#define DISABLECMDFLD 8
#define LISTCMDFLD    9

static CHAR cfmt[]=
 "ACT=%s%c"
 "LGN=%s%c"
 "SYS=%s%c"
 "HOT=%s%c"
 "PKY=%s%c"
 "CLR=%s%c"
 "HLP=%s%c"
 "ENBL=%s%c"
 "DSBL=%s%c"
 "LST=%s%c"
 ;

static CHAR cfsp[]=
 "ACT "
 "LGN(ALT=NO ALT=YES MULTICHOICE) "
 "SYS(MIN=1) "
 "HOT "
 "PKY "
 "CLR "
 "HLP(MIN=2) "
 "ENBL(MIN=2) "
 "DSBL(MIN=2) "
 "LST(MIN=2) "
 "DONE(ALT=SAVE ALT=QUIT ALT=EDIT MULTICHOICE)";

#define CTRL1C 0
#define CTRL2C 1
#define CTRL3C 2
#define CTRL4C 3
#define CTRL5C 4
#define CTRL6C 5
#define CTRL7C 6
#define CTRL8C 7
#define CTRL9C 8
#define CTRL0C 9

static CHAR pkeyfmt[]=
     "1C=%s%c"
     "2C=%s%c"
     "3C=%s%c"
     "4C=%s%c"
     "5C=%s%c"
     "6C=%s%c"
     "7C=%s%c"
     "8C=%s%c"
     "9C=%s%c"
     "0C=%s%c";

static CHAR pkeyfsp[]=
     "1C "
     "2C "
     "3C "
     "4C "
     "5C "
     "6C "
     "7C "
     "8C "
     "9C "
     "0C "
     "DONE(ALT=SAVE ALT=QUIT ALT=EDIT MULTICHOICE)";
