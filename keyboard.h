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

enum keyval{
	KAPP,MENU,KSET,KF_1,KF_2,KF_3,KEXT,KPWR,
	KDEL,BSLH,AGBK,TILD,SHRP,AMPR,EQUL,BKSP,
	EXCL,KQ_W,KE_R,KT_Y,KU_I,KO_P,LPAR,RPAR,
	LSFT,KA_S,KD_F,KG_H,KJ_K,KLSL,APTP,RSFT,
	LCTR,K__Z,KX_C,KV_B,KN_M,COMA,SPAC,RCTR,
	NUM7,NUM8,NUM9,MDIV,MPCT,MFRC,MOPT,APGU,
	NUM4,NUM5,NUM6,MMLT,MPWR,MSQR,MSYB,APGD,
	NUM1,NUM2,NUM3,MMNS,AHOM,A_UP,AEND,BACK,
	NUM0,MDOT,MPIE,MPLS,ALFT,ADWN,ARHT,ENTR
};

extern const enum keyval keymap[9][8];

struct keyboard_event{
	int key_r,key_c,event_type;
	struct keyboard_event *next;
};

struct keyboard{
	int rows,cols;
	struct gpiod_chip *chip;
	struct gpiod_line_bulk *rowpins,*colpins;
	sem_t *event_flag;
	sem_t *event_access;
	struct keyboard_event *event_buffer;
	pthread_t monitor_tid;
	pthread_attr_t thattr;
	int monitor_runflag;
};

struct keyboard* KbdInit();

void* _kbdmonitorproc(void *arg);

void KbdStartMonitoring(struct keyboard *kbd);

void KbdStopMonitoring(struct keyboard *kbd);

void KbdDestroy(struct keyboard *kbd);

void _kbdevent_append(struct keyboard *kbd,struct keyboard_event *event);

// Be sure to free the struct pointer after use, or memory leakage would occur.
struct keyboard_event* KbdWaitEvent(struct keyboard *kbd);

#endif