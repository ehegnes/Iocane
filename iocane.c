
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <string.h>

static Display *dpy;
static int scr, sw, sh;
static Window root;
static Bool running = True;

typedef struct {
	KeySym keysym;
	void (*func)(const char *);
	const char *arg;
} Key;

static void move(const char *arg) {
	if (arg[0] == 'l') XWarpPointer(dpy,None,None,0,0,0,0,-10,0);
	if (arg[0] == 'r') XWarpPointer(dpy,None,None,0,0,0,0,10,0);
	if (arg[0] == 'd') XWarpPointer(dpy,None,None,0,0,0,0,0,10);
	if (arg[0] == 'u') XWarpPointer(dpy,None,None,0,0,0,0,0,-10);
}

static void press(const char *arg) {
	XEvent ev;
	memset(&ev, 0x00, sizeof(ev));
	usleep(100000);
	ev.type = ButtonPress;
	if (arg[0] == '1') ev.xbutton.button = 1;
	else if (arg[0] == '2')  ev.xbutton.button = 2;
	else if (arg[0] == '3')  ev.xbutton.button = 3;
	else if (arg[0] == 'u')  ev.xbutton.button = 4;
	else if (arg[0] == 'd')  ev.xbutton.button = 5;
	else if (arg[0] == 'l')  ev.xbutton.button = 6;
	else if (arg[0] == 'r')  ev.xbutton.button = 7;
	else return;
	ev.xbutton.same_screen = True;
	XQueryPointer(dpy,root,&ev.xbutton.root,&ev.xbutton.window,&ev.xbutton.x_root,&ev.xbutton.y_root,&ev.xbutton.x,&ev.xbutton.y,&ev.xbutton.state);
	ev.xbutton.subwindow = ev.xbutton.window;
	while(ev.xbutton.subwindow) {
		ev.xbutton.window = ev.xbutton.subwindow;
		XQueryPointer(dpy,ev.xbutton.window,&ev.xbutton.root,&ev.xbutton.subwindow,&ev.xbutton.x_root,&ev.xbutton.y_root,&ev.xbutton.x,&ev.xbutton.y,&ev.xbutton.state);
	}
	XSendEvent(dpy,PointerWindow,True,0xfff,&ev);
	XFlush(dpy);
	usleep(100000);
	ev.type = ButtonRelease;
	ev.xbutton.state = 0x400;
	XSendEvent(dpy,PointerWindow, True, 0xfff, &ev);
	XFlush(dpy);
}

static void quit(const char *arg) {
	running = False;
}

static Key keys[] = {
	{ XK_h,		move,	"left"	}, { XK_l,		move,	"right"	}, { XK_j,		move,	"down"	}, { XK_k,		move,	"up"	},
	{ XK_Left,	move,	"left"	}, { XK_Right,	move,	"right"	}, { XK_Down,	move,	"down"	}, { XK_Up,		move,	"up"	},
	{ XK_1,		press,	"1"		}, { XK_2,		press,	"2"		}, { XK_3,		press,	"3"		},
	{ XK_Prior,	press,	"up"	}, { XK_Next,	press,	"down"	},
	{ XK_4,		press,	"up"	}, { XK_5,		press,	"down"	}, { XK_6,		press,	"left"	}, { XK_7,		press,	"right"	},
	{ XK_q,		quit,	NULL	},
};

static void usage() {
	const char use[] = "\n\
\033[1mIOCANE\033[0m\n\
  \"The colorless, oderless, tasteless poision that will elimiate\n\
   your system's rodent infestation\"\n\n\
  Copyright 2012, Jesse McClure    License: GPLv3\n\n\
\033[1mUSAGE\033[0m\n\
iocane [ -h | powder | move (up|down|left|right) | button <button> | <x> <y> [<button>] ]\n\n\
    -h        Show this help menu\n\
    p[owder]  Move the mouse cursor off screen\n\
    m[ove]    Move the mouse cursor in the selected direction\n\
    b[utton]  Simulate a click of mouse button <button>\n\
                button can be 1,2, or 3 for primary buttons, or\n\
                up, down, left, or right for scroll buttons\n\
    <x> <y>   Move the mouse cursor to coordinates provided by <x> and <y>\n\
                optionally simulate button press of <button> at that location\n\
    [NO ARG]  With no argument, iocane runs in interactive mode\n\
                see man page for details\n";
	printf("%s\n",use);
}

int main(int argc, const char **argv) {
	if (!(dpy=XOpenDisplay(0x0))) return 1;
	scr = DefaultScreen(dpy);
	root = RootWindow(dpy,scr);
	sw = DisplayWidth(dpy,scr);
	sh = DisplayHeight(dpy,scr);
	if (argc > 1) {
		if (argv[1][0] == '-' || argv[1][0] == 'h') usage();
		else if (argv[1][0] == 'p') XWarpPointer(dpy,None,root,0,0,0,0,sw,sh);
		else if (argv[1][0] == 'm' && argc == 3) move(argv[2]); 
		else if (argv[1][0] == 'b' && argc == 3) press(argv[2]); 
		else if (argc > 2) XWarpPointer(dpy,None,root,0,0,0,0,atoi(argv[1]),atoi(argv[2]));
		if (argc == 4) press(argv[3]);
		XFlush(dpy);
		return 0;
	}
	int i;
	KeyCode code;
	for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++)
		if ( (code=XKeysymToKeycode(dpy,keys[i].keysym)) ) {
			XGrabKey(dpy,code,0,root,True,GrabModeAsync,GrabModeAsync);
			XGrabKey(dpy,code,LockMask,root,True,GrabModeAsync,GrabModeAsync);
		}
	XEvent ev;
	XKeyEvent *e;
	KeySym keysym;
	while (running && !XNextEvent(dpy,&ev))
		if (ev.type == KeyPress) {
			e = &ev.xkey;
			keysym = XkbKeycodeToKeysym(dpy,(KeyCode)e->keycode,0,0);
			for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++)
				if ( (keysym==keys[i].keysym) && keys[i].func )
					keys[i].func(keys[i].arg);
		}
	XCloseDisplay(dpy);
	return 0;
}
