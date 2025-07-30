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

const int RED_VALUE = 835;
const int GREEN_VALUE = 600;
const int BLUE_VALUE = 80;

/* ASCII Art */
const char *NONE = "";
const char *APERTURE =
    "             .,-:;//;:=,                "
    "         . :H@@@MM@M#H/.,+%;,           "
    "      ,/X+ +M@@M@MM%=,-%HMMM@X/,        "
    "     -+@MM; $M@@MH+-,;XMMMM@MMMM@+-     "
    "    ;@M@@M- XM@X;. -+XXXXXHHH@M@M#@/.   "
    "  ,%MM@@MH ,@%=            .---=-=:=,.  "
    "  -@#@@@MX .,              -%HX$$%%%+;  "
    " =-./@M@M$                  .;@MMMM@MM: "
    " X@/ -$MM/                    .+MM@@@M$ "
    ",@M@H: :@:                    . -X#@@@@-"
    ",@@@MMX, .                    /H- ;@M@M="
    ".H@@@@M@+,                    %MM+..%#$."
    " /MMMM@MMH/.                  XM@MH; -; "
    "  /%+%$XHH@$=              , .H@@@@MX,  "
    "   .=--------.           -%H.,@@@@@MX,  "
    "   .%MM@@@HHHXX$$$%+- .:$MMX -M@@MM%.   "
    "     =XMMM@MM@MM#H;,-+HMM@M+ /MMMX=     "
    "       =%@M@M#@$-.=$@MM@@@M; %M%=       "
    "         ,:+$+-,/H#MMMMMMM@- -,         "
    "               =++%%%%+/:-.             ";

const char *SCIENCE =
    "                 =/;;/-                 "
    "                +:    //                "
    "               /;      /;               "
    "              -X        H.              "
    ".//;;;:;;-,   X=        :+   .-;:=;:;%;."
    "M-       ,=;;;#:,      ,:#;;:=,       ,@"
    ":%           :%.=/++++/=.$=           %="
    " ,%;         %/:+/;,,/++:+/         ;+. "
    "   ,+/.    ,;@+,        ,%H;,    ,/+,   "
    "      ;+;;/= @.  .H##X   -X :///+;      "
    "      ;+=;;;.@,  .XM@$.  =X.//;=%/.     "
    "   ,;:      :@%=        =$H:     .+%-   "
    " ,%=         %;-///==///-//         =%, "
    ";+           :%-;;;;;;;;-X-           +:"
    "@-      .-;;;;M-        =M/;;;-.      -X"
    " :;;::;;-.    %-        :+    ,-;;-;:== "
    "              ,X        H.              "
    "               ;/      %=               "
    "                //    +;                "
    "                 ,////,                 ";

const char *HEARTBREAK =
    "                          .,---.        "
    "                        ,/XM#MMMX;,     "
    "                      -%##########M%,   "
    "                     -@######%  $###@=  "
    "      .,--,         -H#######$   $###M: "
    "   ,;$M###MMX;     .;##########$;HM###X="
    ",/@###########H=      ;################+"
    "-+#############M/,      %##############+"
    "%M###############=      /##############:"
    "H################      .M#############;."
    "@###############M      ,@###########M:. "
    "X################,      -$=X#######@:   "
    "/@##################%-     +######$-    "
    ".;##################X     .X#####+,     "
    " .;H################/     -X####+.      "
    "   ,;X##############,       .MM/        "
    "      ,:+$H@M#######M#$-    .$$=        "
    "           .,-=;+$@###X:    ;/=.        "
    "                  .,/X$;   .::,         "
    "                      .,    ..          ";

const char *BOOM =
    "            .+                          "
    "             /M;                        "
    "              H#@:              ;,      "
    "              -###H-          -@/       "
    "               %####$.  -;  .%#X        "
    "                M#####+;#H :M#M.        "
    "..          .+/;%#############-         "
    " -/%H%+;-,    +##############/          "
    "    .:$M###MH$%+############X  ,--=;-   "
    "        -/H#####################H+=.    "
    "           .+#################X.        "
    "         =%M####################H;.     "
    "           /@###############+;;/%%;,    "
    "         -%###################$         "
    "       ;H######################M=       "
    "    ,%#####MH$%;+#####M###-/@####%      "
    "  :$H%+;=-      -####X.,H#   -+M##@-    "
    " .              ,###;    ;      =$##+   "
    "                .#H,               :XH, "
    "                 +                   .;-";

const char *FIRE =
    "                      -$-               "
    "                    .H##H,              "
    "                   +######+             "
    "                .+#########H.           "
    "              -$############@.          "
    "            =H###############@  -X:     "
    "          .$##################:  @#@-   "
    "     ,;  .M###################;  H###;  "
    "   ;@#:  @###################@  ,#####: "
    " -M###.  M#################@.  ;######H "
    " M####-  +###############$   =@#######X "
    " H####$   -M###########+   :#########M, "
    "  /####X-   =########%   :M########@/.  "
    "    ,;%H@X;   .$###X   :##MM@%+;:-      "
    "                 ..                     "
    "  -/;:-,.              ,,-==+M########H "
    " -##################@HX%%+%%$%%%+:,,    "
    "    .-/H%%%+%%$H@###############M@+=:/+:"
    "/XHX%:#####MH%=    ,---:;;;;/&&XHM,:###$"
    "$@#MX %+;-                           .  ";

const char *CHECK =
    "                                     :X-"
    "                                  :X### "
    "                                ;@####@ "
    "                              ;M######X "
    "                            -@########$ "
    "                          .$##########@ "
    "                         =M############-"
    "                        +##############$"
    "                      .H############$=. "
    "         ,/:         ,M##########M;.    "
    "      -+@###;       =##########M;       "
    "   =%M#######;     :#########M/         "
    "-$M###########;   :########/            "
    " ,;X###########; =#######$.             "
    "     ;H#########+######M=               "
    "       ,+#############+                 "
    "          /M########@-                  "
    "            ;M#####%                    "
    "              +####:                    "
    "               ,$M-                     ";


const char *BLACKMESA = 
    "           .-;+$XHHHHHHX$+;-.           "
    "        ,;X@@X%/;=----=:/%X@@X/,        "
    "      =$@@%=.              .=+H@X:      "
    "    -XMX:                      =XMX=    "
    "   /@@:                          =H@+   "
    "  %@X,                            .$@$  "
    " +@X.                               $@% "
    "-@@,                                .@@="
    "%@%                                  +@$"
    "H@:                                  :@H"
    "H@:         :HHHHHHHHHHHHHHHHHHX,    =@H"
    "%@%         ;@M@@@@@@@@@@@@@@@@@H-   +@$"
    "=@@,        :@@@@@@@@@@@@@@@@@@@@@= .@@:"
    " +@X        :@@@@@@@@@@@@@@@M@@@@@@:%@% "
    "  $@$,      ;@@@@@@@@@@@@@@@@@M@@@@@@$. "
    "   +@@HHHHHHH@@@@@@@@@@@@@@@@@@@@@@@+   "
    "    =X@@@@@@@@@@@@@@@@@@@@@@@@@@@@X=    "
    "      :$@@@@@@@@@@@@@@@@@@@M@@@@$:      "
    "        ,;$@@@@@@@@@@@@@@@@@@X/-        "
    "           .-;+$XXHHHHHX$+;-.           ";

const char *LIE = 
    "            ,:/+/-                      "
    "            /M/              .,-=;//;-  "
    "       .:/= ;MH/,    ,=/+%$XH@MM#@:     "
    "      -$##@+$###@H@MMM#######H:.    -/H#"
    " .,H@H@ X######@ -H#####@+-     -+H###@X"
    "  .,@##H;      +XM##M/,     =%@###@X;-  "
    "X%-  :M##########$.    .:%M###@%:       "
    "M##H,   +H@@@$/-.  ,;$M###@%,          -"
    "M####M=,,---,.-%%H####M$:          ,+@##"
    "@##################@/.         :%H##@$- "
    "M###############H,         ;HM##M$=     "
    "#################.    .=$M##M$=         "
    "################H..;XM##M$=          .:+"
    "M###################@%=           =+@MH%"
    "@#################M/.         =+H#X%=   "
    "=+M###############M,      ,/X#H+:,      "
    "  .;XM###########H=   ,/X#H+:;          "
    "     .=+HM#######M+/+HM@+=.             "
    "         ,:/%XM####H/.                  "
    "              ,.:=-.                    ";

const char *NUCLEAR = 
    "             =+$HM####@H%;,             "
    "          /H###############M$,          "
    "          ,@################+           "
    "           .H##############+            "
    "             X############/             "
    "              $##########/              "
    "               %########/               "
    "                /X/;;+X/                "
    "                                        "
    "                 -XHHX-                 "
    "                ,######,                "
    "#############X  .M####M.  X#############"
    "##############-   -//-   -##############"
    "X##############%,      ,+##############X"
    "-##############X        X##############-"
    " %############%          %############% "
    "  %##########;            ;##########%  "
    "   ;#######M=              =M#######;   "
    "    .+M###@,                ,@###M+.    "
    "       :XH.                  .HX:       ";

const char* GLaDOS =
    "       #+ @      # #              M#@   "
    " .    .X  X.%##@;# #   +@#######X. @H%  "
    "   ,==.   ,######M+  -#####%M####M-    #"
    "  :H##M%:=##+ .M##M,;#####/+#######% ,M#"
    " .M########=  =@#@.=#####M=M#######=  X#"
    " :@@MMM##M.  -##M.,#######M#######. =  M"
    "             @##..###:.    .H####. @@ X,"
    "   ############: ###,/####;  /##= @#. M "
    "           ,M## ;##,@#M;/M#M  @# X#% X# "
    ".%=   ######M## ##.M#:   ./#M ,M #M ,#$ "
    "##/         $## #+;#: #### ;#/ M M- @# :"
    "#+ #M@MM###M-;M #:$#-##$H# .#X @ + $#. #"
    "      ######/.: #%=# M#:MM./#.-#  @#: H#"
    "+,.=   @###: /@ %#,@  ##@X #,-#@.##% .@#"
    "#####+;/##/ @##  @#,+       /#M    . X, "
    "   ;###M#@ M###H .#M-     ,##M  ;@@; ###"
    "   .M#M##H ;####X ,@#######M/ -M###$  -H"
    "    .M###%  X####H  .@@MM@;  ;@#M@      "
    "      H#M    /@####/      ,++.  / ==-,  "
    "               ,=/:, .+X@MMH@#H  #####$=";
/* Printing Instructions & Text */

typedef struct {
    int wait_for;
    char *text;
    const char **img_ptr;
    enum {DEFAULT, FAST} speed;
} print_instr;

const print_instr FORM1[] = {
    {0,  "Forms FORM-29827281-12:\n",         &NONE},
    {0,  "Test Assessment Report\n\n\n",      &NONE},
    {2,  "This was a triumph.\n",             &NONE},
    {14, "I'm making a note here:\n",         &NONE},
    {0,  "HUGE SUCCESS.\n",                   &NONE},
    {15, "It's hard to overstate\n",          &NONE},
    {0,  "my satisfaction.\n",                &NONE},
    {30, "Aperture Science\n",                &APERTURE},
    {20, "We do what we must\n",              &APERTURE},
    {0,  "because we can.\n",                 &APERTURE},
    {20, "For the good of all of us.\n",      &APERTURE},
    {0,  "Except the ones who are dead.\n\n", &APERTURE, FAST},
    {3,  "But there's no sense crying\n",     &APERTURE, FAST},
    {0,  "over every mistake.\n",             &APERTURE, FAST},
    {5,  "You just keep on trying\n",         &APERTURE, FAST},
    {0,  "till you run out of cake.\n",       &APERTURE, FAST},
    {5,  "And the Science gets done.\n",      &SCIENCE,  FAST},
    {0,  "And you make a neat gun.\n",        &SCIENCE,  FAST},
    {0,  "For the people who are\n",          &APERTURE, FAST},
    {0,  "still alive.",                      &APERTURE},    
    {10, "\0"} // null char signals end of form
};

const print_instr FORM2[] = {
    {0, "Forms FORM-55551-5:\n", &APERTURE, FAST},
    {0, "Personnel File Addendum:\n\n", &APERTURE, FAST},
    {0, "Dear <<Subject Name Here>>,\n\n", &APERTURE},
    {0, "I'm not even angry.\n", &APERTURE},
    {0, "I'm being so sincere right now.\n", &APERTURE},
    {0, "Even though you broke my heart.\n", &APERTURE},
    {0, "And killed me.\n", &APERTURE},
    {0, "And tore me to pieces.\n", &APERTURE},
    {0, "And threw every piece into a fire.\n", &APERTURE},
    {0, "As they burned it hurt because\n", &APERTURE},
    {0, "I was so happy for you!\n", &APERTURE},
    {0, "Now these points of data\n", &APERTURE},
    {0, "make a beautiful line.\n", &APERTURE},
    {0, "And we're out of beta.\n", &APERTURE},
    {0, "We're releasing on time.\n", &APERTURE},
    {0, "So I'm GLaD. I got burned.\n", &APERTURE},
    {0, "Think of all the things we learned\n", &APERTURE},
    {0, "for the people who are\n", &APERTURE},
    {0, "still alive.\n", &APERTURE},
    {0, "\0"}
};

const print_instr FORM3[] = {
    {0, "Forms FORM-55551-6:\n", &APERTURE},
    {0, "Personnel File Addendum Addendum:\n\n", &APERTURE},
    {0, "One last thing:\n\n", &APERTURE},
    {0, "Go ahead and leave me.\n", &APERTURE},
    {0, "I think I prefer to stay inside.\n", &APERTURE},
    {0, "Maybe you'll find someone else\n", &APERTURE},
    {0, "to help you.\n", &APERTURE},
    {0, "Maybe Black Mesa...\n", &APERTURE},
    {0, "THAT WAS A JOKE. HA HA. FAT CHANCE.\n", &APERTURE},
    {0, "Anyway, this cake is great.\n", &APERTURE},
    {0, "It's so delicious and moist.\n", &APERTURE},
    {0, "Look at me still talking\n", &APERTURE},
    {0, "when there's Science to do.\n", &APERTURE},
    {0, "When I look out there,\n", &APERTURE},
    {0, "it makes me GLaD I'm not you.\n", &APERTURE},
    {0, "I've experiments to run.\n", &APERTURE},
    {0, "There is research to be done.\n", &APERTURE},
    {0, "On the people who are\n", &APERTURE},
    {0, "still alive.\n", &APERTURE},
    {0, "\0"}
};

const print_instr FORM4[] = {
    {0, "\n\n\n", &APERTURE, FAST},
    {0, "PS: And believe me I am\n", &APERTURE},
    {0, "still alive.\n", &APERTURE},
    {0, "PPS: I'm doing Science and I'm\n", &APERTURE},
    {0, "still alive.\n", &APERTURE},
    {0, "PPPS: I feel FANTASTIC and I'm\n", &APERTURE},
    {0, "still alive.\n\n", &APERTURE},
    {0, "FINAL THOUGHT:\n", &APERTURE},
    {0, "While you're dying I'll be\n", &APERTURE},
    {0, "still alive.\n\n", &APERTURE},
    {0, "FINAL THOUGHT PS:\n", &APERTURE},
    {0, "And when you're dead I will be\n", &APERTURE},
    {0, "still alive.\n\n\n", &APERTURE},
    {0, "STILL ALIVE\n", &APERTURE},
    {0, "\0"}
};

// maybe unnecessary since fade_to_black keeps the cursor blinking 
const print_instr BLANK_FORM[] = {
    {0, "\0", &APERTURE, FAST},
};

const char *CREDITS_LOOP =
    "THANK YOU FOR PARTICIPATING\n"
    "IN THIS\n"
    "ENRICHMENT CENTER ACTIVITY!!!\n";

void animate(WINDOW *form_win, WINDOW *img_win, WINDOW* cred_pad);
void fade_to_black(WINDOW *form_frame, WINDOW *form_win, WINDOW *img_win,
		   WINDOW *cred_frame, WINDOW *cred_pad);
//void display_quit_prompt();
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
//    init_color(COLOR_ORANGE, 835, 600, 80);
    init_color(COLOR_ORANGE, RED_VALUE, GREEN_VALUE, BLUE_VALUE);
    init_color(COLOR_BLACK, 0,0,0);
    init_pair(1, COLOR_ORANGE, COLOR_BLACK);
    
    WINDOW *form_frame = newwin(36, 50, 0, 0);
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

    /* START TYPING */	    
    animate(form_win, img_win, cred_pad);
    
    // start fading once above function is finished
    fade_to_black(form_frame, form_win, img_win, cred_frame, cred_pad);

    // display_quit_prompt();
    
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

    const print_instr *forms[] = {FORM1, FORM2, FORM3, FORM4};

    // move cursor in cred pad to correct pos
     wmove(cred_pad, CREDITS_START_LINE, 0);
    
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

    float weight = 1.0;
    while (weight > 0) {
	sigwait(&set, &sig);

	// multiplying rgb values by weight
	// progressively darkens the original orange
	//init_color(FADE_COLOR, r*weight, g*weight, b*weight);
	init_color(FADE_COLOR, RED_VALUE*weight, GREEN_VALUE*weight, BLUE_VALUE*weight);
	init_pair(2, FADE_COLOR, COLOR_BLACK);
	
	// apply updated color to everything
	wattron(form_frame, COLOR_PAIR(2));
	wattron(form_win, COLOR_PAIR(2));
	wattron(cred_frame, COLOR_PAIR(2));
	wattron(cred_pad, COLOR_PAIR(2));
	wattron(img_win, COLOR_PAIR(2));
	
	// should i have to reprint the art every time to update the color?
	print_img(img_win, APERTURE);

	update_credits(cred_pad);
	update_cursors(form_win, cred_pad);
	
	// redraw borders to update their color too
	wborder(form_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');
	wborder(cred_frame, '|', '|', '-', '-', ' ', ' ', ' ', ' ');    
	
	mvwprintw(form_win, 0,0, "%f", weight);
	
	wrefresh(img_win);
	wrefresh(form_frame);
	wrefresh(form_win);
	wrefresh(cred_frame);

	
	weight -= 0.005;
    }
}

void update_cursors(WINDOW *form_win, WINDOW *cred_pad) {
    static const char cursors[2] = {'_', ' '};
//    static const int blink_threshold = (USECS_IN_SEC / 4) / DEFAULT_TICKRATE_USEC;
    static int curs_i = 0;
    static int blink_counter = 0;
    
    int x, y;

    // update cursor for form
    getyx(form_win, y, x);

    wechochar(form_win, cursors[curs_i]);
    wmove(form_win, y, x);
    wrefresh(form_win);


    // TODO: Fix stray underscore that gets left
    // behind when newlines are printed 
    
    // update cursor for credits
    getyx(cred_pad, y, x);
    
    wechochar(cred_pad, cursors[curs_i ^ 1]);
    wmove(cred_pad, y, x);
    prefresh(cred_pad, 0, 0, 1, 52, 14, 98);
    
    if (blink_counter > BLINK_THRESHOLD) {
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

/*
void display_quit_prompt() {
    WINDOW *end_win = newwin(8, 40, 8, 30);
    wattron(end_win, COLOR_PAIR(1));

    const char* msg = "\n   For Maya, the meow of my life :3\n"
	              "    You may now press 'q' to exit!";
    mvwprintw(end_win, 1, 1, msg);
    wborder(end_win,0,0,0,0,0,0,0,0);

    wrefresh(end_win);
}
*/

void print_img(WINDOW *img_win, const char *img_ptr) {
    wclear(img_win);
    mvwprintw(img_win, 0, 0, "%s", img_ptr);
    wrefresh(img_win);
}
