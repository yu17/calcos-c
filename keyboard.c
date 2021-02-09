#include "keyboard.h"

unsigned int _rowpinsnum[] = {4,17,27,22,5,6,13,19,26};
unsigned int _colpinsnum[] = {21,20,16,12,25,24,23,18};
const int _rowpinhigh[] = {1,1,1,1,1,1,1,1,1};
const int _rowpinlow[] = {0,0,0,0,0,0,0,0,0};
const int _colpinhigh[] = {1,1,1,1,1,1,1,1};
const int _colpinlow[] = {0,0,0,0,0,0,0,0};

const char* consumer = "CALCOS_KEYBOARD";

struct keyboard* KbdInit(){
	struct keyboard *kbd = malloc(sizeof(struct keyboard));
	kbd->event_buffer = NULL;
	kbd->event_flag = malloc(sizeof(sem_t));
	kbd->event_access = malloc(sizeof(sem_t));
	if (sem_init(kbd->event_flag,0,0) || sem_init(kbd->event_access,0,1)) {// Initialize the event_flag semaphore in the locked state for event wait/count.
		printf("Could not initialized semaphore with error %d!\n",errno);
		sem_destroy(kbd->event_flag);
		sem_destroy(kbd->event_access);
		free(kbd);
		return NULL;
	}

	kbd->rows = 9;
	kbd->cols = 8;
	kbd->chip = gpiod_chip_open_by_number(0);
	if (!kbd->chip) {
		printf("Failed to open and initialize /dev/gpiochip0.");
		sem_destroy(kbd->event_flag);
		sem_destroy(kbd->event_access);
		free(kbd);
		return NULL;
	}
	kbd->rowpins = malloc(sizeof(struct gpiod_line_bulk));
	kbd->colpins = malloc(sizeof(struct gpiod_line_bulk));
	gpiod_line_bulk_init(kbd->rowpins);
	gpiod_line_bulk_init(kbd->colpins);
	if (gpiod_chip_get_lines(kbd->chip,_rowpinsnum,kbd->rows,kbd->rowpins)<0 || gpiod_chip_get_lines(kbd->chip,_colpinsnum,kbd->cols,kbd->colpins)<0) {
		printf("Failed to get gpio lines.");
		free(kbd->rowpins);
		free(kbd->colpins);
		gpiod_chip_close(kbd->chip);
		sem_destroy(kbd->event_flag);
		sem_destroy(kbd->event_access);
		free(kbd);
		return NULL;
	}
	gpiod_line_request_bulk_input_flags(kbd->rowpins,consumer,GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN);
	gpiod_line_request_bulk_output(kbd->colpins,consumer,_colpinhigh);

	pthread_attr_init(&(kbd->thattr));
	kbd->monitor_runflag = 0;
	return kbd;
}

void* _kbdmonitorproc(void *arg){
	struct keyboard *kbd = (struct keyboard *)arg;
	int key_r,key_c,kprv_r,kprv_c;
	int prob[9];
	kprv_r = kprv_c = -1;
	struct keyboard_event *newev;
	while (kbd->monitor_runflag){
		gpiod_line_set_config_bulk(kbd->rowpins,GPIOD_LINE_REQUEST_DIRECTION_INPUT,GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN,NULL);
		gpiod_line_set_config_bulk(kbd->colpins,GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,0,_colpinhigh);
		gpiod_line_get_value_bulk(kbd->rowpins,prob);
		for(key_r=8;key_r>=0&&!prob[key_r];key_r--);
		gpiod_line_set_config_bulk(kbd->colpins,GPIOD_LINE_REQUEST_DIRECTION_INPUT,GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP,NULL);
		gpiod_line_set_config_bulk(kbd->rowpins,GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,0,_rowpinlow);
		gpiod_line_get_value_bulk(kbd->colpins,prob);
		for(key_c=7;key_c>=0&&prob[key_c];key_c--);
		if (key_r!=kprv_r || key_c!=kprv_c) {
			if (kprv_r!=-1 && kprv_c!=-1) {
				newev = malloc(sizeof(struct keyboard_event));
				newev->event_type = KEYBOARD_EVENT_KEYUP;
				newev->key_r = kprv_r;
				newev->key_c = kprv_c;
				newev->next = NULL;
				_kbdevent_append(kbd,newev);
			}
			if (key_r!=-1 && key_c!=-1) {
				newev = malloc(sizeof(struct keyboard_event));
				newev->event_type = KEYBOARD_EVENT_KEYDOWN;
				newev->key_r = key_r;
				newev->key_c = key_c;
				newev->next = NULL;
				_kbdevent_append(kbd,newev);
			}
			kprv_r = key_r;
			kprv_c = key_c;
		}
	}
	return NULL;
}

void KbdStartMonitoring(struct keyboard *kbd){
	if (kbd->monitor_runflag) return;
	kbd->monitor_runflag = 1;
	pthread_create(&(kbd->monitor_tid),&(kbd->thattr),_kbdmonitorproc,(void*)kbd);
}

void KbdStopMonitoring(struct keyboard *kbd){
	if (!kbd->monitor_runflag) return;
	kbd->monitor_runflag = 0;
	pthread_join(kbd->monitor_tid,NULL);
	kbd->monitor_runflag = 0;
	return;
}

void KbdDestroy(struct keyboard *kbd){
	KbdStopMonitoring(kbd);
	sem_destroy(kbd->event_flag);
	sem_destroy(kbd->event_access);
	gpiod_line_release_bulk(kbd->rowpins);
	gpiod_line_release_bulk(kbd->colpins);
	gpiod_chip_close(kbd->chip);
	free(kbd->rowpins);
	free(kbd->colpins);
}

void _kbdevent_append(struct keyboard *kbd,struct keyboard_event *event){
	sem_wait(kbd->event_access);
	if (!kbd->event_buffer) kbd->event_buffer = event;
	else {
		struct keyboard_event *pt = kbd->event_buffer;
		while (pt->next) pt = pt->next;
		pt->next = event;
	}
	sem_post(kbd->event_access);
	sem_post(kbd->event_flag);
}

// Be sure to free the struct pointer after use, or memory leakage would occur.
struct keyboard_event* KbdWaitEvent(struct keyboard *kbd){
	sem_wait(kbd->event_flag);
	sem_wait(kbd->event_access);
	struct keyboard_event *pt = kbd->event_buffer;
	kbd->event_buffer = pt->next;
	sem_post(kbd->event_access);
	return pt;
}
