/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifndef __MQTTHF_LPB100_
#define __MQTTHF_LPB100_



//#include <stdio.h>
//#include <errno.h>
#include <string.h>
//#include <stdlib.h>
#include <hsf.h>
#include "lwip/netdb.h"

typedef struct __Timer_mqtt__ Timer_mqtt;

struct __Timer_mqtt__ {
	struct timeval end_time;
};

typedef struct Network Network;

struct Network
{
	int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

char expired(Timer_mqtt*);
void countdown_ms(Timer_mqtt*, unsigned int);
void countdown(Timer_mqtt*, unsigned int);
int left_ms(Timer_mqtt*);

void InitTimer(Timer_mqtt*);

int hf_read(Network*, unsigned char*, int, int);
int hf_write(Network*, unsigned char*, int, int);
void hf_disconnect(Network*);
void NewNetwork(Network*);
const char *MQTT_version(void);
int ConnectNetwork(Network*, char*, int);

#endif
