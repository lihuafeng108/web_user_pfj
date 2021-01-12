/*****************************************************************************
 * 文件名称:ubus_visitor.c
 * 文件作者:lihf
 * 完成日期:2020.01.09
 * 程序功能:向ubus总线注册的ubus_test对象请求say_hello方法
 *****************************************************************************/
 /********************************************************************************************************
  *                                           File Include                                               *
  ********************************************************************************************************/
#include <stdio.h>
#include <json/json.h>

#include "libubox/ustream.h"
#include "libubox/blobmsg.h"
#include "libubox/blobmsg_json.h"
#include "libubox/uloop.h"
#include "libubox/usock.h"

#include "ubus/libubus.h"

/********************************************************************************************************
 *                                        Defines                                                       *
 ********************************************************************************************************/
typedef struct config_ubus
{
    struct ubus_context *ubus_ctx;
    struct uloop_timeout ubus_timer;
}config_ubus_t;


/********************************************************************************************************
 *                                        Local Functions Declaring                                     *
 ********************************************************************************************************/

/********************************************************************************************************
 *                                        Global Variables                                              *
 ********************************************************************************************************/

/********************************************************************************************************
 *                                        Local Variables                                               *
 ********************************************************************************************************/
static config_ubus_t gs_ubus = {.ubus_ctx=NULL, };

static const struct blobmsg_policy gsc_info_policy[] = {
    {"action", BLOBMSG_TYPE_STRING},
    {"para", BLOBMSG_TYPE_TABLE},
};

/********************************************************************************************************
 *                                        Local Functions                                               *
 ********************************************************************************************************/
static inline char *blobmsg_get_string_s(struct blob_attr *attr)
{
    char *str = blobmsg_get_string(attr);
    if(NULL == str)
    {
        str = "";
    }
    return str;
}

static void ubus_reconnect_cb(struct uloop_timeout *timeout)
{
    if(!ubus_reconnect(gs_ubus.ubus_ctx, NULL))
    {
        ubus_add_uloop(gs_ubus.ubus_ctx);
    }
    else
    {
        uloop_timeout_set(timeout, 2000);
    }
}

static void ubus_disconnect_cb(struct ubus_context *ctx)
{
    printf("FILE : %s    LINE : %d  \n", __FILE__, __LINE__);
    gs_ubus.ubus_timer.cb = ubus_reconnect_cb;
    uloop_timeout_set(&gs_ubus.ubus_timer, 2000);
}

static int ubus_init(void)
{
    uloop_init();
    gs_ubus.ubus_ctx = ubus_connect(NULL);
    if(NULL == gs_ubus.ubus_ctx)
    {
        printf("ubus_connect failed\n");
        return -1;
    }

    gs_ubus.ubus_ctx->connection_lost = ubus_disconnect_cb;
    ubus_add_uloop(gs_ubus.ubus_ctx);

    return 0;
}

static int ubus_done(void)
{
    if(gs_ubus.ubus_ctx)
    {
        ubus_free(gs_ubus.ubus_ctx);
    }

    return 0;
}

static void ubus_say_hello_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    static const struct blobmsg_policy say_hello_policy[] = {
        {"Hi", BLOBMSG_TYPE_STRING},
    };
    int pllicy_size = sizeof(say_hello_policy) / sizeof(struct blobmsg_policy);
    struct blob_attr *tb[pllicy_size];

    blobmsg_parse(say_hello_policy, ARRAY_SIZE(say_hello_policy), tb, blob_data(msg), blob_len(msg));
    for(int i=0; i<pllicy_size; i++)
    {
        if(NULL != tb[i])
        {
            if(BLOBMSG_TYPE_STRING == blobmsg_type(tb[i]))
            {
                printf("%s:%s\n", say_hello_policy[i].name, (char *)blobmsg_get_string_s(tb[i]));
            }
            else
            {
                printf("%s:%d\n", say_hello_policy[i].name, blobmsg_get_u32(tb[i]));
            }
        }

    }
}

int main(int argc, char *argv[])
{
    ubus_init();

    //ubus_test
    int ret ;
    uint32_t id;
    struct blob_buf para = {0};

    blob_buf_init(&para, 0);
    blobmsg_add_string(&para, "action", "say");
    ret = ubus_lookup_id(gs_ubus.ubus_ctx, "ubus_boss", &id);
    if(ret != UBUS_STATUS_OK)
    {
        printf("lookup ubus_boss obj failed!\n");
        return ret;
    }
    ubus_invoke(gs_ubus.ubus_ctx, id, "say_hello", para.head, ubus_say_hello_cb, NULL, 200*1000);
    blob_buf_free(&para);

    uloop_run();
    ubus_done();
    return 0;
}

/********************************************************************************************************
 *                                        Global Functions                                              *
 ********************************************************************************************************/

