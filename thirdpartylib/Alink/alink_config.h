
#ifndef _ALINK_CONFIG_H
#define _ALINK_CONFIG_H


#define ALINK_VER "Alink-1.2-0511"

#define ALINK_SDK_USED_USERBIN_ADDR 0
#define ALINK_SDK_USED_USERBIN_SIZE 0x1000



/* 设备信息：根据网页注册信息导出的电子表格更新对应信息 */
#define DEVICE_PRODUCT_VERSION		"1.10"
#define DEVICE_PRODUCT_NAME			"alink_product"
#define DEVICE_PRODUCT_MODEL			"OPENALINK_LIVING_LIGHT_SDS_TEST"
#define DEVICE_PRODUCT_KEY				"1L6ueddLqnRORAQ2sGOL"
#define DEVICE_PRODUCT_SECRET			"qfxCLoc1yXEk9aLdx5F74tl1pdxl0W0q7eYOvvuo"
#define DEVICE_PRODUCT_DEBUG_KEY		"dpZZEpm9eBfqzK7yVeLq"
#define DEVICE_PRODUCT_DEBUG_SECRET	"THnfRRsU5vu6g6m9X6uFyAjUWflgZ0iyGjdEneKm"


typedef enum
{
	HFALI_CLOUD_CONNECTED=0,
	HFALI_CLOUD_DISCONNECTED,
	HFALI_GET_DEVICE_STATUS,
	HFALI_SET_DEVICE_STATUS,
	HFALI_MAIN_LOOP
}hfali_event_id_t;

typedef int (*hfali_callback_t)( hfali_event_id_t event,void *data,unsigned int len);

/**
 * @brief start alink service
 *
 * @param[in] callback event callback function
 * @return[out] none
 * @see None.
 * @note None
 */
void hfali_main_start(hfali_callback_t callback);

/**
 * @brief report alink message
 *
 * @param[in] son_buffer: json format buffer, like
 *              {"OnOff":"1", "Light":"80"}
 * @retval 0 when successfully got response from cloud,
 *          otherwise this func will block until timeout and -1 will return
 * @see None.
 * @note when connection with server is unstable, this func will block
 *      until got response from server or timeout.
 */
int hfali_report_data(const char *json_buffer);

/**
 * @brief start smartconfig
 *
 * @param[in] none
 * @return[out] none
 * @see None.
 * @note this function will reboot.
 */
void hfali_smtlk_start(void);

/**
 * @brief restore factory settings
 *
 * @param[in] none
 * @return[out] none
 * @see None.
 * @note this function will reboot.
 */
void hfali_factory_reset(void);

/**
 * @brief is in smartlink mode
 *
 * @param[in] none.
 * @retval 1-in smartlink mode, other value is not in smartlink mode
 * @see None.
 * @note None.
 */
int alink_is_in_smtlk(void);

/**
 * @brief is in softap mode
 *
 * @param[in] none.
 * @retval 1-in softap mode, other value is not in softap mode
 * @see None.
 * @note None.
 */
int alink_is_in_softap(void);

/**
 * @brief is in add device mode
 *
 * @param[in] none.
 * @retval 1-in add device mode, other value is not in add device mode
 * @see None.
 * @note if in add device mode, must active device first, then post all status.
 */
int alink_is_add_device(void);

/**
 * @brief get UTC time from ali server
 *
 * @param[in] utc_time @n using to store UTC time.
 * @retval 0-successfully, other value is failed
 * @see None.
 * @note None.
 */
int alink_get_utc_time(unsigned int *utc_time);

/**
 * @brief get alink device key, is necessary for SDS
 *
 * @param[in] device_key @n Buffer for using to store key string.
 * @retval The key string
 * @see None.
 * @note None.
 */
char *alink_get_device_key(char device_key[]);

/**
 * @brief set alink device key, is necessary for SDS
 *
 * @param[in] device_key @n Buffer for using to store key string.
 * @retval 0 when set successfully, otherwise -1 will return
 * @see None.
 * @note None.
 */
int alink_set_device_key(char device_key[]);

/**
 * @brief get alink device secret, is necessary for SDS
 *
 * @param[in] device_secret @n Buffer for using to store secret string.
 * @retval The secret string
 * @see None.
 * @note None.
 */
char *alink_get_device_secret(char device_secret[]);

/**
 * @brief set alink device secret, is necessary for SDS
 *
 * @param[in] device_secret @n Buffer for using to store secret string.
 * @retval 0 when set successfully, otherwise -1 will return
 * @see None.
 * @note None.
 */
int alink_set_device_secret(char device_secret[]);

/**
 * @brief get alink debug level
 *
 * @param[in] none
 * @retval debug level is 0-6, 6 mean printf all debug info
 * @see None.
 * @note None.
 */
int alink_get_debug_level(void);

/**
 * @brief set alink debug level
 *
 * @param[in] debug level is 0-6, 6 mean printf all debug info
 * @retval 0 when set successfully, otherwise -1 will return
 * @see None.
 * @note None.
 */
int alink_set_debug_level(int level);

/**
 * @brief set alink run environment. 
 *
 * @param[in] env is 0-3, (0-default, 1-sandbox, 2-online, 3-daily)
 * @retval 0 when set successfully, otherwise -1 will return
 * @see None.
 * @note None.
 */
int alink_set_run_env(int env);

#endif/*_ALINK_CONFIG_H*/

