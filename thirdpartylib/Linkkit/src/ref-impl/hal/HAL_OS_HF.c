/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */





#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iotx_hal_internal.h"
#include "hsf.h"
#include <time.h>

#define __DEMO__

#ifdef __DEMO__
    char _product_key[PRODUCT_KEY_LEN + 1];
    char _product_secret[PRODUCT_SECRET_LEN + 1];
    char _device_name[DEVICE_NAME_LEN + 1];
    char _device_secret[DEVICE_SECRET_LEN + 1];
#endif

void *HAL_MutexCreate(void)
{
	hfthread_mutex_t mutex;
	if(hfthread_mutext_new(&mutex) == HF_SUCCESS)
	{
		return (void *)mutex;
	}
	else
		return NULL;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
	hfthread_mutext_free((hfthread_mutex_t)mutex);
}

void HAL_MutexLock(_IN_ void *mutex)
{
	hfthread_mutext_wait((hfthread_mutex_t)mutex, 0xFFFFFFFF);
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
	hfthread_mutext_unlock((hfthread_mutex_t)mutex);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
	return hfmem_malloc(size);
}

void *HAL_Realloc(_IN_ void *ptr, _IN_ uint32_t size)
{
	if(ptr==NULL)
		return hfmem_malloc(size);
	
	if(size == 0)
	{
		hfmem_free(ptr);
		return NULL;
	}
	
	char *p=(char *)hfmem_malloc(size);
	if(p==NULL)
		return NULL;
	
	memcpy(p, ptr, size);
	
	if(ptr!=NULL)
		hfmem_free(ptr);
	
	return (void *)p;
}

void *HAL_Calloc(_IN_ uint32_t nmemb, _IN_ uint32_t size)
{
	char *p=(char *)hfmem_malloc(nmemb*size);
	if(p==NULL)
		return NULL;
	
	memset(p, 0, nmemb*size);
	return (void *)p;
}

void HAL_Free(_IN_ void *ptr)
{
	if(ptr)
		hfmem_free(ptr);
}

uint64_t HAL_UptimeMs(void)
{
	return hfsys_get_time();
}

char *HAL_GetTimeStr(_IN_ char *buf, _IN_ int len)
{
	struct timeval tv;
	int str_len    = 0;

	if (buf == NULL || len < 28) {
		return NULL;
	}
	gettimeofday(&tv, NULL);
	time_t now= tv.tv_sec;
	struct tm * nowtm = localtime(&now);

	snprintf(buf, 28, "%d-%d %d:%d:%d", nowtm->tm_mon+1, nowtm->tm_mday, nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
	str_len = strlen(buf);
	if (str_len + 3 < len) {
		snprintf(buf + str_len, len, ".%3.3d", (int)(tv.tv_usec) / 1000);
	}
	return buf;
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
	msleep(ms);
}

void HAL_Srandom(uint32_t seed)
{
	srandom(seed);
}

uint32_t HAL_Random(uint32_t region)
{
	return (region > 0) ? (random() % region) : 0;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = vsnprintf(str, len, fmt, args);
	va_end(args);

	return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
	return vsnprintf(str, len, format, ap);
}

static char hal_log_buf[1024];
void HAL_Printf(_IN_ const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(hal_log_buf, sizeof(hal_log_buf), fmt, args );
	va_end( args );

	printf("%s", hal_log_buf);
}

int HAL_GetPartnerID(char *pid_str)
{
	memset(pid_str, 0x0, PID_STRLEN_MAX);
	strcpy(pid_str, "HF-LPX30");
	
	return strlen(pid_str);
}

int HAL_GetModuleID(char *mid_str)
{
	memset(mid_str, 0x0, MID_STRLEN_MAX);
	hfnet_get_hostname(mid_str);
	
	return strlen(mid_str);
}


char *HAL_GetChipID(_OU_ char *cid_str)
{
	memset(cid_str, 0x0, HAL_CID_LEN);
	hfnet_get_mac_address(cid_str);
	
	return cid_str;
}


int HAL_GetDeviceID(_OU_ char *device_id)
{
	memset(device_id, 0x0, DEVICE_ID_LEN);
#ifdef __DEMO__
	HAL_Snprintf(device_id, DEVICE_ID_LEN, "%s.%s", _product_key, _device_name);
	device_id[DEVICE_ID_LEN - 1] = '\0';
#endif

	return strlen(device_id);
}

int HAL_SetProductKey(_IN_ char *product_key)
{
	int len = strlen(product_key);
#ifdef __DEMO__
	if (len > PRODUCT_KEY_LEN) {
		return -1;
	}
	memset(_product_key, 0x0, PRODUCT_KEY_LEN + 1);
	strncpy(_product_key, product_key, len);
#endif
	return len;
}


int HAL_SetDeviceName(_IN_ char *device_name)
{
	int len = strlen(device_name);
#ifdef __DEMO__
	if (len > DEVICE_NAME_LEN) {
		return -1;
	}
	memset(_device_name, 0x0, DEVICE_NAME_LEN + 1);
	strncpy(_device_name, device_name, len);
#endif
	return len;
}


int HAL_SetDeviceSecret(_IN_ char *device_secret)
{
	int len = strlen(device_secret);
#ifdef __DEMO__
	if (len > DEVICE_SECRET_LEN) {
		return -1;
	}
	memset(_device_secret, 0x0, DEVICE_SECRET_LEN + 1);
	strncpy(_device_secret, device_secret, len);
#endif
	return len;
}


int HAL_SetProductSecret(_IN_ char *product_secret)
{
	int len = strlen(product_secret);
#ifdef __DEMO__
	if (len > PRODUCT_SECRET_LEN) {
		return -1;
	}
	memset(_product_secret, 0x0, PRODUCT_SECRET_LEN + 1);
	strncpy(_product_secret, product_secret, len);
#endif
	return len;
}

int HAL_GetProductKey(_OU_ char *product_key)
{
	int len = strlen(_product_key);
	memset(product_key, 0x0, PRODUCT_KEY_LEN);

#ifdef __DEMO__
	strncpy(product_key, _product_key, len);
#endif

	return len;
}

int HAL_GetProductSecret(_OU_ char *product_secret)
{
	int len = strlen(_product_secret);
	memset(product_secret, 0x0, PRODUCT_SECRET_LEN);

#ifdef __DEMO__
	strncpy(product_secret, _product_secret, len);
#endif

	return len;
}

int HAL_GetDeviceName(_OU_ char *device_name)
{
	int len = strlen(_device_name);
	memset(device_name, 0x0, DEVICE_NAME_LEN);

#ifdef __DEMO__
	strncpy(device_name, _device_name, len);
#endif

	return strlen(device_name);
}

int HAL_GetDeviceSecret(_OU_ char *device_secret)
{
	int len = strlen(_device_secret);
	memset(device_secret, 0x0, DEVICE_SECRET_LEN);

#ifdef __DEMO__
	strncpy(device_secret, _device_secret, len);
#endif

	return len;
}

/*
 * This need to be same with app version as in uOTA module (ota_version.h)

    #ifndef SYSINFO_APP_VERSION
    #define SYSINFO_APP_VERSION "app-1.0.0-20180101.1000"
    #endif
 *
 */
int HAL_GetFirmwareVersion(_OU_ char *version)
{
	char *ver = "app-1.0.0-20181203.1637";
	int len = strlen(ver);
	memset(version, 0x0, FIRMWARE_VERSION_MAXLEN);
#ifdef __DEMO__
	strncpy(version, ver, len);
	version[len] = '\0';
#endif
	return strlen(version);
}

void *HAL_SemaphoreCreate(void)
{
	hfthread_sem_t sem;
	if(hfthread_sem_new(&sem, 0) == HF_SUCCESS)
	{
		return (void *)sem;
	}
	else
		return NULL;
}

void HAL_SemaphoreDestroy(_IN_ void *sem)
{
	hfthread_sem_free((hfthread_sem_t)sem);
}

void HAL_SemaphorePost(_IN_ void *sem)
{
	hfthread_sem_signal((hfthread_sem_t)sem);
}

int HAL_SemaphoreWait(_IN_ void *sem, _IN_ uint32_t timeout_ms)
{
	int ret = hfthread_sem_wait((hfthread_sem_t)sem, timeout_ms);
	if(ret == 1)
		ret = 0;
	else
		ret = -1;
	
	return ret;
}

#define DEFAULT_THREAD_SIZE 4096
int HAL_ThreadCreate(
            _OU_ void **thread_handle,
            _IN_ void *(*work_routine)(void *),
            _IN_ void *arg,
            _IN_ hal_os_thread_param_t *hal_os_thread_param,
            _OU_ int *stack_used)
{
	int stack_size;
	char * name;

	if (!hal_os_thread_param || hal_os_thread_param->stack_size == 0) {
		stack_size = DEFAULT_THREAD_SIZE;
	} else {
		stack_size = hal_os_thread_param->stack_size;
	}
	stack_size = stack_size/4;

	if (!hal_os_thread_param || !hal_os_thread_param->name) {
		name = DEFAULT_THREAD_NAME;
	} else {
		name = hal_os_thread_param->name;
	}

	if(hfthread_create((PHFTHREAD_START_ROUTINE)work_routine, name, stack_size, arg, HFTHREAD_PRIORITIES_MID, (hfthread_hande_t *)thread_handle, NULL) == HF_SUCCESS)
	{
		if (stack_used)
			*stack_used = 0;
		return 0;
	}
	else
	{
		return -1;
	}
}

void HAL_ThreadDetach(_IN_ void *thread_handle)
{
	HAL_Printf("HAL_ThreadDetach unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
}

void HAL_ThreadDelete(_IN_ void *thread_handle)
{
	hfthread_destroy(thread_handle);
}

static uint32_t offset = 0;
void HAL_Firmware_Persistence_Start(void)
{
	offset = 0;
	hfupdate_start(HFUPDATE_SW);
}

int HAL_Firmware_Persistence_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
	hfupdate_write_file(HFUPDATE_SW, offset, (uint8_t *)buffer, length);
	offset += length;
	return 0;
}

int HAL_Firmware_Persistence_Stop(void)
{
	int ret = hfupdate_complete(HFUPDATE_SW, offset);
	if(ret != HF_SUCCESS)
	{
		return ret;
	}

	return 0;
}

int HAL_Config_Write(const char *buffer, int length)
{
	hffile_userbin_write(0, (char *)buffer, length);
	return 0;
}

int HAL_Config_Read(char *buffer, int length)
{
	hffile_userbin_read(0, (char *)buffer, length);
	return 0;
}

void HAL_Reboot(void)
{
	hfsys_reset();
}

uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
	int ip, mask, gw;
	hfnet_get_dhcp_ip (&ip, &mask, &gw);
	sprintf(ip_str, "%d.%d.%d.%d", ((ip>>(8*0))&0xFF), ((ip>>(8*1))&0xFF), ((ip>>(8*2))&0xFF), ((ip>>(8*3))&0xFF));
	return (uint32_t)ip;
}

int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
	HAL_Printf("HAL_Kv_Set unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
	return 0;
}

int HAL_Kv_Get(const char *key, void *buffer, int *buffer_len)
{
	HAL_Printf("HAL_Kv_Get unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
	return 0;
}

int HAL_Kv_Del(const char *key)
{
	HAL_Printf("HAL_Kv_Del unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
	return 0;
}

static long long os_time_get(void)
{
	return hfsys_get_time();
}

static long long delta_time = 0;

void HAL_UTC_Set(long long ms)
{
	delta_time = ms - os_time_get();
}

long long HAL_UTC_Get(void)
{
	return delta_time + os_time_get();
}

void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data)
{
#if 0
    timer_t *timer = NULL;

    struct sigevent ent;

    /* check parameter */
    if (func == NULL) {
        return NULL;
    }

    timer = (timer_t *)malloc(sizeof(time_t));

    /* Init */
    memset(&ent, 0x00, sizeof(struct sigevent));

    /* create a timer */
    ent.sigev_notify = SIGEV_THREAD;
    ent.sigev_notify_function = (void (*)(union sigval))func;
    ent.sigev_value.sival_ptr = user_data;

    printf("HAL_Timer_Create\n");

    if (timer_create(CLOCK_MONOTONIC, &ent, timer) != 0) {
        free(timer);
        return NULL;
    }

    return (void *)timer;
	#endif
	HAL_Printf("HAL_Timer_Create unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
	return NULL;
}

int HAL_Timer_Start(void *timer, int ms)
{
#if 0
    struct itimerspec ts;

    /* check parameter */
    if (timer == NULL) {
        return -1;
    }

    /* it_interval=0: timer run only once */
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    /* it_value=0: stop timer */
    ts.it_value.tv_sec = ms / 1000;
    ts.it_value.tv_nsec = (ms % 1000) * 1000;

    return timer_settime(*(timer_t *)timer, 0, &ts, NULL);
		#endif
		HAL_Printf("HAL_Timer_Start unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
		return 0;
}

int HAL_Timer_Stop(void *timer)
{
#if 0
    struct itimerspec ts;

    /* check parameter */
    if (timer == NULL) {
        return -1;
    }

    /* it_interval=0: timer run only once */
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    /* it_value=0: stop timer */
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;

    return timer_settime(*(timer_t *)timer, 0, &ts, NULL);
		#endif
		HAL_Printf("HAL_Timer_Stop unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
		return 0;
	
}

int HAL_Timer_Delete(void *timer)
{
#if 0
    int ret = 0;

    /* check parameter */
    if (timer == NULL) {
        return -1;
    }

    ret = timer_delete(*(timer_t *)timer);

    free(timer);

    return ret;
			#endif
			HAL_Printf("HAL_Timer_Delete unsupport, error!!!!!!!!!!!!!!!\r\n");
	while(1);
			return 0;
}

int HAL_GetNetifInfo(char *nif_str)
{
	memset(nif_str, 0x0, NIF_STRLEN_MAX);
#ifdef __DEMO__
	/* if the device have only WIFI, then list as follow, note that the len MUST NOT exceed NIF_STRLEN_MAX */
	const char *net_info = "WiFi|03ACDEFF0032";
	strncpy(nif_str, net_info, strlen(net_info));
	/* if the device have ETH, WIFI, GSM connections, then list all of them as follow, note that the len MUST NOT exceed NIF_STRLEN_MAX */
	// const char *multi_net_info = "ETH|0123456789abcde|WiFi|03ACDEFF0032|Cellular|imei_0123456789abcde|iccid_0123456789abcdef01234|imsi_0123456789abcde|msisdn_86123456789ab");
	// strncpy(nif_str, multi_net_info, strlen(multi_net_info));
#endif
	return strlen(nif_str);
}
