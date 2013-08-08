/* 
 * Project: VNES
 * Author: Kurt Sassenrath
 * Created: 08-Aug-2013
 * File: d-opengl.c
 * 
 * Description:
 * 
 *      OpenGL Display implementation.
 * 
 * Change Log:
 *      08-Aug-2013:
 *          File created.
 */

#include "display.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

GLint default_att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

/* OpenGL implementation */
struct win_impl {
    u16 x;      /* X offset of window */
    u16 y;      /* Y offset of window */
    u16 width;  /* Width of window */
    u16 height; /* Height of window */

    /* OpenGL attributes */
    GLint       *att;
    
    /* X11-specific variables */
    Display                 *dpy;
    Window                   root;
    Window                   win;
    XVisualInfo             *vi;
    Colormap                 cmap;
    XSetWindowAttributes     swa;
    GLXContext               glc;
    XWindowAttributes        gwa;
    XEvent                   xev;
    
    /* Window Manager Protocols */
    struct {
        Atom delete_window;
    } wmproto;
};

int Open_Display(vnes_display **disp, u16 w, u16 h) {
    struct win_impl *win;
    /* Allocate display memory */
    *disp = (vnes_display *)malloc(sizeof(vnes_display));
    bzero(*disp, sizeof(vnes_display));
    
    /* Allocate window memory and create a pointer to it */
    (*disp)->win = (struct win_impl *)malloc(sizeof(struct win_impl));
    bzero((*disp)->win, sizeof(struct win_impl));
    win = (*disp)->win;
    
    /* Assign width/height */
    win->width = w;
    win->height = h;
    
    /* Attempt to open the X display */
    win->dpy = XOpenDisplay(NULL);
    if (NULL == win->dpy) {
        /* Error - cannot connect to X server */
        goto err;
    }
    
    /* Get the handle of the root window, set OpenGL attributes */
    win->root = DefaultRootWindow(win->dpy);
    win->att = default_att;
    
    /* Get the OpenGL Visual */
    win->vi = glXChooseVisual(win->dpy, 0, win->att);
    if (NULL == win->vi) {
        /* Error - cannot find an appropriate visual */
        goto err;
    }
    
    /* Create the color map */
    win->cmap = XCreateColormap(win->dpy, win->root, win->vi->visual, AllocNone);
    
    /* Initialize the SetWindowAttributes struct */
    win->swa.colormap = win->cmap;
    win->swa.event_mask = ExposureMask | KeyPressMask;
    
    /* Create the X Window, make it appear, show the title string. */
    win->win = XCreateWindow(win->dpy, win->root, 0, 0, win->width, win->height, 0, win->vi->depth, InputOutput, win->vi->visual, CWColormap | CWEventMask, &(win->swa));
    XMapWindow(win->dpy, win->win);
    Set_Display_Title(*disp, "[VNES]");
    
    /* Create the OpenGL context */
    win->glc = glXCreateContext(win->dpy, win->vi, NULL, GL_TRUE);
    glXMakeCurrent(win->dpy, win->win, win->glc);
    
    /* Enable GL Depth buffer */
    glEnable(GL_DEPTH_TEST);
    
    /* Attach window manager messages */
    win->wmproto.delete_window = XInternAtom(win->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(win->dpy, win->win, &win->wmproto.delete_window, 1);
    return 1;
err:
    free(win);
    free(*disp);
    *disp = 0;
    return 0;
}

void Close_Display(vnes_display *disp) {
    if (!disp) return;
    struct win_impl *win = disp->win;
    glXMakeCurrent(win->dpy, None, NULL);
    glXDestroyContext(win->dpy, win->glc);
    XDestroyWindow(win->dpy, win->win);
    XCloseDisplay(win->dpy);
    free(win);
    free(disp);
}

void Test_GL_Render(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 1., -1., 1., 1., 20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

    glBegin(GL_QUADS);
        glColor3f(1., 0., 0.); glVertex3f(-1.0, -1.0, 0.);
        glColor3f(0., 1., 0.); glVertex3f( 1.0, -1.0, 0.);
        glColor3f(0., 0., 1.); glVertex3f( 1.0,  1.0, 0.);
        glColor3f(1., 1., 0.); glVertex3f(-1.0,  1.0, 0.);
    glEnd();
}

void Display_Loop(vnes_display *disp) {
    struct win_impl *win;
    XEvent *xev;
    if (!disp) return;
    win = disp->win;
    xev = &(win->xev);
    while (1) {
        XNextEvent(win->dpy, xev);
        if (xev->type == Expose) {
            XGetWindowAttributes(win->dpy, win->win, &(win->gwa));
            glViewport(0, 0, win->gwa.width, win->gwa.height);
            win->width = win->gwa.width;
            win->height = win->gwa.height;
            Set_Display_Title(disp, "[VNES] %ux%u", win->width, win->height);
            Test_GL_Render();
            glXSwapBuffers(win->dpy, win->win);
        } else if (xev->type == KeyPress) {
            XKeyPressedEvent *keypress = (XKeyPressedEvent *)xev;
            KeyCode keycode = keypress->keycode;
            KeySym keysim = XKeycodeToKeysym(win->dpy, keycode, 0);
            char *key = XKeysymToString(keysim);
            printf("Key pressed: %s\n", key);
            if (0 == strcmp(key, "q")) {
                Close_Display(disp);
                break;
            }
        } else if (xev->type == ClientMessage) {
            if (win->wmproto.delete_window == (Atom)xev->xclient.data.l[0]) {
                Close_Display(disp);
                break;
            }
        } else {
            printf("Unhandled event type: %u\n", xev->type);
        }
    }
}

void Set_Display_Title(vnes_display *disp, const char *format, ...) {
    char title[256];
    va_list args;
    if (!disp) return;
    va_start(args, format);
    vsprintf(title, format, args);
    XStoreName(disp->win->dpy, disp->win->win, title);
    va_end(args);
}
