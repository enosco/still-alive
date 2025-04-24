#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <ncurses.h>
#include "assets.h"

struct interval {
    int length;   
    enum {PAUSE, TYPE} cursor_action;
    
    // later add action to change page and ascii art
};

void print_lyrics(WINDOW *form_win, WINDOW *cred_win, WINDOW *img_win);
void print_img(WINDOW *img_win);

int main(int argc, char **argv) {
    int COLOR_ORANGE;
    initscr();
//    cbreak();
    start_color();

    init_color(COLOR_ORANGE, 835, 600, 80);
    init_pair(1, COLOR_ORANGE, COLOR_BLACK);
    
    WINDOW *form_frame = newwin(30, 50, 0, 0);
    WINDOW *form_win = newwin(28, 48, 1, 1);
    WINDOW *cred_win = newwin(16, 50, 0, 50);
    WINDOW *img_win = newwin(20, 40, 16, 55);
    refresh();

    wattron(form_frame, COLOR_PAIR(1));
    wattron(form_win, COLOR_PAIR(1));
    wattron(cred_win, COLOR_PAIR(1));
    wattron(img_win, COLOR_PAIR(1));
    wattron(img_win, A_BOLD);

    wborder(form_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');
    wborder(cred_win, '|', '|', '-', '-', ' ', ' ', ' ', ' ');
    box(img_win, 0,0);

    wrefresh(form_frame);
    wrefresh(form_win);
    wrefresh(cred_win);
    wrefresh(img_win);

    print_lyrics(form_win, cred_win, img_win);
	
    wattroff(form_win, COLOR_PAIR(1));
    wattroff(cred_win, COLOR_PAIR(1));
    wattroff(img_win, COLOR_PAIR(1));
    wattroff(img_win, A_BOLD);

    while (getch() != 'q') {}
    endwin();
    return 0;
    
}

void print_lyrics(WINDOW *form_win, WINDOW *cred_win, WINDOW *img_win) {    
    print_img(img_win);

    struct interval actions[] = {
	{49, TYPE}, {25, PAUSE}, {20, TYPE}, {9999, PAUSE}
    };

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    timer.it_interval = timer.it_value;  
    setitimer(ITIMER_REAL, &timer, NULL);

    int sig;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    
    sigprocmask(SIG_BLOCK, &set, NULL);

    int tick_c = 0;
    int tick_lim = 1000;
    int actions_idx = 0;
    struct interval curr = actions[actions_idx];
    
    for (int i = 0; i < tick_lim; i++) {
	if (tick_c >= curr.length) {
	    curr = actions[++actions_idx];
	    tick_c = 0;
//	    beep();
	}	
	tick_c++;

	sigwait(&set, &sig);

	if (curr.cursor_action == TYPE) {
	    waddch(form_win, FORM1[i]);
	    wrefresh(form_win);
	} else {
	    --i;
	}
    }        
}

void print_img(WINDOW *img_win) {
    for (int line = 0; line < 20; line++) {
	mvwprintw(img_win, line, 0, APERTURE[line]);	
    }
    wrefresh(img_win);
}
