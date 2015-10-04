#ifndef EVT_TLS_H
#define EVT_TLS_H


#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#include "queue.h"


typedef struct evt_tls_s evt_tls_t;

typedef void (*evt_conn_cb)(evt_tls_t *con, int status);
typedef void (*evt_accept_cb)(evt_tls_t *con, int status);
typedef void (*evt_allocator)(evt_tls_t *con, size_t sz, void *buf);
typedef void (*evt_read_cb)(evt_tls_t *con, char *buf, int size);
typedef void (*evt_write_cb)(evt_tls_t *con, int status);
//XXX: should we remove status param
typedef void (*evt_close_cb)(evt_tls_t *con, int status);

typedef int (*net_wrtr)(evt_tls_t *tls, void *edata, int len);


typedef struct evt_ctx_s
{
    //find better place for it , should be one time init
    SSL_CTX *ctx;

    //is cert set
    int cert_set;

    //is key set
    int key_set;

    //flag to signify if ssl error has occured
    int ssl_err_;

    void *live_con[2];

    net_wrtr writer;
} evt_ctx_t;

struct evt_tls_s {

    void    *data;

    BIO     *app_bio_; //Our BIO, All IO should be through this

    SSL     *ssl;
    
    //network writer used for writing encrypted data
    net_wrtr writer;

    evt_allocator allocator;

    //callbacks
    evt_conn_cb connect_cb;
    evt_accept_cb accept_cb;
    evt_read_cb rd_cb;
    evt_write_cb write_cb;
    evt_close_cb cls_cb;

    //back handle to parent
    evt_ctx_t *evt_ctx;

    QUEUE q;
    BIO     *ssl_bio_; //the ssl BIO used only by openSSL
};


//supported TLS operation
enum tls_op_type {
    EVT_TLS_OP_HANDSHAKE
   ,EVT_TLS_OP_READ
   ,EVT_TLS_OP_WRITE
   ,EVT_TLS_OP_SHUTDOWN
};

/*configure the tls state machine */
int evt_ctx_init(evt_ctx_t *tls);

/* set the certifcate and key in order */
int evt_ctx_set_crt_key(evt_ctx_t *tls, char *crtf, char *key);

/* test if the certificate */
int evt_ctx_is_crtf_set(evt_ctx_t *t);

/* test if the key is set */
int evt_ctx_is_key_set(evt_ctx_t *t);

evt_tls_t *get_tls(evt_ctx_t *d_eng);
void evt_ctx_set_writer(evt_ctx_t *ctx, net_wrtr my_writer);

int evt_tls_feed_data(evt_tls_t *c, void *data, int sz);

int evt_tls_connect(evt_tls_t *con, evt_conn_cb cb);
int evt_tls_accept( evt_tls_t *tls, evt_accept_cb cb);
int evt_tls_write(evt_tls_t *c, void *msg, int str_len, evt_write_cb on_write);
int evt_tls_read(evt_tls_t *c, evt_allocator allok, evt_read_cb on_read );
int evt_tls_close(evt_tls_t *c, evt_close_cb cls);


//openssl 1.0.2 and later has SSL_is_server API to check
//if the ssl connection is server or not
// Some older versions does not have this function.
// Hence this function is introduced.

// 0 - client
// 1 - server
// XXX: make this enum
int evt_tls_get_role(const evt_tls_t *t);

void evt_tls_set_role(evt_tls_t *t, int role);


#ifdef __cplusplus 
}
#endif

#endif //define EVT_TLS_H
