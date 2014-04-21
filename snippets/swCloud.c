/******************************************************************************
 *
 * Copyright (c) 2013 TP-LINK Technologies CO.,LTD.
 * All rights reserved.
 *
 * FILE NAME  :   swCloud.c
 * VERSION    :   1.1
 * DESCRIPTION:   cloud function
 *				 
 *
 * 01	16/8/2013	cxq	create.
 * 02   24/2/2014   refactor by zhibo.
 ******************************************************************************/
#include<ctype.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/un.h>
#include<arpa/inet.h>
#include "sysMain.h"
#include "cmLock.h"
#include "cmList.h"
#include "cmCommand.h"
#include "swCloud.h"
#include "cldProtocol.h"


#define CLOUD_PACE			500 /* ms, must less than 1 sec */
#define CLOUD_PID_FILE		"/var/run/cloud.pid"
#define CLOUD_SOCK_PATH 	"/tmp/cloud.socket"
#define CLOUD_CONFIG_PATH	SERVER_CONFIG_DIR"cloud.conf"


#define CLOUD_STATUS_INIT			0	/* start status */
#define CLOUD_STATUS_CONNECTING		1	/* connecting to server */
#define CLOUD_STATUS_CONNECTED		2	/* connected with server */
#define CLOUD_STATUS_DISCONNECTED	3	/* cant connect to cloud */
#define CLOUD_STATUS_BROKEN			4	/* binded but auth failed */
#define CLOUD_STATUS_RESTART        5   /* environment changed */

#define CLOUD_REACHABLE_NO           1   /* ping failed */
#define CLOUD_REACHABLE_YES          2   /* ping success */

#define CLOUD_OP_NONE				0
#define CLOUD_OP_REGUSR				0
#define CLOUD_OP_NONE				0
#define CLOUD_OP_NONE				0
typedef struct _devinfo_t{
	char	id[CLD_DEVID_LEN];
	char	model[CLOUD_NAME_LEN];
	char	desc[CLOUD_NAME_LEN];
	char	admin[CLOUD_NAME_LEN];
	char	password[CLOUD_NAME_LEN];
	char	eip[16];
	char	eport[8];
	char	stream_port[8];
}devinfo_t;

typedef struct _cloud_t{
	char			host[CLOUD_NAME_LEN];
	char			port[8];
	int				retry_wait; /* msec of retry intvl */
	int				retry_max;	/* # of maximum retry to connect to server */
	int				p2p_timeout;

	CM_MUTEX_DEFINE	(lock);
	devinfo_t		devinfo;
	char			account[CLOUD_NAME_LEN];
	char			password[CLOUD_NAME_LEN];
    int             reachable;  /* reachable to cloud ? */
	int				binded;     /* binded to cloud ? */
	int				update;     /* need update ? */
	int				status;	    /* current connection status */
	int				op;		    /* current operation */
	int				fd;
	int				waits;	    /* msec waited */
	int				retries;
}cloud_t;

static cloud_t cloud;


#define CFG_CLOUD_HOST 			"CLOUD_HOST"
#define CFG_STREAMD_PORT 		"CLOUD_STREAMD_PORT"
#define CFG_P2P_TIMEOUT 		"CLOUD_P2P_TIMEOUT"
#define CFG_CLOUD_RETRY_WAIT	"CLOUD_RETRY_WAIT"
#define CFG_CLOUD_RETRY_MAX		"CLOUD_RETRY_MAX"
static int def_auth_retrywait = 3; /* seconds */
static int def_auth_retries = 20;
static int def_p2p_timeout = 30;
static param_t CloudConf[] = {
	{
		.name = CFG_CLOUD_HOST,
		.data = "control.skylight.tp-link.com",
		.type = DT_SZSTR,
		.size = sizeof("control.skylight.tp-link.com"),
		.cap = CLOUD_NAME_LEN
	},
	{
		.name = CFG_STREAMD_PORT,
		.data = "8080",
		.type = DT_SZSTR,
		.size = sizeof("8080"),
		.cap = sizeof(cloud.port)
	},
	{
		.name = CFG_CLOUD_ACCOUNT,
		.data = "",
		.type = DT_SZSTR,
		.size = sizeof(""),
		.cap = CLOUD_NAME_LEN
	},
	{
		.name = CFG_CLOUD_PASSWORD,
		.data = "",
		.type = DT_SZSTR,
		.size = sizeof(""),
		.cap = CLOUD_NAME_LEN
	},
	{
		.name = CFG_CLOUD_RETRY_WAIT,
		.data = &def_auth_retrywait,
		.type = DT_ULONG,
		.size = sizeof(unsigned long),
		.cap = sizeof(unsigned long)
	},
	{
		.name = CFG_CLOUD_RETRY_MAX,
		.data = &def_auth_retries,
		.type = DT_ULONG,
		.size = sizeof(unsigned long),
		.cap = sizeof(unsigned long)
	},
	{
		.name = CFG_P2P_TIMEOUT,
		.data = &def_p2p_timeout,
		.type = DT_ULONG,
		.size = sizeof(unsigned long),
		.cap = sizeof(unsigned long)
	},

	{
		.name = NULL
	}
};


// TODO add msg handler instead of direct call
extern COMMON_STATUS swSystemSetProductName(char * name);
extern COMMON_STATUS swSystemSetProductNameCheck(char * name);
extern int swVideoCtrlSetResolution(int width, int height);

static void cloud_set_status(int status)
{
    if ( 0 <= status && status <= 5 )
    {
        CM_MUTEX_LOCK(cloud.lock);
        cloud.status = status;
        CM_MUTEX_UNLOCK(cloud.lock);
    }
}

static int cloud_get_status()
{
    int status;
    CM_MUTEX_LOCK(cloud.lock);
    status = cloud.status;
    CM_MUTEX_UNLOCK(cloud.lock);
    
    return status;
}

static void cloud_do_disconnect()
{
	if ( cloud.fd != -1 ) {
		close(cloud.fd);
		cloud.fd = -1;
	}
}


static int swMD5Code(const char* id, char code[36])
{
#include<openssl/md5.h>
	MD5_CTX md5_ctx;
	const char *mid;
	char buf[64] = {0};
	unsigned char sign[16];
	int i;

	if ( !id || !code || !id[0] ) {
		return -1;
	}
	MD5_Init(&md5_ctx);
	/* add salt */
	mid = id + (strlen(id)>>1) + 1;
	snprintf(buf, sizeof(buf), "%d%s%d%s%d", mid[0], mid, mid[0], id, mid[0]);
	MD5_Update(&md5_ctx, buf, sizeof(buf));
	MD5_Final(sign, &md5_ctx);

	for (i = 0; i < sizeof(sign); i++) {
		sprintf(code, "%02x", sign[i]);
		code += 2;
	}
	return 0;
}


/* @return : 0 - succeed, -1 - socket error */
static COMMON_STATUS cloud_do_connect()
{
	do {
		char msg[32];
		struct sockaddr_un addr_un;

		cloud.fd = socket(PF_UNIX, SOCK_STREAM, 0);
		if ( -1 == cloud.fd ) {
			break;
		}

		addr_un.sun_family = AF_UNIX;
		snprintf(addr_un.sun_path, sizeof(addr_un.sun_path), "%s", CLOUD_SOCK_PATH);
		if ( connect(cloud.fd, (struct sockaddr*)&addr_un, sizeof(addr_un)) != 0 ) {
			break;
		}

		if ( read(cloud.fd, msg, sizeof(msg)) <= 0 ) {
			break;
		}

        cloud.reachable = CLOUD_REACHABLE_YES;
		return SUCCEED;
	} while (0);

	SYSLOG(CMLOG_ERR, "failed to connect with cloud(local), errno:%d\n", errno);
    cloud.reachable = CLOUD_REACHABLE_NO;
	return ERRNO_CLOUD_NETWORK;
}


static int tcp_read(int fd, void* data, int size)
{
	int off = 0;
	do {
		int ret = read(fd, data+off, size-off);
		if ( -1 == ret && EINTR == errno ) {
			continue;
		}
		if ( ret <= 0 ) {
			return -1;
		}
		off += ret;
	} while ( off < size );
	return 0;
}


static int tcp_write(int fd, void* data, int size)
{
	int off = 0;
	do {
		int ret = write(fd, data+off, size-off);
		if ( -1 == ret ) {
			if ( EINTR == errno ) {
				continue;
			}
			return -1;
		}
		off += ret;
	} while ( off < size );
	return 0;
}


static int cloud_recv_packet(char* res, int len)
{
	int size;
	cldhdr_t* hdr = (cldhdr_t*)res;

	/* read header */
	if ( tcp_read(cloud.fd, res, sizeof(cldhdr_t)) != 0 ) {
		return -1;
	}

	size = hdr->size<<2;
	if ( tcp_read(cloud.fd, res+sizeof(cldhdr_t), size) != 0 ) {
		return -1;
	}
	return sizeof(cldhdr_t) + size;
}


/* @return: 0 succeed, -1 failed */
static int cloud_send_packet(int cmd, int subcmd, const char* data, int flags, char* res, int* len)
{
	char buf[CLD_MAX_PKTSIZE]={0};
	int size;
	cldhdr_t* hdr = (cldhdr_t*)buf;
	
	hdr->magic = CLD_MAGIC;
	hdr->ver = CLD_VER;
	hdr->cmd = cmd;
	hdr->flags = flags;
	size = snprintf(buf+sizeof(cldhdr_t), sizeof(buf)-sizeof(cldhdr_t), "%s", data);
	if ( sizeof(cldhdr_t) + size >= sizeof(buf) ) {
		SYSLOG(CMLOG_ERR, "packet too large\n");
		return -1;
	}
	SYSLOG(CMLOG_DEBUG, "service id %d, content:\n%s\n", cmd, data);
	size += 4;
	size &= ~3;
	hdr->size = size>>2;
	size += sizeof(cldhdr_t);
	if ( tcp_write(cloud.fd, buf, size) != 0 ) {
		SYSLOG(CMLOG_ERR, "tcp_write error, fd %d\n", cloud.fd);
		return -1;
	}
	if ( hdr->flags & (PKTFLG_NOTIFY|PKTFLG_RESPONSE) ) {
		return 0;
	}

	/* response */
	if ( cloud_recv_packet(buf, sizeof(buf)) < 0 ) {
		SYSLOG(CMLOG_ERR, "cloud_recv_packet error\n");
		return -1;
	}
	*len = hdr->size<<2;
	memcpy(res, buf+sizeof(cldhdr_t), *len);
/*
	if ( tcp_read(cloud.fd, buf, sizeof(cldhdr_t)) != 0 ) {
		SYSLOG(CMLOG_ERR, "tcp_read error\n");
		return -1;
	}
	*len = hdr->size<<2;
	if ( tcp_read(cloud.fd, res, hdr->size<<2) != 0 ) {
		SYSLOG(CMLOG_ERR, "tcp_read error\n");
		return -1;
	}
*/

	return 0;
}

/**
 * @return: 0 - succeed, 1 - register user failed, -1 - socket error 
 */
static COMMON_STATUS cloud_do_regusr(const char* account,
	   const char* username, const char* password)
{
	char buf[CLD_MAX_PKTSIZE]={0}, tmp[CLD_MAX_PKTSIZE], code[36];
	int len, ret;
	
	snprintf(tmp, sizeof(tmp), "%s%s%s", account, username, password);
	swMD5Code(tmp, code);
	snprintf(buf, sizeof(buf), "%s\n%s\n%s\n%s\n", account, username, password, code);
	ret = cloud_send_packet(CLDCMD_REGUSR, 0, buf, 0, buf, &len);
	if ( -1 == ret ) {
		return ERRNO_CLOUD_NETWORK;
	} else {
		if ( !strncasecmp(buf, "OK", 2) ){
			return SUCCEED;
		}
		if ( strstr(buf, "-3") ) {
			return ERRNO_CLOUD_ACCOUNT_EXIST;
		}
		if ( strstr(buf, "-4") ) {
			return ERRNO_CLOUD_USERNAME_EXIST;
		}
		// TODO error detail check
	}

	return ERROR;
}


/**
 * @return: 0 - succeed, 1 - login failed, -1 - socket error 
 */
static COMMON_STATUS cloud_do_auth()
{
	char buf[CLD_MAX_PKTSIZE]={0};
	int len, ret;
	
	snprintf(buf, sizeof(buf), "%s\n%s\n%s\n", cloud.account, cloud.password, cloud.devinfo.id);
	ret = cloud_send_packet(CLDCMD_AUTH, 0, buf, 0, buf, &len);
	if ( -1 == ret ) {
		return ERRNO_CLOUD_NETWORK;
	} else {
		if ( !strncasecmp(buf, "OK", 2) ){
			return SUCCEED;
		}
		if ( strstr(buf, "-6") ) {
			return ERRNO_CLOUD_NOUSER;
		}
		// TODO error detail check
	}

	return ERRNO_CLOUD_AUTH;
}


/* @return: 0 - succeed, 1 - unbind failed, -1 - socket error 
 */
static COMMON_STATUS cloud_do_ubind()
{
	char buf[CLD_MAX_PKTSIZE]={0};
	int len, ret;
	
	SYSLOG(CMLOG_DEBUG, "do_ubind...\n");
	ret = cloud_send_packet(CLDCMD_UBINDEV, 0, "", 0, buf, &len);
	if ( -1 == ret ) {
		return ERRNO_CLOUD_NETWORK;
	} else {
		if ( !strncasecmp(buf, "OK", 2) ){
			return SUCCEED;
		}
		// TODO error detail check
	}

	return ERROR;
}


/* @return: 0 - succeed, 1 - bind failed, -1 - socket error 
 */
static COMMON_STATUS cloud_do_bind()
{
	char buf[CLD_MAX_PKTSIZE]={0};
	char md5code[36];
	int len, ret;
	
	swMD5Code(cloud.devinfo.id, md5code);
	snprintf(buf, sizeof(buf), "%s\n%s\n", cloud.devinfo.id, md5code);
	ret = cloud_send_packet(CLDCMD_BINDEV, 0, buf, 0, buf, &len);
	if ( -1 == ret ) {
		return ERRNO_CLOUD_NETWORK;
	} else {
		if ( !strncasecmp(buf, "OK", 2) ){
			return SUCCEED;
		}
		// TODO error detail check
	}

	return ERROR;
}

/* @return: 0 - succeed, 1 - update failed, -1 - socket error 
 */
static COMMON_STATUS cloud_do_update()
{
	char buf[CLD_MAX_PKTSIZE]={0};
	int len, ret;
	
	/* pkt: model\ndesc\nadmin\npassword\neip\neport\n */
	len = snprintf(buf, sizeof(buf),
		   	"%s\n%s\n%s\n%s\n%s\n%s\n",
		   	cloud.devinfo.model,
		   	cloud.devinfo.desc,
		   	cloud.devinfo.admin,
		   	cloud.devinfo.password,
		   	cloud.devinfo.eip,
			cloud.devinfo.eport);
	ret = cloud_send_packet(CLDCMD_UPDEV, 0, buf, 0, buf, &len);
	if ( -1 == ret ) {
        cloud.reachable = CLOUD_REACHABLE_NO;
		return ERRNO_CLOUD_NETWORK;
	} else {
		if ( !strncasecmp(buf, "OK", 2) ){
			return SUCCEED;
		}
		// TODO error detail check
	}

	return ERROR;
}


/** @return: 0 - no update, 1 - has update
 */
static int cloud_need_update()
{
	int update;

	if ( -1 == cloud.fd ) {
		return 0;
	}

	update = cloud.update;
	cloud.update = 0;

	return update;
}


static int network_read_event(int mwait)
{
	int ret;
	struct timeval tv = {0, 0};
	fd_set fdr;

	tv.tv_sec = mwait/1000;
	tv.tv_usec = mwait*1000;
	if ( -1 == cloud.fd ) {
		sleep(tv.tv_sec);
		usleep(tv.tv_usec);
		return 0;
	}
	FD_ZERO(&fdr);
	FD_SET(cloud.fd, &fdr);
	ret = select(cloud.fd+1, &fdr, NULL, NULL, &tv);
	if ( 0 == ret || (-1 == ret && EINTR == errno) ) {
		return 0;
	}

	SYSLOG(CMLOG_DEBUG, "got socket read event from cloud\n");
	return 1;
}


static int is_cloud_running()
{
	int running = 0;
	struct stat pid_stat;

	if (stat(CLOUD_PID_FILE, &pid_stat) == 0) {
		running = 1;
	}

	return running;
}


/* device control packet 
 * pkt: route\nxxx
 */
static int network_device_control()
{
	char buf[CLD_MAX_PKTSIZE];
	char *pktdata = buf+sizeof(cldhdr_t);
	char *content = pktdata+CLD_RTSIZE+1;
	char *request, *tmp;
	int ret;

	CM_MUTEX_LOCK(cloud.lock);
	if ( (ret=network_read_event(1)) > 0 ) {
		/* device control packet */
		ret = cloud_recv_packet(buf, sizeof(buf));
	}
	CM_MUTEX_UNLOCK(cloud.lock);
	if ( 0 == ret ) {
		return 0; /* no device control packet */
	} else if ( ret < 0 ) {
		return -1;
	}

	// The code is urgly...
	SYSLOG(CMLOG_DEBUG, "got DEVCTL request:\n%s\n", content);
	if ( !strncmp(content, "ECHO\n", strlen("ECHO\n")) ) {
		/* ECHO\nWhat_ever_you_want */
		request = content+strlen("ECHO\n");
		memmove(content, request, strlen(request)+1);
		if ( cloud_send_packet(CLDCMD_DEVCTL, CLDSUBCMD_USER, pktdata, PKTFLG_RESPONSE, 0, 0) != 0 ) {
			SYSLOG(CMLOG_DEBUG, "cloud_send_packet failed\n");
			return -1;
		}
	}
	else if ( !strncmp(content, "RENAME\n", strlen("RENAME\n")) ) {
		/* RENAME\nNEWNAME\n */
		request = content+strlen("RENAME\n");
		strtok_r(request, "\n", &tmp);
		if ( swSystemSetProductName(request) == SUCCEED ) {
			sprintf(content, "OK\n");
		} else {
			sprintf(content, "ER\n");
		}
		if ( cloud_send_packet(CLDCMD_DEVCTL, CLDSUBCMD_USER, pktdata, PKTFLG_RESPONSE, 0, 0) != 0 ) {
			SYSLOG(CMLOG_DEBUG, "cloud_send_packet failed\n");
			return -1;
		}
	} else if ( !strncmp(content, "UNBIND\n", strlen("UNBIND\n")) ) {
		/* UNBIND\n */
		if ( cloud_do_ubind() == SUCCEED ) {
			sprintf(content, "OK\n");
			cloud.binded = 0;
            cloud_set_status(CLOUD_STATUS_CONNECTED);
			sysConfSetString(CFG_CLOUD_ACCOUNT, "");
			sysConfSetString(CFG_CLOUD_PASSWORD, "");
		} else {
			sprintf(content, "ER\n");
		}
		if ( cloud_send_packet(CLDCMD_DEVCTL, CLDSUBCMD_USER, pktdata, PKTFLG_RESPONSE, 0, 0) != 0 ) {
			SYSLOG(CMLOG_DEBUG, "cloud_send_packet failed\n");
			return -1;
		}
	} else if ( !strncmp(content, "GET_RESOLUTION\n", strlen("GET_RESOLUTION\n")) ) {
		int width = 640, height = 480;

		swVideoCtrlGetResolution(&width, &height);
		sprintf(content, "%d*%d", width, height);
		if ( cloud_send_packet(CLDCMD_DEVCTL, CLDSUBCMD_USER, pktdata, PKTFLG_RESPONSE, 0, 0) != 0 ) {
			SYSLOG(CMLOG_DEBUG, "cloud_send_packet failed\n");
			return -1;
		}
	} else if ( !strncmp(content, "RESOLUTION\n", strlen("RESOLUTION\n")) ) {
		/* RESOLUTION\nWIDTH\nHEIGHT\n */
		char *width, *height;

		request = content+strlen("RESOLUTION\n");
		width = strtok_r(request, "\n", &tmp);
		height = strtok_r(NULL, "\n", &tmp);

		if ( width && height && swVideoCtrlSetResolution(atoi(width), atoi(height)) == 0 ) {
			sprintf(content, "OK\n");
		} else {
			sprintf(content, "ER\n");
		}
		if ( cloud_send_packet(CLDCMD_DEVCTL, CLDSUBCMD_USER, pktdata, PKTFLG_RESPONSE, 0, 0) != 0 ) {
			SYSLOG(CMLOG_DEBUG, "cloud_send_packet failed\n");
			return -1;
		}
	}

	return 0;
}

static void cloud_clear_counter()
{
    cloud.retries = 0;
    cloud.waits = 0;
    cloud.retry_wait = sysConfGetLong(CFG_CLOUD_RETRY_WAIT)*1000;
}


static void* cloud_daemon(void* arg)
{
	COMMON_STATUS error;

	pthread_detach(pthread_self());
	while ( !is_cloud_running() ) {
		usleep(CLOUD_PACE);
	}

	while (1) {
		/* state machine */
		switch ( cloud_get_status() ) {
		case CLOUD_STATUS_INIT:         
			if ( !cloud.binded ) {
				break;
			}
            cloud_set_status(CLOUD_STATUS_CONNECTING);
            continue;
		case CLOUD_STATUS_CONNECTING:
			SYSLOG(CMLOG_DEBUG, "connecting to cloud server...\n");
			if ( SUCCEED != cloud_do_connect() ) {
                cloud_set_status(CLOUD_STATUS_DISCONNECTED);
				SYSLOG(CMLOG_DEBUG, "connect to cloud failed.\n");
				continue;
			} else { /* connect return success */
				SYSLOG(CMLOG_DEBUG, "connect to cloud success.\n");
            }
			if ( cloud.binded ) {
				error = cloud_do_auth();
				if ( ERRNO_CLOUD_NETWORK == error ) {
                    cloud_set_status(CLOUD_STATUS_DISCONNECTED);
					SYSLOG(CMLOG_DEBUG, "disconnected\n");
					continue;
				}
				if ( SUCCEED != error ) { /* auth failed */
                    cloud_set_status(CLOUD_STATUS_BROKEN);
					SYSLOG(CMLOG_DEBUG, "auth failed\n");
					continue;
				}
				error = cloud_do_update();
				if ( ERRNO_CLOUD_NETWORK == error ){
                    cloud_set_status(CLOUD_STATUS_DISCONNECTED);
					SYSLOG(CMLOG_DEBUG, "disconnected\n");
					continue;
				}
				if ( SUCCEED != error ) { /* update failed */
                    cloud_set_status(CLOUD_STATUS_BROKEN);
					SYSLOG(CMLOG_DEBUG, "update failed, may be unbinded\n");
					continue;
				}
                cloud_set_status(CLOUD_STATUS_CONNECTED);
                continue;
			} else {
                cloud_set_status(CLOUD_STATUS_DISCONNECTED);
                cloud_clear_counter();
                continue;
            }
		case CLOUD_STATUS_CONNECTED:
			if ( cloud.binded ) {
                if ( cloud_need_update() ){
				    error = cloud_do_update();
				    if ( ERRNO_CLOUD_NETWORK == error ){
                        cloud_set_status(CLOUD_STATUS_DISCONNECTED);
				    	SYSLOG(CMLOG_DEBUG, "disconnected\n");
				    	continue;
				    }
				    if ( SUCCEED != error ) { /* update failed */
                        cloud_set_status(CLOUD_STATUS_BROKEN);
				    	SYSLOG(CMLOG_DEBUG, "update failed, may be unbinded\n");
				    	continue;
				    }
                }
			    break;
			} else {
                cloud_set_status(CLOUD_STATUS_DISCONNECTED);
                continue;
            }
		case CLOUD_STATUS_DISCONNECTED:
			cloud_do_disconnect();
			if ( cloud.binded && cloud.retries < cloud.retry_max ) {
				if ( cloud.waits >= cloud.retry_wait ) {
					/* reconnect cloud server after a while */
					cloud.retries++;
					cloud.retry_wait <<= 1;
					cloud.waits = 0;
                    cloud_set_status(CLOUD_STATUS_CONNECTING);
				}
				cloud.waits += CLOUD_PACE;
			} 
			break;
		case CLOUD_STATUS_BROKEN:
			cloud_do_disconnect();
		    SYSLOG(CMLOG_DEBUG, "cloud status broken\n");
			break;
        case CLOUD_STATUS_RESTART:
            cloud_do_disconnect();
            cloud.reachable = CLOUD_REACHABLE_NO;
            cloud_clear_counter();
            cloud_set_status(CLOUD_STATUS_DISCONNECTED);
		    SYSLOG(CMLOG_DEBUG, 
                    "network environment changed, restart connect...\n");
            continue;
		default:
			SYSLOG(CMLOG_ERR, "unkown cloud status %#x\n", cloud_get_status() );
			break;
		}

		if ( network_read_event(CLOUD_PACE) ) {
            error = network_device_control();
			if ( 0 != error ){
			    cloud_do_disconnect();
                cloud_set_status(CLOUD_STATUS_DISCONNECTED);
			    SYSLOG(CMLOG_ERR, "disconnected from cloud server, error '%d'\n", error);
			}
			continue;
		}
	}

	return NULL;
}


static void msg_sysname_change(void* arg)
{
	if( !arg || cloud_get_status() != CLOUD_STATUS_CONNECTED ) {
		return;
	}

	SYSLOG(CMLOG_DEBUG, "%s, '%s'\n", __FUNCTION__, (char*)arg);
	//sysConfGet("SYSTEM_PRODUCT_NAME", cloud.devinfo.desc, sizeof(cloud.devinfo.desc));
	snprintf(cloud.devinfo.desc, sizeof(cloud.devinfo.desc), "%s", (char*)arg);
	CM_MUTEX_LOCK(cloud.lock);
	cloud.update = 1;
	CM_MUTEX_UNLOCK(cloud.lock);
}


static void msg_network_change(void* arg)
{
	if( !arg || cloud_get_status() != CLOUD_STATUS_CONNECTED ) {
		return;
	}

	SYSLOG(CMLOG_DEBUG, "%s, '%s'\n", __FUNCTION__, (char*)arg);
	CM_MUTEX_LOCK(cloud.lock);
	cloud.update = 1;
	CM_MUTEX_UNLOCK(cloud.lock);
    cloud_set_status(CLOUD_STATUS_RESTART);
}


static void msg_password_change(void* arg)
{
	if( !arg || cloud_get_status() != CLOUD_STATUS_CONNECTED ) {
		return;
	}

	SYSLOG(CMLOG_DEBUG, "%s, '%s'\n", __FUNCTION__, (char*)arg);
	snprintf(cloud.devinfo.password, sizeof(cloud.devinfo.password), "%s", (char*)arg);
	CM_MUTEX_LOCK(cloud.lock);
	cloud.update = 1;
	CM_MUTEX_UNLOCK(cloud.lock);
}


static void msg_upnp_change(void* arg)
{
	char buf[sizeof(cloud.devinfo.eip)+sizeof(cloud.devinfo.eport)];
	char* port = NULL;

	if( !arg || cloud_get_status() != CLOUD_STATUS_CONNECTED ) {
		return;
	}

	/* "ip:port" or "null" */
	SYSLOG(CMLOG_DEBUG, "%s, '%s'\n", __FUNCTION__, (char*)arg);
	snprintf(buf, sizeof(buf), "%s", (char*)arg);
	port = strchr(buf, ':');
	if ( port ) {
		*port = '\0';
		port++;
	}
	CM_MUTEX_LOCK(cloud.lock);
	if ( port ) {
		snprintf(cloud.devinfo.eip, sizeof(cloud.devinfo.eip), "%s", buf);
		snprintf(cloud.devinfo.eport, sizeof(cloud.devinfo.eport), "%s", port);
	} else {
		snprintf(cloud.devinfo.eip, sizeof(cloud.devinfo.eip), "null");
		snprintf(cloud.devinfo.eport, sizeof(cloud.devinfo.eport), "null");
	}
	cloud.update = 1;
	CM_MUTEX_UNLOCK(cloud.lock);
}


static int cloud_init()
{
	memset(&cloud, 0, sizeof(cloud_t));
	if (CM_MUTEX_INIT(cloud.lock) != 0) {
		SYSLOG(CMLOG_CRIT, "CM_MUTEXT_INIT failed, errno %d\n", errno);
		return -1;
	}

	/* load config */
	sysConfLoadFile(CLOUD_CONFIG_PATH, CloudConf, CONF_SAVEONSET);
	sysConfGet(CFG_CLOUD_HOST, cloud.host, sizeof(cloud.host));
	sysConfGet(CFG_CLOUD_ACCOUNT, cloud.account, sizeof(cloud.account));
	sysConfGet(CFG_CLOUD_PASSWORD, cloud.password, sizeof(cloud.password));
	sysConfGet(CFG_STREAMD_PORT, cloud.devinfo.stream_port, sizeof(cloud.devinfo.stream_port));
	cloud.retry_wait = sysConfGetLong(CFG_CLOUD_RETRY_WAIT)*1000;
	cloud.retry_max = sysConfGetLong(CFG_CLOUD_RETRY_MAX);
	cloud.p2p_timeout = sysConfGetLong(CFG_P2P_TIMEOUT);
	cloud.binded = cloud.account[0] ? 1 : 0;
    cloud_set_status(CLOUD_STATUS_INIT);
    cloud.reachable = CLOUD_REACHABLE_NO;
	if ( cloud.binded ) {
		SYSLOG(CMLOG_DEBUG, "device binded to cloud account '%s'\n", cloud.account);
	} else {
		SYSLOG(CMLOG_DEBUG, "device not binded to any cloud account\n");
	}
	cloud.fd = -1;
	sysConfSaveFile(CLOUD_CONFIG_PATH);

	/* register intrested messages */
	sysMsgRegister(MSG_NETCONF_IP_CHANGE, msg_network_change, CMSG_PERSIST);
	sysMsgRegister(MSG_NETCONF_GW_CHANGE, msg_network_change, CMSG_PERSIST);
	sysMsgRegister(MSG_UM_DEFAULT_ADMIN_PASSWORD_CHANGE, msg_password_change, CMSG_PERSIST);
	sysMsgRegister(MSG_UPNP_INFO_STREAM_SERVER_CHANGE, msg_upnp_change, CMSG_PERSIST);
	sysMsgRegister(MSG_SYSTEM_NAME_CHANGE, msg_sysname_change, CMSG_PERSIST);

	return 0;
}


/* start cloud deamon thread */
static int cloud_start()
{
	pthread_t thread;
	char msg_buf[64];
	char upnp_addr[sizeof(cloud.devinfo.eip)+sizeof(cloud.devinfo.eport)]={0};
	char* port;

	/* these configs depend on other modules */
	sysConfGet("NETCONF_WIRE_MAC", cloud.devinfo.id, sizeof(cloud.devinfo.id));
	sysConfGet("SYSTEM_PRODUCT_NAME", cloud.devinfo.desc, sizeof(cloud.devinfo.desc));
	sysConfGet("SYSTEM_PRODUCT_MODEL", cloud.devinfo.model, sizeof(cloud.devinfo.model));
	sysConfGet("UM_DEFAULT_ADMIN_NAME", cloud.devinfo.admin, sizeof(cloud.devinfo.admin));
	sysConfGet("UM_DEFAULT_ADMIN_PASSWORD", cloud.devinfo.password,
		   	sizeof(cloud.devinfo.password));

	/* get upnp address pair 'ip:port' or 'null' */
	snprintf(msg_buf, sizeof(msg_buf), "%u %u",
			(unsigned int)upnp_addr,
			sizeof(upnp_addr));
	sysMsgDispatch(MSG_UPNP_INFO_STREAM_SERVER_GET, msg_buf, 0);
	port = strchr(upnp_addr, ':');
	if ( port ) {
		*port = '\0';
		port++;
	}
	if ( port ){
		snprintf(cloud.devinfo.eip, sizeof(cloud.devinfo.eip), "%s", upnp_addr);
		snprintf(cloud.devinfo.eport, sizeof(cloud.devinfo.eport), "%s", port);
	} else {
		snprintf(cloud.devinfo.eip, sizeof(cloud.devinfo.eip), "null");
		snprintf(cloud.devinfo.eport, sizeof(cloud.devinfo.eport), "null");
	}

	cmCommand("cloud %s %d %s", cloud.host, cloud.p2p_timeout, cloud.devinfo.stream_port);
	if ( pthread_create(&thread, NULL, cloud_daemon, NULL) != 0 ) {
        CM_MUTEX_DESTROY(cloud.lock);
		return -1;
	}

	return 0;
}

static int cloud_reset()
{
    return swCloudUBind();
}

static int cloud_destroy()
{
    CM_MUTEX_DESTROY(cloud.lock);
	return 0;
}

COMMON_STATUS swCloudRegister()
{
	module_t cloud;

	snprintf(cloud.name, sizeof(cloud.name), "Cloud");
	cloud.init = cloud_init;
	cloud.start = cloud_start;
	cloud.stop = NULL;
	cloud.destroy = cloud_destroy;
	cloud.reset = cloud_reset;
	cloud.save = NULL;
	if ( 0 == sysModRegister(&cloud) ) {
		return SUCCEED;
	}
	return ERROR;
}


int swCloudGetStatus()
{
	return cloud_get_status();
}


int swCloudDeviceBindStatus()
{
	return cloud.binded ? 1 : 0;
}


COMMON_STATUS swCloudRegUsr(const char* account, const char* username, const char* password)
{
	COMMON_STATUS error = ERROR;
	if ( !account || !username || !password ) {
		return ERROR;
	}

	SYSLOG(CMLOG_DEBUG, "register cloud account '%s','%s'...\n", account, username);

	if ( cloud_get_status() != CLOUD_STATUS_CONNECTED ){
		error = cloud_do_connect();
		if ( error != SUCCEED ) {
			goto L_REGUSR_RETURN;
		}
	}
	error = cloud_do_regusr(account, username, password);
L_REGUSR_RETURN:
	if ( !cloud.binded && CLOUD_STATUS_CONNECTED == cloud_get_status() ) {
		cloud_do_disconnect();
        cloud_set_status(CLOUD_STATUS_DISCONNECTED);
	}

	SYSLOG(CMLOG_DEBUG, "register return %d\n", error);
	return error;
}


COMMON_STATUS swCloudBind(const char* user, const char* password, const char* devicename)
{
	COMMON_STATUS error = ERROR;
	if (!user || !password || !devicename) {
		return ERROR;
	}

	SYSLOG(CMLOG_DEBUG, "bind camera to user '%s'...\n", user);

    CM_MUTEX_LOCK(cloud.lock);
	if ( cloud.binded && cloud_get_status() != CLOUD_STATUS_BROKEN ) {
		/* unbind first */
        CM_MUTEX_UNLOCK(cloud.lock);
		error = ERRNO_CLOUD_BINDED;
		goto L_BIND_RETURN;
	}
	cloud_do_disconnect();
	error = cloud_do_connect();
	if ( SUCCEED == error ) {
		snprintf(cloud.account, sizeof(cloud.account), "%s", user);
		snprintf(cloud.password, sizeof(cloud.password), "%s", password);
		snprintf(cloud.devinfo.desc, sizeof(cloud.devinfo.desc), "%s", devicename);
		error = cloud_do_auth();
		if ( SUCCEED == error ) {
			error = cloud_do_bind();
			if ( SUCCEED == error ) {
				cloud_do_update();
                cloud_set_status(CLOUD_STATUS_CONNECTED);
				cloud.binded = 1;
				sysConfSetString(CFG_CLOUD_ACCOUNT, cloud.account);
				sysConfSetString(CFG_CLOUD_PASSWORD, cloud.password);
			}
		}
		if ( SUCCEED != error ) {
			cloud_do_disconnect();
		}
	} else {
		cloud_do_disconnect();
	}

    CM_MUTEX_UNLOCK(cloud.lock);
L_BIND_RETURN:
	SYSLOG(CMLOG_DEBUG, "bind return %d\n", error);
	return error;
}


COMMON_STATUS swCloudUBind()
{
	COMMON_STATUS error = ERROR;

	SYSLOG(CMLOG_DEBUG, "ubind camera from user '%s'...\n", cloud.account);

	if ( !cloud.binded ) {
		error = SUCCEED;
		goto L_UBIND_RETURN;
	}
	if ( cloud_get_status() != CLOUD_STATUS_CONNECTED ){
		/* device is offline */
		error = cloud_do_connect();
		if ( SUCCEED == error ) {
			error = cloud_do_auth();
			if ( error != SUCCEED ) {
                cloud_set_status(CLOUD_STATUS_BROKEN);
			}
		} else {
            cloud_set_status(CLOUD_STATUS_DISCONNECTED);
		}
	} else {
		error = SUCCEED;
	}
	if ( SUCCEED == error ){
		error = cloud_do_ubind();
		if ( SUCCEED == error ) {
			cloud.binded = 0;
            cloud_set_status(CLOUD_STATUS_INIT);
			sysConfSetString(CFG_CLOUD_ACCOUNT, "");
			sysConfSetString(CFG_CLOUD_PASSWORD, "");
		} else {
            cloud_set_status(CLOUD_STATUS_DISCONNECTED);
		}
	}
	cloud_do_disconnect();

L_UBIND_RETURN:
	SYSLOG(CMLOG_DEBUG, "ubind return %d\n", error);
	return error;
}


int swCloudPing()
{
	SYSLOG(CMLOG_DEBUG, "ping cloud...\n");
    int ret = 0;

	if ( cloud_get_status() != CLOUD_STATUS_CONNECTING 
         && cloud_get_status() != CLOUD_STATUS_CONNECTED ) {
        cloud_clear_counter();
        cloud_set_status(CLOUD_STATUS_CONNECTING);
	    SYSLOG(CMLOG_DEBUG, "device is not connected to cloud, retrying...\n");
	}

    ret = cloud.reachable;

	SYSLOG(CMLOG_DEBUG, "ping return %d\n", ret);
    return ret;
}


#define P_USERNAME	"USERNAME:"
#define P_PASSWORD	"PASSWORD:"
#define P_ACCOUNT	"ACCOUNT:"
#define P_DEVICENAME	"DEVICENAME:"
#define C_REGISTER	"REGISTER"
#define C_BIND		"BIND"
static int _cldcmd_parse(char* input, char** cmd, char** pusr, char** ppass, char** pother)
{
	char* p = input;
	const char* other;

	*cmd = input;
	*pusr = strstr(input, P_USERNAME);
	if ( !*pusr ) {
		return -1;
	}
	*pusr += strlen(P_USERNAME);
	*ppass = strstr(input, P_PASSWORD);
	if ( !*ppass ) {
		return -1;
	}
	*ppass += strlen(P_PASSWORD);
	if ( !strncmp(*cmd, C_REGISTER, strlen(C_REGISTER))){
		other = P_ACCOUNT;
	} else if ( !strncmp(*cmd, C_BIND, strlen(C_BIND))){
		other = P_DEVICENAME;
	} else {
		return -1;
	}
	*pother = strstr(input, other);
	if ( !*pother ) {
		return -1;
	}
	*pother += strlen(other);

	if ( !(p = strchr( *pusr, '\n')) ) {
		return -1;
	}
	*p = '\0';
	if ( !(p = strchr( *ppass, '\n')) ) {
		return -1;
	}
	*p = '\0';
	if ( !(p = strchr( *pother, '\n')) ) {
		return -1;
	}
	*p = '\0';

	return 0;
}
/*
 * errno to tddp
 * 2001 - network error
 * 1000 - system error
 * 1001 - db error
 * 1002 - user does not exist
 * 1003 - password error
 * 1004 - protocol error
 * 1005 - timeout
 * 1006 - device binded
 */
void swCloudCommand(char* input, char* output)
{
	COMMON_STATUS retval = ERROR;
	char *cmd, *usr, *pass, *other;
	if (!input || !output) {
		return;
	}

	SYSLOG(CMLOG_DEBUG, "input: '%s'\n", input);
	if (_cldcmd_parse(input, &cmd, &usr, &pass, &other) != 0) {
		SYSLOG(CMLOG_ERR, "protocol error\n");
		sprintf(output, "1004");
		return;
	}
	SYSLOG(CMLOG_DEBUG, "usr '%s', pass '%s', other '%s'\n", usr, pass, other);
	if ( !strncmp(cmd, C_REGISTER, sizeof(C_REGISTER)-1)) {
		retval = swCloudRegUsr(other, usr, pass);
	} else if(!strncmp(cmd, C_BIND, sizeof(C_BIND)-1)) {
		retval = swSystemSetProductNameCheck(other);
		if ( SUCCEED == retval ) {
			retval = swCloudBind(usr, pass, other);
			if (SUCCEED == retval ){
				swSystemSetProductName(other);
			}
		}
	} else {
		SYSLOG(CMLOG_ERR, "unsupported command\n");
	}

	switch (retval) {
	case SUCCEED:
		sprintf(output, "OK\n");
		break;
	case ERRNO_CLOUD_NOUSER:
		sprintf(output, "1002");
		break;
	case ERRNO_CLOUD_AUTH:
		sprintf(output, "1003");
		break;
	case ERRNO_CLOUD_BINDED:
		sprintf(output, "1006");
		break;
	case ERRNO_CLOUD_USERNAME_EXIST:
		sprintf(output, "1008");
		break;
	case ERRNO_CLOUD_ACCOUNT_EXIST:
		sprintf(output, "1009");
		break;
	case ERRNO_CLOUD_NETWORK:
		sprintf(output, "2001");
		break;
	default:
		sprintf(output, "1000");
	}
}


