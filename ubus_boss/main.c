/*****************************************************************************
 * �ļ�����:main.c
 * �ļ�����:lihf
 * �������:2021.01.12
 * ��ǰ�汾:V1.0.0
 * ����˵��:ע��ubus_boss����
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
static int say_hello(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *blob_msg);


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

static struct ubus_method gs_say_hello_method[] = {UBUS_METHOD("say_hello", say_hello, gsc_info_policy)};  //���󷽷�����
static struct ubus_object_type gs_say_hello_type = UBUS_OBJECT_TYPE("sayhello", gs_say_hello_method); //�������ͣ��Ա�Ӧ��ûʲô����

static struct ubus_object gs_rsu_info = {
    .name = "ubus_boss",  //��������
    .type = &gs_say_hello_type,
    .methods = gs_say_hello_method,
    .n_methods = ARRAY_SIZE(gs_say_hello_method),
};

static struct blob_buf rs_blobbuf = {0};  //�洢��Ӧ���ݵ�blob�ṹ�����

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

static int say_hello(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *blob_msg)
{
    int  ret = -1;
    char action[32] = {0};
    int  policy_size = sizeof(gsc_info_policy) / sizeof(struct blobmsg_policy);
    struct blob_attr *tb[policy_size];

    blobmsg_parse_array(gsc_info_policy, ARRAY_SIZE(gsc_info_policy), tb, blob_data(blob_msg), blob_len(blob_msg));
    for(int i=0; i<policy_size; i++)
    {
        if(NULL != tb[i])
        {
            if(0 == strcasecmp("action", gsc_info_policy[i].name))
            {
                printf("action=%s\n", blobmsg_get_string_s(tb[i]));
                strncpy(action, blobmsg_get_string_s(tb[i]), sizeof(action) - 1);
            }
        }
    }

    if(0 != strcasecmp("say", action))
    {   //ֻ��Ӧget����
        return ret;
    }

    //��ubus������Ҫ����Ϣ�����Ұ���Э����װ������������
    memset((char *)&rs_blobbuf, 0, sizeof(rs_blobbuf));
    blob_buf_init(&rs_blobbuf, 0);
    blobmsg_add_string(&rs_blobbuf, "Hi", "Welcome guy!");
    ret = ubus_send_reply(ctx, req, rs_blobbuf.head);

    return ret;
}

int ubus_init(void)
{
    int ret = -1;

    blob_buf_init(&rs_blobbuf, 0);

    uloop_init();
    gs_ubus.ubus_ctx = ubus_connect(NULL);
    if(NULL == gs_ubus.ubus_ctx)
    {
        printf("ubus_connect failed\n");
        return ret;
    }

    gs_ubus.ubus_ctx->connection_lost = ubus_disconnect_cb;
    ubus_add_uloop(gs_ubus.ubus_ctx);
    ret = ubus_add_object(gs_ubus.ubus_ctx, &gs_rsu_info);
    if(ret)
    {
        printf("ubus add object failed!\n");
        return ret;
    }

    return ret;
}

int ubus_done(void)
{
    if(gs_ubus.ubus_ctx)
    {
        ubus_free(gs_ubus.ubus_ctx);
    }

    blob_buf_free(&rs_blobbuf);
    return 0;
}

int main(int argc, char *argv[])
{
    printf("I am the boss ubus_boject\n");
    ubus_init();
    uloop_run();
    ubus_done();
    return 0;
}

/********************************************************************************************************
 *                                        Global Functions                                              *
 ********************************************************************************************************/

