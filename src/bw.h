#ifndef __BW_H__
#define __BW_H__

#include <sys/types.h>

#include "url.h"     /* for DilloUrl */

/*
 * Flag Defines for a_Bw_stop_clients()
 */
#define BW_Root            (1)  /* Root URLs */
#define BW_Img             (2)  /* Image URLs */
#define Bw_Force           (4)  /* Stop connection too */


typedef struct _BrowserWindow BrowserWindow;


/* browser_window contains the specific data for a single window */
struct _BrowserWindow
{
   /* Pointer to the UI object this bw belongs to */
   void *ui;

   /* All the rendering is done by this.
    * It is defined as a void pointer to avoid C++ in this structure.
    * C++ sources have to include browser.h and cast it into an object. */
   void *render_layout;

   /* A list of active cache clients in the window (The primary Key) */
   Dlist *RootClients;
   /* Image Keys for all active connections in the window */
   Dlist *ImageClients;
   /* Number of images in the page */
   int NumImages;
   /* Number of images already loaded */
   int NumImagesGot;
   /* List of all Urls requested by this page (and its types) */
   Dlist *PageUrls;

   /* The navigation stack (holds indexes to history list) */
   int *nav_stack;
   int nav_stack_size;       /* [1 based] */
   int nav_stack_size_max;
   /* 'nav_stack_ptr' refers to what's being displayed */
   int nav_stack_ptr;        /* [0 based] */
   /* When the user clicks a link, the URL isn't pushed directly to history;
    * nav_expect_url holds it until the first answer-bytes are got. Only then
    * it is sent to history and referenced in 'nav_stack[++nav_stack_ptr]' */
   DilloUrl *nav_expect_url;
   /* 'nav_expecting' is true if the last URL is being loaded for
    * the first time and has not gotten the dw yet. */
   bool_t nav_expecting;

   /* Counter for the number of hops on a redirection. Used to stop
    * redirection loops (accounts for WEB_RootUrl only) */
   int redirect_level;

   /* flag for button sens idle function */
   int sens_idle_up;

   /* TODO: remove me */
   void *question_dialog_data;
   void *question_dialog_answer;

   /* TODO: maybe this fits better in the linkblock.
    * Although having it here avoids having a signal for handling it. */
   int num_page_bugs;
   Dstr *page_bugs;
};


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


void a_Bw_init(void);
BrowserWindow *a_Bw_new(int width, int height, uint32_t xid);
void a_Bw_free(BrowserWindow *bw);
BrowserWindow *a_Bw_get();

void a_Bw_add_client(BrowserWindow *bw, int Key, int Root);
int a_Bw_remove_client(BrowserWindow *bw, int ClientKey);
void a_Bw_close_client(BrowserWindow *bw, int ClientKey);
void a_Bw_stop_clients(BrowserWindow *bw, int flags);
void a_Bw_add_url(BrowserWindow *bw, const DilloUrl *Url);
void a_Bw_cleanup(BrowserWindow *bw);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BROWSER_H__ */
