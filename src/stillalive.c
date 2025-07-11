#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include "miniaudio.h"

const int USECS_IN_SEC = 1000000;
const int DEFAULT_TICKRATE_USEC = 100000;
const int FAST_TICKRATE_USEC = 75000;

const int BLINK_THRESHOLD = (USECS_IN_SEC / 4) / DEFAULT_TICKRATE_USEC;

const int CREDITS_START_LINE = 11; 
const int CREDITS_SCROLL_COUNT = 4;

/* ASCII Art */
const char *NONE = "";
const char *APERTURE =
    "             .,-:;//;:=,                "
    "         . :H@@@MM@M#H/.,+%%;,           "
    "      ,/X+ +M@@M@MM%%=,-%%HMMM@X/,        "
    "     -+@MM; $M@@MH+-,;XMMMM@MMMM@+-     "
    "    ;@M@@M- XM@X;. -+XXXXXHHH@M@M#@/.   "
    "  ,%%MM@@MH ,@%%=            .---=-=:=,.  "
    "  -@#@@@MX .,              -%%HX$$%%%%%%+;  "
    " =-./@M@M$                  .;@MMMM@MM: "
    " X@/ -$MM/                    .+MM@@@M$ "
    ",@M@H: :@:                    . -X#@@@@-"
    ",@@@MMX, .                    /H- ;@M@M="
    ".H@@@@M@+,                    %%MM+..%%#$."
    " /MMMM@MMH/.                  XM@MH; -; "
    "  /%%+%%$XHH@$=              , .H@@@@MX,  "
    "   .=--------.           -%%H.,@@@@@MX,  "
    "   .%%MM@@@HHHXX$$$%%+- .:$MMX -M@@MM%%.   "
    "     =XMMM@MM@MM#H;,-+HMM@M+ /MMMX=     "
    "       =%%@M@M#@$-.=$@MM@@@M; %%M%%=       "
    "         ,:+$+-,/H#MMMMMMM@- -,         "
    "               =++%%%%%%%%+/:-.             "
;

/* Printing Instructions & Text */

typedef struct {
    int wait_for;
    char *text;
    const char **img_ptr;
    enum {DEFAULT, FAST} speed;
} print_instr;

const print_instr FORM1[] = {
    {0,  "Forms FORM-29827281-12:\n",    &NONE},
    {0,  "Test Assessment Report\n\n\n", &NONE},
    {2,  "This was a triumph.\n",        &NONE},
    {14, "I'm making a note here:\n",    &NONE},
    {0,  "HUGE SUCCESS.\n",              &NONE},
    {15,  "It's hard to overstate\n",     &NONE},
    {2,  "my satisfaction.\n",           &NONE},
    {30,  "Aperture Science\n",           &APERTURE},
    {20,"We do what we must\n",             &APERTURE},
    {0,"because we can.\n",                &APERTURE},
    {20,"For the good of all of us.\n",     &APERTURE},
    {0,"Except the ones who are dead.\n\n",   &APERTURE, FAST},
    {0,"But there's no sense crying\n",   &APERTURE, FAST},
    {0,"over every mistake.\n",             &APERTURE, FAST},
    {0,"You just keep on trying\n",         &APERTURE},
    {0,"till you run out of cake.\n",       &APERTURE},
    {0,"And the Science gets done.\n",      &APERTURE},
    {0,"And you make a neat gun.\n",        &APERTURE},
    {0,"For the people who are\n",          &APERTURE},
    {0,"still alive.",                      &APERTURE},    
    {0,"\0"} // null char signals end of form
};

const char *CREDITS_LOOP =
    "THANK YOU FOR PARTICIPATING\n"
    "IN THIS\n"
    "ENRICHMENT CENTER ACTIVITY!!!\n";

void animate(WINDOW *form_win, WINDOW *img_win, WINDOW* cred_pad);
void fade_to_black(WINDOW *form_frame, WINDOW *form_win, WINDOW *img_win,
		   WINDOW *cred_frame, WINDOW *cred_pad);
void display_quit_prompt();
void print_img(WINDOW *img_win, const char *img);
void update_credits(WINDOW *cred_pad);
void update_cursors(WINDOW *form_win, WINDOW *cred_pad);

int main(int argc, char **argv) {

    /* INIT NCURSES */
    initscr();
    //cbreak;
    curs_set(0);
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

    //animate(form_win, img_win, cred_pad);

    //fade_to_black(form_frame, form_win, img_win, cred_frame, cred_pad);

    display_quit_prompt();
    
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
    timer.it_value.tv_usec = DEFAULT_TICKRATE_USEC; 
    timer.it_interval = timer.it_value;  
    setitimer(ITIMER_REAL, &timer, NULL);        

    const print_instr *forms[] = {FORM1, FORM1, FORM1};

    // move cursor in cred pad to correct pos
    // wmove(cred_pad, CREDITS_START_LINE, 0);
    
    for (int i = 0; i < 3; i ++) {
	const print_instr *curr_form = forms[i];

	print_instr line = *(curr_form);
	const char **curr_img = NULL;
	char *text;
	while (*(text = line.text)) {
	    	    
	    while (line.wait_for > 0) {
		sigwait(&set, &sig);
		line.wait_for--;
		update_credits(cred_pad);
		update_cursors(form_win, cred_pad);
	    }
	    
	    if (curr_img != line.img_ptr) {      
		print_img(img_win, *line.img_ptr);
		curr_img = line.img_ptr;
	    }	    
		
	    if (line.speed == FAST) {
		timer.it_value.tv_usec = FAST_TICKRATE_USEC;
		timer.it_interval = timer.it_value;
		setitimer(ITIMER_REAL, &timer, NULL);
	    }

	    while (*text) {
		sigwait(&set, &sig);	       
		pechochar(form_win, *(text++));
		update_credits(cred_pad);		
		update_cursors(form_win, cred_pad);
		wrefresh(form_win);
	    }
	    
	    line = *(++curr_form);

	    timer.it_value.tv_usec = DEFAULT_TICKRATE_USEC;
	    timer.it_interval = timer.it_value;
	    setitimer(ITIMER_REAL, &timer, NULL);
	}
	wclear(form_win);
    }
}

void fade_to_black(WINDOW *form_frame,
		   WINDOW *form_win,
		   WINDOW *img_win,
		   WINDOW *cred_frame,
		   WINDOW *cred_pad) {

    int sig;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);

    sigprocmask(SIG_BLOCK, &set, NULL);

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = DEFAULT_TICKRATE_USEC; 
    timer.it_interval = timer.it_value;  
    setitimer(ITIMER_REAL, &timer, NULL);        
      
    int FADE_COLOR = 18;
    int r = 835;
    int g = 600;
    int b = 80;

    float weight = 1.0;

    while (weight > 0) {
	sigwait(&set, &sig);
	init_color(FADE_COLOR, r*weight, g*weight, b*weight);
	init_pair(2, FADE_COLOR, COLOR_BLACK);
    
	wattron(form_frame, COLOR_PAIR(2));
	wattron(form_win, COLOR_PAIR(2));
	wattron(cred_frame, COLOR_PAIR(2));
	wattron(cred_pad, COLOR_PAIR(2));
	wattron(img_win, COLOR_PAIR(2));
	
//	wborder(form_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');
//	wborder(cred_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');    

	wrefresh(form_frame);
	wrefresh(form_win);
	
	wrefresh(cred_frame);
	wrefresh(img_win);
	
	weight -= 0.005;
    }
}

void update_cursors(WINDOW *form_win, WINDOW *cred_pad) {
    static const char cursors[2] = {'_', ' '};
    static const int blink_threshold = (USECS_IN_SEC / 4) / DEFAULT_TICKRATE_USEC;
    static int curs_i = 0;
    static int blink_counter = 0;

    int x, y;
    getyx(form_win, y, x);
    wechochar(form_win, cursors[curs_i]);
    wmove(form_win, y, x);
    wrefresh(form_win);
    
    if (blink_counter > blink_threshold) {
	blink_counter = 0;
	curs_i = curs_i ^ 1;
    } else {
	blink_counter++;
    }    
}

void update_credits(WINDOW *cred_pad) {
    static int cursor = 0;
    static int scroll_d = CREDITS_SCROLL_COUNT;

    char cred_char = CREDITS_LOOP[cursor];
    
    if (cred_char) {
	pechochar(cred_pad, cred_char);
	cursor++;
    } else {
	if (scroll_d > 0) {
	    pechochar(cred_pad, '\n');
	    scroll_d--;		    
	} else {
	    wmove(cred_pad, CREDITS_START_LINE, 0);
	    cursor = 0;
	    scroll_d = CREDITS_SCROLL_COUNT;
	}
    }
   
    prefresh(cred_pad, 0, 0, 1, 52, 14, 98);    
}

void display_quit_prompt() {
    WINDOW *end_win = newwin(8, 40, 8, 30);
    wattron(end_win, COLOR_PAIR(1));

    const char* msg = "\n   For Maya, the meow of my life :3\n"
	              "    You may now press 'q' to exit";
    mvwprintw(end_win, 1, 1, msg);
    wborder(end_win,0,0,0,0,0,0,0,0);

    wrefresh(end_win);
}

void print_img(WINDOW *img_win, const char *img_ptr) {
    wclear(img_win);
    mvwprintw(img_win, 0, 0, img_ptr);
    wrefresh(img_win);
}
