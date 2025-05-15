#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include "miniaudio.h"
#include "assets.h"

typedef struct {
    enum {TYPE, PAUSE} action;
    int remaining; // only relevant for PAUSE actions
//  int target_img;
} anim_instr;

void animate(WINDOW *form_win, WINDOW *img_win, WINDOW* cred_pad);
void print_img(WINDOW *img_win);

int main(int argc, char **argv) {

    /* INIT NCURSES */
    initscr();
    //cbreak;
    //curs_set(0);
    start_color();
    
    int COLOR_ORANGE = 16;
    init_color(COLOR_ORANGE, 835, 600, 80);
    init_color(COLOR_BLACK, 0,0,0);
    init_pair(1, COLOR_ORANGE, COLOR_BLACK);
    
    WINDOW *form_frame = newwin(30, 50, 0, 0);
    WINDOW *form_win = newwin(28, 47, 1, 2);
   
    WINDOW *cred_frame = newwin(16, 50, 0, 50);
    WINDOW *cred_pad = newpad(14, 48);

    scrollok(cred_pad, TRUE);
    
    WINDOW *img_win = newwin(20, 40, 16, 55);
    
    wattron(form_frame, COLOR_PAIR(1));
    wattron(form_win, COLOR_PAIR(1));
    wattron(cred_frame, COLOR_PAIR(1));
    wattron(cred_pad, COLOR_PAIR(1));
    wattron(img_win, COLOR_PAIR(1));

    wborder(form_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');
    wborder(cred_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');    

    wrefresh(form_frame);
    wrefresh(form_win);
    
    wrefresh(cred_frame);
    wrefresh(img_win);

    /* START AUDIO */

    ma_engine engine;
    ma_result result;
    ma_sound sound;

    ma_engine_init(NULL, &engine);
    ma_sound_init_from_file(&engine, "stillalive.mp3", 0, NULL, NULL, &sound);

    ma_sound_set_volume(&sound, 0.5);
    ma_sound_set_start_time_in_milliseconds(&sound, 5000);
    ma_sound_start(&sound);

    print_img(img_win);
    animate(form_win, img_win, cred_pad);

    while (getchar() != 'q') {}
    
    endwin();
    pthread_exit(NULL);
    return 0;
}


void animate(WINDOW *form_win, WINDOW *img_win, WINDOW* cred_pad) {
    int sig;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);

    sigprocmask(SIG_BLOCK, &set, NULL);

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;    
    timer.it_interval = timer.it_value;  
    setitimer(ITIMER_REAL, &timer, NULL);        

    const char *creds = CREDITS_LOOP;   
    const int START_LINE = 11;
    const int SCROLL_C = 4;
    int scroll_d = SCROLL_C;
   
    const char *forms[] = {FORM1, FORM2, FORM3, FORM4};
    
    int tick_c = 0;
    int script_i = 0;
    
    const anim_instr script[] = {
	{TYPE, -1}, {PAUSE, 999}
    };
    anim_instr curr_instr = script[script_i];
    
    wmove(cred_pad, START_LINE, 0);
    for (int i = 0; i < 4; i++) {
	const char *curr_form = forms[i];
	
	while (*curr_form) {
	    sigwait(&set, &sig);


	    /* CREDITS WINDOW */
	    if (*creds) {
		pechochar(cred_pad, *(creds++));
	    } else {
		if (scroll_d > 0) {
		    pechochar(cred_pad, '\n');
		    scroll_d--;		    
		} else {
		    wmove(cred_pad, START_LINE, 0);
		    creds = CREDITS_LOOP;
		    scroll_d = SCROLL_C;
		}
	    }

	    /* LYRICS WINDOW */
	    char lyr_c = *(curr_form++);

	    if (lyr_c == ';' || curr_instr.remaining == 0) {
		curr_instr = script[++script_i];
	    }
	    
	    if (curr_instr.action == TYPE) {
		waddch(form_win, lyr_c);
	    } else {
		curr_instr.remaining--;
	    }
	   
	    
	    prefresh(cred_pad, 0, 0, 1, 52, 14, 98);	    
	    wrefresh(form_win);		    

	}
	wclear(form_win);		
    }
}
    
void print_img(WINDOW *img_win) {
    for (int line = 0; line < 20; line++) {
	mvwprintw(img_win, line, 0, APERTURE[line]);	
    }
    wrefresh(img_win);
}
