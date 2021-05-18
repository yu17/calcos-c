#include "sys_IM.h"

static enum keyval input_key,prev_key;
static char prev_opt;// Change between 0/2 indicating primary or secondary character.
static char prev_shift;// Save the shift status for the pending input.
static sem_t pending_flag;
static char IMmainloop_runflag = 0;
static pthread_t IMmainloop_t;
static pthread_attr_t thattr;

void (*appcharhandler)(int,char) = NULL;

// First dimension represents shift and secondary flag. Shift is on bit 0 and secondary is on bit 1. Thus,
// 0 - Non-shifted primary char
// 1 - Shifted primary char
// 2 - Non-shifted secondary char
// 3 - Shifted secondary char
static const char keychar[4][46] = {
	{// Non-shifted primary char
		'q','e','t','u','o',
		'a','d','g','j','l',
		'x','v','n',
		'.','?','\\','<','~','#','&','=',
		'z','(',')','\'',' ',
		'0','1','2','3','4','5','6','7','8','9','.',
		'\227','+','-','*','/',
		'%','/','^','\251'
	},
	{// Shifted primary char
		'Q','E','T','U','O',
		'A','D','G','J','L',
		'X','V','N',
		',','!','|','>','`','@','$','_',
		'Z','(',')','\'',' ',
		'0','1','2','3','4','5','6','7','8','9','.',
		'\227','+','-','*','/',
		'%','/','^','\251'
	},
	{// Non-shifted secondary char
		'w','r','y','i','p',
		's','f','h','k',';',
		'c','b','m',
		'.','?','\\','<','~','#','&','=',
		'z','(',')','\'',' ',
		'0','1','2','3','4','5','6','7','8','9','.',
		'\227','+','-','*','/',
		'%','/','^','\251'
	},
	{// Shifted secondary char
		'W','R','Y','I','P',
		'S','F','H','K',';',
		'C','B','M',
		',','!','|','>','`','@','$','_',
		'Z','(',')','\'',' ',
		'0','1','2','3','4','5','6','7','8','9','.',
		'\227','+','-','*','/',
		'%','/','^','\251'
	}
};

void SysIMInit() {
	sem_init(&pending_flag,0,0);
	prev_shift = 0;
	pthread_attr_init(&thattr);
	IMmainloop_runflag = 1;
	input_key = prev_key = KNIL;
	pthread_create(&IMmainloop_t,&thattr,_immainproc,NULL);
}

void SysIMTerm() {
	IMmainloop_runflag = 0;
	sem_post(&pending_flag);
	pthread_join(IMmainloop_t,NULL);
	sem_destroy(&pending_flag);
	appcharhandler = NULL;
}

//Also resets IM
static inline void IMSetCharHandler(void (*charhandler)(int,char)) {
	if (IMmainloop_runflag) {
		input_key = KNIL;
		sem_post(&pending_flag);
	}
	else SysIMInit();
	appcharhandler = charhandler;
}

static inline void IMKeyPress(enum keyval key) {
	if (appcharhandler) {
		input_key = key;
		sem_post(&pending_flag);
	}
}

// Using POSIX time functions here to get system time, so as to have the return type match the semaphore timewait function.

void *_immainproc(void *arg) {
	struct timespec ts;
	while (IMmainloop_runflag) {
		sem_wait(&pending_flag);
		if (!IMmainloop_runflag) break;
		if (input_key >= MOPT) continue;
		if (input_key >=K__Z) {
			appcharhandler(IM_NONHYBRID,keychar[shift_flag][input_key]);
			input_key = KNIL;
			continue;
		}
		prev_key = input_key;
		prev_shift = shift_flag;
		input_key = KNIL;
		prev_opt = 0;
		appcharhandler(IM_PENDING,keychar[prev_opt|prev_shift][prev_key]);
		if (clock_gettime(CLOCK_REALTIME,&ts)) {
			appcharhandler(IM_DETERMINED,keychar[prev_opt|prev_shift][prev_key]);
			shift_flag = 0;
			continue;
		}
		ts.tv_sec += 1;
		while (1) {
			if (sem_timedwait(&pending_flag,&ts)) {
				appcharhandler(IM_DETERMINED,keychar[prev_opt|prev_shift][prev_key]);
				shift_flag = 0;
				break;
			}
			else {
				if (input_key==prev_key) {
					prev_opt ^= 2;
					appcharhandler(IM_SWITCH,keychar[prev_opt|prev_shift][prev_key]);
					if (clock_gettime(CLOCK_REALTIME,&ts)) break;
					ts.tv_sec += 1;
				}
				else {
					appcharhandler(IM_DETERMINED,keychar[prev_opt|prev_shift][prev_key]);
					shift_flag = 0;
					sem_post(&pending_flag);
					break;
				}
			}
		}
	}
}