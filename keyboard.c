#include "keyboard.h"

const enum keyval keymap[9][8] = {
	{KAPP,MENU,KSET,KF_1,KF_2,KF_3,KEXT,KPWR},
	{KDEL,BSLH,AGBK,TILD,SHRP,AMPR,EQUL,BKSP},
	{EXCL,KQ_W,KE_R,KT_Y,KU_I,KO_P,LPAR,RPAR},
	{LSFT,KA_S,KD_F,KG_H,KJ_K,KLSL,APTP,RSFT},
	{LCTR,K__Z,KX_C,KV_B,KN_M,COMA,SPAC,RCTR},
	{NUM7,NUM8,NUM9,MDIV,MPCT,MFRC,MOPT,APGU},
	{NUM4,NUM5,NUM6,MMLT,MPWR,MSQR,MSYB,APGD},
	{NUM1,NUM2,NUM3,MMNS,AHOM,A_UP,AEND,BACK},
	{NUM0,MDOT,MPIE,MPLS,ALFT,ADWN,ARHT,ENTR}
};

const unsigned int _rowpinsnum[] = {4,17,27,22,5,6,13,19,26};
const unsigned int _colpinsnum[] = {21,20,16,12,25,24,23,18};
const int _rowpinhigh[] = {1,1,1,1,1,1,1,1,1};
const int _rowpinlow[] = {0,0,0,0,0,0,0,0,0};
const int _colpinhigh[] = {1,1,1,1,1,1,1,1};
const int _colpinlow[] = {0,0,0,0,0,0,0,0};

const char* consumer = "CALCOS_KEYBOARD";

static int rows = 9;
static int cols = 8;
static struct gpiod_chip *chip;
static struct gpiod_line_bulk rowpins,colpins;
static sem_t event_flag;
static struct keyboard_event event_loopbuffer[KEYBOARD_EVENT_LOOP_BUFFER_LENGTH];
static int buffer_head,buffer_tail;
static pthread_t monitor_tid;
static pthread_attr_t thattr;
static int monitor_runflag;

void KbdInit() {
	buffer_head = buffer_tail = 0;
	if (sem_init(&(event_flag),0,0)) {// Initialize the event_flag semaphore in the locked state for event wait/count.
		printf("Could not initialized semaphore with error %d!\n",errno);
		return;
	}
	chip = gpiod_chip_open_by_number(0);
	if (!chip) {
		printf("Failed to open and initialize /dev/gpiochip0.");
		sem_destroy(&event_flag);
		return;
	}
	gpiod_line_bulk_init(&rowpins);
	gpiod_line_bulk_init(&colpins);
	if (gpiod_chip_get_lines(chip,_rowpinsnum,rows,&rowpins)<0 || gpiod_chip_get_lines(chip,_colpinsnum,cols,&colpins)<0) {
		printf("Failed to get gpio lines.");
		gpiod_chip_close(chip);
		sem_destroy(&event_flag);
		return;
	}
	gpiod_line_request_bulk_input_flags(&rowpins,consumer,GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN);
	gpiod_line_request_bulk_output(&colpins,consumer,_colpinhigh);
	pthread_attr_init(&thattr);
	monitor_runflag = 0;
}

void* _kbdmonitorproc(void *arg) {
	int key_r,key_c,kprv_r,kprv_c;
	int prob[9];
	kprv_r = kprv_c = -1;
	struct timespec sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_nsec = 10000;
	while (monitor_runflag){
		gpiod_line_set_config_bulk(&rowpins,GPIOD_LINE_REQUEST_DIRECTION_INPUT,GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN,NULL);
		gpiod_line_set_config_bulk(&colpins,GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,0,_colpinhigh);
		gpiod_line_get_value_bulk(&rowpins,prob);
		for(key_r=8;key_r>=0&&!prob[key_r];key_r--);
		gpiod_line_set_config_bulk(&colpins,GPIOD_LINE_REQUEST_DIRECTION_INPUT,GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP,NULL);
		gpiod_line_set_config_bulk(&rowpins,GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,0,_rowpinlow);
		gpiod_line_get_value_bulk(&colpins,prob);
		for(key_c=7;key_c>=0&&prob[key_c];key_c--);
		if (key_r!=kprv_r || key_c!=kprv_c) {
			if (kprv_r!=-1 && kprv_c!=-1) _kbdevent_append(kprv_r,kprv_c,KEYBOARD_EVENT_KEYUP);
			if (key_r!=-1 && key_c!=-1) _kbdevent_append(key_r,key_c,KEYBOARD_EVENT_KEYDOWN);
			kprv_r = key_r;
			kprv_c = key_c;
		}
		nanosleep(&sleeptime,&sleeptime);
	}
	return NULL;
}

void KbdStartMonitoring() {
	if (monitor_runflag) return;
	monitor_runflag = 1;
	pthread_create(&monitor_tid,&thattr,_kbdmonitorproc,NULL);
}

void KbdStopMonitoring() {
	if (!monitor_runflag) return;
	monitor_runflag = 0;
	pthread_join(monitor_tid,NULL);
}

void KbdDestroy() {
	KbdStopMonitoring();
	sem_destroy(&event_flag);
	gpiod_line_release_bulk(&rowpins);
	gpiod_line_release_bulk(&colpins);
	gpiod_chip_close(chip);
}

int _kbdevent_append(int key_r,int key_c,int event_type) {
	int flag_val;
	sem_getvalue(&event_flag,&flag_val);
	if (flag_val==KEYBOARD_EVENT_LOOP_BUFFER_LENGTH) return -1; // Return failure when buffer full(dropping future input until previous events consumed)
	event_loopbuffer[buffer_tail].key_r = key_r;
	event_loopbuffer[buffer_tail].key_c = key_c;
	event_loopbuffer[buffer_tail].event_type = event_type;
	buffer_tail = (buffer_tail+1)%KEYBOARD_EVENT_LOOP_BUFFER_LENGTH;
	sem_post(&event_flag);
	return 0;
}


struct keyboard_event *KbdWaitEvent() {
	sem_wait(&event_flag);
	if (buffer_tail==buffer_head) return NULL;
	return &(event_loopbuffer[buffer_head]);
}

void KbdPostEvent() {
	buffer_head = (buffer_head+1)%KEYBOARD_EVENT_LOOP_BUFFER_LENGTH;
}