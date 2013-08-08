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
};

int Open_Display(vnes_display **disp) {
    struct win_impl *win;
    /* Allocate display memory */
    *disp = (vnes_display *)malloc(sizeof(vnes_display));
    bzero(*disp, sizeof(vnes_display));
    
    /* Allocate window memory and create a pointer to it */
    (*disp)->win = (struct win_impl *)malloc(sizeof(struct win_impl));
    bzero((*disp)->win, sizeof(struct win_impl));
    win = (*disp)->win;
    
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
    win->win = XCreateWindow(win->dpy, win->root, 0, 0, 600, 600, 0, win->vi->depth, InputOutput, win->vi->visual, CWColormap | CWEventMask, &(win->swa));
    XMapWindow(win->dpy, win->win);
    XStoreName(win->dpy, win->win, "[VNES]");
    
    /* Create the OpenGL context */
    win->glc = glXCreateContext(win->dpy, win->vi, NULL, GL_TRUE);
    glXMakeCurrent(win->dpy, win->win, win->glc);
    
    /* Enable GL Depth buffer */
    glEnable(GL_DEPTH_TEST);
    
    return 1;
err:
    free(win);
    free(*disp);
    *disp = 0;
    return 0;
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
            Test_GL_Render();
            glXSwapBuffers(win->dpy, win->win);
        } else if (xev->type == KeyPress) {
            glXMakeCurrent(win->dpy, None, NULL);
            glXDestroyContext(win->dpy, win->glc);
            XDestroyWindow(win->dpy, win->win);
            XCloseDisplay(win->dpy);
            free(win);
            free(disp);
            break;
        }
    }
}
