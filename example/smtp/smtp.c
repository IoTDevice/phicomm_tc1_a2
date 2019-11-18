
#include <hsf.h>
#include <string.h>
#include "smtp.h"


#define DGB u_printf


static char g_base64_out[128];
static char *EncodeBase64(char *buf)
{
	hfcrypto_base64_encode(buf, strlen(buf), g_base64_out, sizeof(g_base64_out));
	return g_base64_out;
}

static int g_tcp_client_fd = -1;
static int tcp_connect(char *host, unsigned short port)
{
	int fd;
	struct sockaddr_in addr;
	ip_addr_t dest_addr;
	
	if ((fd =  socket(AF_INET, SOCK_STREAM, 0))  < 0)	//a TCP socket	
	{
		return -1;
	}

	if(!hfnet_is_ipaddress(host))
	{
		if(hfnet_gethostbyname(host, &dest_addr)!= 0)
		{
			return -2;
		}
	}
	else
		ipaddr_aton(host, (ip_addr_t *) &dest_addr);

	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(addr);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr= dest_addr.addr;

	int on= 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr))< 0)
	{
		close(fd);
		return -3;
	}

	g_tcp_client_fd = fd;
	return fd;
}

static void tcp_close(void)
{
	if(g_tcp_client_fd >= 0)
	{
		close(g_tcp_client_fd);
		g_tcp_client_fd = -1;
	}
}

static int tcp_send(char *buf, int len)
{
	if(g_tcp_client_fd < 0)
		return -1;

	return send(g_tcp_client_fd, buf, len, 0);
}

static int tcp_recv(char *buf, int len, int timeout)
{
	if(g_tcp_client_fd < 0)
		return -1;
	
	fd_set fdR;
	struct timeval tm;
	int ret, recvNum=0, startTime=hfsys_get_time();

	memset(buf, 0, len);
	while (hfsys_get_time() - startTime < timeout && recvNum < len)
	{
		FD_ZERO(&fdR);
		FD_SET(g_tcp_client_fd, &fdR);
		tm.tv_sec= 0;
		tm.tv_usec= 100000;
		ret = select(g_tcp_client_fd+1, &fdR, NULL, NULL, &tm);
		if (ret <= 0)
		{
			if(recvNum > 0)
				break;
		}
		else if (FD_ISSET(g_tcp_client_fd, &fdR))
		{
			ret = recv(g_tcp_client_fd, buf+recvNum, len-recvNum, 0);
			if(ret <= 0)
			{
				close(g_tcp_client_fd);
				g_tcp_client_fd = -1;
				break;
			}
			
			recvNum += ret;
		}
	}

	return recvNum;
}


enum EMAIL_STATE
{
	EMAIL_STATE_START,
	EMAIL_STATE_SEND_EHLO,
	EMAIL_STATE_SEND_AUTH,
	EMAIL_STATE_SEND_USER,
	EMAIL_STATE_SEND_PASSWD,
	EMAIL_STATE_SEND_FROM_ACCOUNT,
	EMAIL_STATE_SEND_TO_ACCOUNT,
	EMAIL_STATE_SEND_DATA_FLAG,
	EMAIL_STATE_SEND_MESSAGE,
	EMAIL_STATE_SEND_QUIT,
	EMAIL_STATE_FAIL,
	EMAIL_STATE_END
};

int SendMail(char *from_account, char *from_password, char *to_account, char *mail_message)
{
	DGB("[EMail] start connect to [%s:%d]\r\n", EMAIL_SERVER_ADDR, EMAIL_SERVER_PORT);
	if(tcp_connect(EMAIL_SERVER_ADDR, EMAIL_SERVER_PORT) < 0)
	{
		DGB("[EMail] connect to failed\r\n");
		return -1;
	}
	DGB("[EMail] connect success\r\n");

	enum EMAIL_STATE state = EMAIL_STATE_START;
	char *mail_buf = (char *)hfmem_malloc(EMAIL_BUF_SIZE);
	if(mail_buf == NULL)
		return -1;
	
	while(1)
	{
		switch(state)
		{
			case EMAIL_STATE_START:
				tcp_send("EHLO HYL-PC\r\n", strlen("EHLO HYL-PC\r\n"));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) <= 0)
				{
					DGB("[EMail] send ehlo failed.\r\n");
					state =EMAIL_STATE_FAIL ;
				}
				else
				{
					DGB("[EMail] EHLO recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_AUTH;
				}
				break;

			case EMAIL_STATE_SEND_AUTH:
				tcp_send("AUTH LOGIN\r\n", strlen("AUTH LOGIN\r\n"));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) <= 0)
				{
					DGB("[EMail] send auth failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] AUTH recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_USER;
				}
				break;

			case EMAIL_STATE_SEND_USER:
				sprintf(mail_buf, "%s\r\n", EncodeBase64(from_account));
				tcp_send(mail_buf, strlen(mail_buf));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send user failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] USER recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_PASSWD;
				}
				break;

			case EMAIL_STATE_SEND_PASSWD:
				sprintf(mail_buf, "%s\r\n", EncodeBase64(from_password));
				tcp_send(mail_buf, strlen(mail_buf));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send password failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] PASSWORD recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_FROM_ACCOUNT;
				}
				break;

			case EMAIL_STATE_SEND_FROM_ACCOUNT:
				sprintf(mail_buf, "MAIL FROM: <%s>\r\n", from_account);
				tcp_send(mail_buf, strlen(mail_buf));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send from account failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] From account recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_TO_ACCOUNT;
				}
				break;

			case EMAIL_STATE_SEND_TO_ACCOUNT:
				sprintf(mail_buf, "RCPT TO:<%s>\r\n", to_account);
				tcp_send(mail_buf, strlen(mail_buf));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send to account failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] To account recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_DATA_FLAG;
				}
				break;

			case EMAIL_STATE_SEND_DATA_FLAG:
				tcp_send("DATA\r\n", strlen("DATA\r\n"));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send data flag failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] Data flag recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_MESSAGE;
				}
				break;

			case EMAIL_STATE_SEND_MESSAGE:
				sprintf(mail_buf, "%s\r\n.\r\n", mail_message);
				tcp_send(mail_buf, strlen(mail_buf));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send mail message failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] mail message recv: %s", mail_buf);
					state = EMAIL_STATE_SEND_QUIT;
				}
				break;

			case EMAIL_STATE_SEND_QUIT:
				tcp_send("QUIT\r\n", strlen("QUIT\r\n"));
				if(tcp_recv(mail_buf, EMAIL_BUF_SIZE, 5000) < 0)
				{
					DGB("[EMail] send quit failed.\r\n");
					state = EMAIL_STATE_FAIL;
				}
				else
				{
					DGB("[EMail] Quit recv: %s", mail_buf);
					state = EMAIL_STATE_END;
				}
				break;
	
			default:
				state = EMAIL_STATE_FAIL;
				break;
		}

		if(state == EMAIL_STATE_FAIL)
		{
			DGB("[EMail] send mail failed.\r\n");
			tcp_close();
			hfmem_free(mail_buf);
			return -1;
		}

		if(state == EMAIL_STATE_END)
		{
			DGB("[EMail] send mail success.\r\n");
			tcp_close();
			hfmem_free(mail_buf);
			return 0;
		}
	}

	return -1;
}

static void smtp_main(void* arg)
{
	char mail_message[512];
	
	while(1)
	{
		sprintf(mail_message, "From: \"%s\"<%s>\r\nTo: <%s>\r\nSubject:%s\r\n\r\n%s", 
			"SYSTEM NOTICE",
			EMAIL_FROM_ACCOUNT,
			EMAIL_TO_ACCOUNT, 
			"This is a test email!",
			"Hello!");
		SendMail(EMAIL_FROM_ACCOUNT, EMAIL_FROM_PASSWD, EMAIL_TO_ACCOUNT, mail_message);

		msleep(300*1000);//every 300 seconds
	}

	hfthread_destroy(NULL);
	return;
}
	
void start_smtp_demo(void)
{
	if(hfthread_create(smtp_main, "smtp_main", 512, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS)
		u_printf("start smtp demo failed\r\n");
}

