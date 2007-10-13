#ifndef __CACHE_H__
#define __CACHE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "chain.h"
#include "url.h"

/*
 * Cache Op codes
 */
#define CA_Send    (0)  /* Normal update */
#define CA_Close   (1)  /* Successful operation close */
#define CA_Abort   (2)  /* Operation abort */

/*
 * Flag Defines
 */
#define CA_GotHeader       (1)  /* True after header is completely got */
#define CA_GotContentType  (2)  /* True after Content-Type is known */
#define CA_GotLength       (4)  /* True if Content-Length is known */
#define CA_GotData         (8)  /* True if we have all Data in cache */
#define CA_FreeData       (16)  /* Free the cache Data on close */
#define CA_Redirect       (32)  /* Data actually points to a redirect */
#define CA_ForceRedirect  (64)  /* Unconditional redirect */
#define CA_TempRedirect  (128)  /* Temporal redirect */
#define CA_NotFound      (256)  /* True if remote server didn't find the URL */
#define CA_Stopped       (512)  /* True if the entry has been stopped */
#define CA_MsgErased    (1024)  /* Used to erase the bw's status bar */
#define CA_RedirectLoop (2048)  /* Redirect loop */
#define CA_InternalUrl  (4096)  /* URL content is generated by dillo */
#define CA_HugeFile     (8192)  /* URL content is too big */

/*
 * Callback type for cache clients
 */
typedef struct _CacheClient CacheClient_t;
typedef void (*CA_Callback_t)(int Op, CacheClient_t *Client);

/*
 * Data structure for cache clients.
 */
struct _CacheClient {
   int Key;                 /* Primary Key for this client */
   const DilloUrl *Url;     /* Pointer to a cache entry Url */
   void *Buf;               /* Pointer to cache-data */
   uint_t BufSize;          /* Valid size of cache-data */
   CA_Callback_t Callback;  /* Client function */
   void *CbData;            /* Client function data */
   void *Web;               /* Pointer to the Web structure of our client */
};

/*
 * Function prototypes
 */
void a_Cache_init(void);
int a_Cache_open_url(void *Web, CA_Callback_t Call, void *CbData);
int a_Cache_get_buf(const DilloUrl *Url, char **PBuf, int *BufSize);
void a_Cache_process_dbuf(int Op, const char *buf, size_t buf_size,
                          const DilloUrl *Url);
void a_Cache_entry_inject(const DilloUrl *Url, Dstr *data_ds);
void a_Cache_entry_remove_by_url(DilloUrl *url);
void a_Cache_freeall(void);
CacheClient_t *a_Cache_client_get_if_unique(int Key);
void a_Cache_stop_client(int Key);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CACHE_H__ */
