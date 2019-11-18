/*****************************************************************************************
* File Name: custom.c
*
* Description:
*  Common user application file.
*
*****************************************************************************************/

#include "mqtt_ssl.h"

#include <cyassl/openssl/ssl.h>
#include <cyassl/internal.h>
#include <cyassl/cyassl_config.h>

#include "MQTT/mqtt_api.h"


//if you want use certificate, please define macro 'SSL_CONNECT_NEED_CERT'
//#define SSL_CONNECT_NEED_CERT


#define MQTT_RCV_BUF_NUM 1500
static char g_mqtt_recv_buf[MQTT_RCV_BUF_NUM];


#ifdef SSL_CONNECT_NEED_CERT
static const char* root_ca = 
"-----BEGIN CERTIFICATE-----\n"
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n"
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n"
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n"
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n"
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n"
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n"
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n"
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n"
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n"
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n"
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n"
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n"
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n"
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n"
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n"
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n"
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n"
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n"
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n"
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n"
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n"
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n"
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n"
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n"
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n"
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n"
"-----END CERTIFICATE-----";

static const char *device_priv_key =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEowIBAAKCAQEAhtezpDbU2Y3V2YiZLxqAiJW9tqfc+ab+mknE7vrldePeYMRx\n"
"husKSeeVDT5QwQxt4/JUufLf5++yVszCxD4ON08zlqvKfXnUrCRJ7S7LXY3NhQa/\n"
"uTxd5fakz1BWSuIGgV71mywAOSjeYRZwlAo2oaSbPFomb4S5bncuqssgd5fGkspv\n"
"nyn3yQu4iRpg/XVqkXB9KBfgvJaUwZ+QrwKl2J0wz7DL/CWymV+e/lu0Njce8GCK\n"
"ngdbxLJxLnf+GD71uDivzgeejnyOIEmrAIE45DIwV9nklhMotNx3oAcajUZIKoux\n"
"UIFditl58Nf7BS7nNo2eiafHzz+mniD1NFYXZQIDAQABAoIBADY5M0IL0pgRkzdA\n"
"2Oi6LYm/PWTgJ9o0rxl4JLs9JVy830XmeQiaJ68Ec0m6D1syFladsjh8VFG+b+a7\n"
"vG6RzcGsFcC3IFxIy57VkOzR7nGWfljKF/rzLYc2IZxEYA8IyTjo6LcXgJd7ucEs\n"
"9uRxHvrtaIdicSZTJwYm4xtHR60PQ2t3uZg11menZ7aOFb2A/iJEh4warBJS84tL\n"
"oqoM+qe0J7AmbuKKta/p0ZBsURd0OyTXeKjnNwU5hAV/aft2ObUQ9otIqVJDA2Yp\n"
"km4LpwSGW5dW6+Y4vrjUV0LYzLhmL5fGCIgfWfTPtkOt+kT21WSS/4TV5K4QLs3N\n"
"7k/nb4ECgYEAzH8NWIieBiWPhbHrNGi4Ge0eo0TlQpgVZFxZRv9taXNXBYjjgFVC\n"
"WeB/bhvYTh8LarA73fwOzYBVCkBNADhPu0sMgqVdJzZzJyDo0tYhlOwM0yi0jZKt\n"
"SlNRvyyepDZnB8HkzIU1VsIc0wLd0M6u3ToPgFT7ZSBDfNCFTAA6EXUCgYEAqM21\n"
"soLSwJGm/BfrbTOr0Gf9cG1iri4me2JtOLWDC25Rz+yT/i8Pt6jkN7OdoYqx580u\n"
"2/53xQeb46CjGW8sbxAvROqo+9oVRqiG0enaZPaZCTNlQ8TGoRwirw4/OKT7f+sr\n"
"5LcD99K53Qq6XVF5gf9dOdBzfHbyZvFKLf59wDECgYBHYi5pcj8fTCs+cy30fH2P\n"
"Z0tjmOAij+4fu3DarL0nq0B8SR2JJbnkn847xHKMSp/0nRI42gzcxWhqDKoUYpPK\n"
"gFccn9bVolcuGHnNZzwPvRp492OXa9Ub9du6TTwPfIIYniFdFz7YSyg+rGh7xDoc\n"
"NXFFzeR3l3yXbD6rmgSrJQKBgGgkASTQcRU8xrhgSW5lompAvarO0UKEVllor9l8\n"
"ogw7U13jpqn6fsff+mwi8+zllDxd6brvkxSLTTFxuS3H2LHsWj06SGaYdOfSsNpw\n"
"W7xOJrv2aVMV4zUiFoPJVler8T1J6RqBjoEmEGS+aZuI2F3LHci9QYJYH9RCqHEX\n"
"wJ0BAoGBAJWkG4K6wqK8/gpOynVpRssOYQh1l+7FXa8DQCfhPNB4M3HzsVdU7Z0f\n"
"+4WKXVnrYUZTYhRH6SH/9TxAXTBqPEe33A+3GDLwLfqHrQV5r3QLaqx057KY8vgQ\n"
"3t4QvK/PzPzbHkqGuDbOwLKD6Abp6DW47z7WHD4NCVD0Ax8iucCX\n"
"-----END RSA PRIVATE KEY-----";

static const char *device_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUXrLBTrjlvw9lmYKeRAj8ki3897gwDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE2MTIyODA3MDUw\n"
"OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAIbXs6Q21NmN1dmImS8a\n"
"gIiVvban3Pmm/ppJxO765XXj3mDEcYbrCknnlQ0+UMEMbePyVLny3+fvslbMwsQ+\n"
"DjdPM5aryn151KwkSe0uy12NzYUGv7k8XeX2pM9QVkriBoFe9ZssADko3mEWcJQK\n"
"NqGkmzxaJm+EuW53LqrLIHeXxpLKb58p98kLuIkaYP11apFwfSgX4LyWlMGfkK8C\n"
"pdidMM+wy/wlsplfnv5btDY3HvBgip4HW8SycS53/hg+9bg4r84Hno58jiBJqwCB\n"
"OOQyMFfZ5JYTKLTcd6AHGo1GSCqLsVCBXYrZefDX+wUu5zaNnomnx88/pp4g9TRW\n"
"F2UCAwEAAaNgMF4wHwYDVR0jBBgwFoAU8txoPcBS6ldotERQkqSJUmuAhdIwHQYD\n"
"VR0OBBYEFMr10mo5k3/pguFFRRWEKKjgeTA/MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAft5uR4HP6Mh8h6m5lqqxy1CG6\n"
"FJA7G2Bpry/Mfd0BZ1/nCK5Dzb5AaojB0m+QzdkkToFaJLDXiLwvwlI4cf/yS91+\n"
"2qN9je8G9hG04eZGaLGrcEN1hFKvwJVb3bz69EEGrqOTkOBRqIFibHNGaESZ/6GS\n"
"BN6V73xvcG4TbXtaHszHl6Lq0BB660jjZK5mFheB2Vv9wZNlio7bLCLNz4PCIP9h\n"
"0ezc8R0ELBhCAcFDM0XxlEMUI7TUSNIXdVIbipcvz73Lg3c6HSVg9sx9pTs1kEUh\n"
"SzPCP/pYqq8nCkzgp7TeJ12u7OLEoI9ThmLPtcXk/SGUFYHIoJnNyVDRT1iD\n"
"-----END CERTIFICATE-----";
#endif

static int tcp_connect_ssl_server(char *addrp, unsigned short port)
{
	int fd;	
	struct sockaddr_in addr;

	ip_addr_t dest_addr;
	if(hfnet_is_ipaddress((const char *)(addrp)) !=1 )
	{
		if(hfnet_gethostbyname((const char *)(addrp), &dest_addr) !=HF_SUCCESS)
		{
			u_printf("gethostbyname [%s] fail\r\n", addrp);
			return -1;
		}
	}
	else
		inet_aton((char *)(addrp), (ip_addr_t *) &dest_addr);
	
	memset((char*)&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr=dest_addr.addr;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd<0)
		return -1;

	int tmp=1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp) )<0)
	{
		u_printf("set SOL_SOCKET fail\r\n");
	}	
	
	if(setsockopt(fd, SOL_SOCKET,SO_KEEPALIVE,&tmp,sizeof(tmp))<0)
	{
		u_printf("set SO_KEEPALIVE fail\r\n");
	}
	tmp = 60;//60s
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPIDLE,&tmp,sizeof(tmp))<0)
	{
		u_printf("set TCP_KEEPIDLE fail\r\n");
	}
	tmp = 5;
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPINTVL,&tmp,sizeof(tmp))<0)
	{
		u_printf("set TCP_KEEPINTVL fail\r\n");
	}
	tmp = 3;
	if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPCNT,&tmp,sizeof(tmp))<0)
	{
		u_printf("set TCP_KEEPCNT fail\r\n");
	}
	
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr))< 0)
	{
		close(fd);
		return -1;
	}
	
	return fd;
}

static int mqtt_callback(hfmqtt_event_id_t event, char *topic, unsigned char *data, int data_len)
{
	switch(event)
	{
		case HFMQTT_CONNECTED:
			u_printf("MQTT >>> connected to server\r\n");
			break;

		case HFMQTT_DISCONNECTED:
			u_printf("MQTT >>> disconected\r\n");
			break;

		case HFMQTT_TOPIC_SUB_SUCCESSED:
			u_printf("MQTT >>> sub topic <%s> successed\r\n",topic);
			break;

		case HFMQTT_TOPIC_SUB_DATA:
			u_printf("MQTT >>> recv topic <%s> data_len [%d]\r\n", topic,data_len);
			break;

		case HFMQTT_TOPIC_UNSUB_SUCCESSED:
			u_printf("MQTT >>> unsub topic <%s> successed\r\n",topic);
			break;

		case HFMQTT_PING_RESPONSE:
			u_printf("MQTT >>> ping response\r\n");
			break;
			
		default :
			break;
	}
	
	return 0;
}

static void mqtt_thread(void *arg)
{
	CYASSL_CTX*     ctx = NULL;
	CYASSL*         ssl = NULL;
	int sockfd;
	fd_set fdR;
	int rc, beat;
	struct timeval interval;

	CyaSSL_Init();
	
	hfmqtt_init(mqtt_callback);
	//hfmqtt_set_clientid(char *clientid); //default is station MAC address
	//hfmqtt_set_auth(char *username, char *password);
	//hfmqtt_set_alive_time(300);//default is 300 seconds
	hfmqtt_sub("mqtt_test_topic_1");
	hfmqtt_sub("mqtt_test_topic_2");
	
	while(1)
	{
		ctx = NULL;
		ssl = NULL;
		sockfd = -1;
		msleep(1000);
		
		if(!hfwifi_sta_is_connected())
			continue;

		sockfd=tcp_connect_ssl_server(MQTT_SERVER_ADDR, MQTT_SERVER_PORT);
		if(sockfd<0)
		{
			HF_Debug(DEBUG_LEVEL_LOW, "create socket error\r\n");
			continue;
		}
		
		InitMemoryTracker();//for debug, it can show how many memory used in SSL
		CyaSSL_Debugging_ON();//for debug

		ctx = CyaSSL_CTX_new(CyaTLSv1_2_client_method());
		if (ctx == NULL)
		{
			HF_Debug(DEBUG_LEVEL_LOW, "unable to get ctx\r\n");
			goto EXIT_SLL;
		}

#ifdef SSL_CONNECT_NEED_CERT
		if(SSL_SUCCESS != CyaSSL_CTX_load_verify_buffer(ctx, (const unsigned char*)root_ca, (long)strlen(root_ca),  SSL_FILETYPE_PEM))
		{
			HF_Debug(DEBUG_LEVEL_LOW, "unable to load verify buffer\r\n");
			goto EXIT_SLL;
		}

		if(SSL_SUCCESS != CyaSSL_CTX_use_PrivateKey_buffer(ctx, (const unsigned char*)device_priv_key, (long)strlen(device_priv_key), SSL_FILETYPE_PEM))
		{
			HF_Debug(DEBUG_LEVEL_LOW, "unable to load privatekey buffer\r\n");
			goto EXIT_SLL;
		}

		if(SSL_SUCCESS != CyaSSL_CTX_use_certificate_buffer(ctx, (const unsigned char*)device_cert, (long)strlen(device_cert), SSL_FILETYPE_PEM)) 
		{
			HF_Debug(DEBUG_LEVEL_LOW, "unable to load certificate buffer\r\n");
			goto EXIT_SLL;
		}
#else
		CyaSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);//disable verify certificates
#endif

		ssl = CyaSSL_new(ctx);
		if (ssl == NULL)
		{
			HF_Debug(DEBUG_LEVEL_LOW, "unable to get SSL object\r\n");
			goto EXIT_SLL;
		}
		
		CyaSSL_set_fd(ssl, sockfd);
		if (CyaSSL_connect(ssl) != SSL_SUCCESS)
	 	{
			int  err = CyaSSL_get_error(ssl, 0);
			char buffer[80];
			HF_Debug(DEBUG_LEVEL_LOW, "err = %d, %s\r\n", err,CyaSSL_ERR_error_string(err, buffer));
			HF_Debug(DEBUG_LEVEL_LOW, "SSL_connect failed\r\n");

			goto EXIT_SLL;
	    	}

		HF_Debug(DEBUG_LEVEL_LOW,"SSL connect success----------------------------------------\r\n");
		
		hfmqtt_login((void *)ssl);

		beat = hfsys_get_time();
		while(1)
		{
			if(hfsys_get_time() - beat > 60*1000)//send MQTT ping each 60s
			{
				hfmqtt_ping();
				beat = hfsys_get_time();
			}
			
			interval.tv_sec = 0;
			interval.tv_usec = 100000;

			FD_ZERO(&fdR);
			FD_SET(sockfd,&fdR);
			rc= select(sockfd+1,&fdR,NULL,NULL,&interval);
			if(rc <= 0)
				continue;

			if(!FD_ISSET(sockfd, &fdR))
				continue;

			int rc = CyaSSL_recv(ssl, g_mqtt_recv_buf, MQTT_RCV_BUF_NUM, 0);
			if(rc <= 0)
				break;//socket closed
				
			hfmqtt_data_parse(g_mqtt_recv_buf);
		}
		
	EXIT_SLL:
		CyaSSL_shutdown(ssl);
		CyaSSL_free(ssl);
		CyaSSL_CTX_free(ctx);
		close(sockfd);

		hfmqtt_close();
		CyaSSL_Debugging_OFF();//close debug
		ShowMemoryTracker();//peek into how memory was used

		HF_Debug(DEBUG_LEVEL_LOW,"SSL closed.\r\n");
		msleep(3000);
	}
}

void start_mqtt_ssl_test(void)
{
	if(hfthread_create(mqtt_thread, "MQTT", 2048, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL)!= HF_SUCCESS)
	{
		HF_Debug(DEBUG_ERROR,"create MQTT thread fail\n");
	}

	return;
}

