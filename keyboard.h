#ifndef CALCOS_KEYBOARD
#define CALCOS_KEYBOARD 0.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h> // Compile with -lpthread
#include <semaphore.h> // Compile with -lpthread

#include <gpiod.h> // Compile with -lgpiod

#define KEYBOARD_EVENT_KEYDOWN 50
#define KEYBOARD_EVENT_KEYUP 51

#define KEYBOARD_EVENT_LOOP_BUFFER_LENGTH 64

// Raw Layout
//['APP', 'MENU',  'SET',   'F1',  'F2',   'F3',  'EXT',  'PWR']
//['DEL', '|-\\',  '<->',  '`-~', '@-#',  '$-&',  '_-=', 'BKSP']
//['!-?',  'q-w',  'e-r',  't-y', 'u-i',  'o-p',    '(',    ')']
//['LSF',  'a-s',  'd-f',  'g-h', 'j-k',  'l-;',   '\'',  'RSF']
//['LCL',    'z',  'x-c',  'v-b', 'n-m',  ',-.',  'SPA',  'RCL']
//[  '7',    '8',    '9',    '/',   '%',  'LOG',  'OPT',  'PUP']
//[  '4',    '5',    '6',    '*',   '^',  'SQR',  'SYB',  'PDN']
//[  '1',    '2',    '3',    '-', 'HOM',  'UPK',  'END',  'BAK']
//[  '0',    '.',  'pie',    '+', 'LFT',  'DWN',  'RHT',  'ENT']

// Encoded Layout
//enum keyval{
//	KAPP,MENU,KSET,KF_1,KF_2,KF_3,KEXT,KPWR,
//	KDEL,BSLH,AGBK,TILD,SHRP,AMPR,EQUL,BKSP,
//	EXCL,KQ_W,KE_R,KT_Y,KU_I,KO_P,LPAR,RPAR,
//	LSFT,KA_S,KD_F,KG_H,KJ_K,KLSL,APTP,RSFT,
//	LCTR,K__Z,KX_C,KV_B,KN_M,COMA,SPAC,RCTR,
//	NUM7,NUM8,NUM9,MDIV,MPCT,MLOG,MOPT,APGU,
//	NUM4,NUM5,NUM6,MMLT,MPWR,MSQR,MSYB,APGD,
//	NUM1,NUM2,NUM3,MMNS,AHOM,A_UP,AEND,BACK,
//	NUM0,MDOT,MPIE,MPLS,ALFT,ADWN,ARHT,ENTR
//};

// Reordered for easier text parsing.
enum keyval{
	KQ_W,KE_R,KT_Y,KU_I,KO_P,
	KA_S,KD_F,KG_H,KJ_K,KLSL,
	KX_C,KV_B,KN_M,
	COMA,EXCL,BSLH,AGBK,TILD,SHRP,AMPR,EQUL,
	K__Z,LPAR,RPAR,APTP,SPAC,
	NUM0,NUM1,NUM2,NUM3,NUM4,NUM5,NUM6,NUM7,NUM8,NUM9,MDOT,
	MPIE,MPLS,MMNS,MMLT,MDIV,
	MPCT,MLOG,MPWR,MSQR,
	MOPT,MSYB,
	KDEL,BKSP,LSFT,RSFT,LCTR,RCTR,
	KAPP,MENU,KSET,KF_1,KF_2,KF_3,KEXT,KPWR,
	APGU,APGD,AHOM,AEND,A_UP,ADWN,ALFT,ARHT,
	BACK,ENTR,
	KNIL
};

extern const enum keyval keymap[9][8];

struct keyboard_event{
	int key_r,key_c,event_type;
};

void KbdInit();

void* _kbdmonitorproc(void *arg);

void KbdStartMonitoring();

void KbdStopMonitoring();

void KbdDestroy();

int _kbdevent_append(int key_r,int key_c,int event_type);

struct keyboard_event *KbdWaitEvent();

void KbdPostEvent();

#endif