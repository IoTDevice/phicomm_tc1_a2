
#ifndef _HFSERVICE_DOWNSTREAM_H_
#define _HFSERVICE_DOWNSTREAM_H_

/********************************************************************************
*	Description	: 收到云端数据后的处理行为
*	Name		: down_data_action
*	Input		: data
                              len     
*	Returns		: None
 ********************************************************************************/
void down_data_action(unsigned char *data, int len);


/********************************************************************************
*	Description	: 云数据接收，接收状态控制
*	Name		: hfservice_cloud_data_recv
*	Input		: type 
                              data
                              len
*	Returns		: None
 ********************************************************************************/
void hfservice_cloud_data_recv(enum CLOUD_DATA_TYPE type, unsigned char *data, int *len);	

#endif/*_HFSERVICE_DOWNSTREAM_H_*/

