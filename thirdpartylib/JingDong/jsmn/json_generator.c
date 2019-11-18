/*
 * Copyright 2008-2015, Marvell International Ltd.
 * All Rights Reserved.
 */

/*
 * Simple JSON Generator
 */
#include "hsf.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "json_generator.h"
#include "jsonv2.h"
#include <errno.h>


#define json_e(...)				// wmlog_e("json", ##__VA_ARGS__)
#define json_w(...)				// wmlog_w("json", ##__VA_ARGS__)
#define json_d(...)				// wmlog("json", ##__VA_ARGS__)


/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
	while (in && (unsigned char)*in <= 32)
		in++;
	return in;
}

static const char *rev_skip(const char *in)
{
	while (in && (unsigned char)*in <= 32)
		in--;
	return in;
}

#ifdef CONFIG_JSON_FLOAT
static void print_float(float val, short precision, char *str_val,
			int len_str_val)
{
	int val_int, val_frac;
	int scale = 1;
	short pre_tmp = precision;
	char sign[2] = "";

	while (pre_tmp--)
		scale *= 10;

	if (val < 0) {
		val = -val;
		sign[0] = '-';
		sign[1] = '\0';
	}

	val_int = (int)val;
	val_frac = (int)((val - val_int) * scale);

	snprintf(str_val, len_str_val, "%s%d.%.*d", sign, val_int, precision,
		 val_frac);
}
#else
static void print_float(float val, short precision, char *str_val,
			int len_str_val)
{
	snprintf(str_val, len_str_val, "\"unsupported\"");
}
#endif

const char *verify_json_start(const char *buff)
{
	buff = skip(buff);
	if (*buff != '{') {
		json_e("Invalid JSON document");
		return NULL;
	} else {
		return ++buff;
	}
}

static int verify_buffer_limit(json_str_t *jptr)
{
	/*
	 * Check for buffer overflow condition here, and then copy remaining
	 * data using snprintf. This makes sure there is no mem corruption in
	 * json set operations.
	 */
	if (jptr->free_ptr >= (jptr->len - 1)) {
		json_e("buffer maximum limit reached");
		return -1;
	} else
		return WM_SUCCESS;
}

void json_str_init(json_str_t *jptr, char *buff, int len)
{
	jptr->buff = buff;
	memset(jptr->buff, 0, len);
	jptr->free_ptr = 0;
	jptr->len = len;
}

int json_push_object(json_str_t *jptr, const char *name)
{
	char *buff;

	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	/* From last skip cr/lf */
	buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);
	if (*buff != '{')	/* Element in object */
		jptr->buff[jptr->free_ptr++] = ',';

	snprintf(&jptr->buff[jptr->free_ptr],
		 jptr->len - jptr->free_ptr, "\"%s\":{", name);

	jptr->free_ptr = strlen(jptr->buff);
	return WM_SUCCESS;
}

int json_push_array_object(json_str_t *jptr, const char *name)
{
	char *buff;

	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	/* From last skip cr/lf */
	buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);
	if (*buff != '{')	/* Element in object */
		jptr->buff[jptr->free_ptr++] = ',';

	snprintf(&jptr->buff[jptr->free_ptr],
		 jptr->len - jptr->free_ptr, "\"%s\":[", name);

	jptr->free_ptr = strlen(jptr->buff);
	return WM_SUCCESS;
}

int json_start_object(json_str_t *jptr)
{
	char *buff;

	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	if (jptr->free_ptr) {
		/* This should be first call after json_str_init so free_ptr
		 * should be 0 but if it is not then we add ',' before
		 * starting object as there could have been earlier object
		 * already present as case in array.
		 */
		/* From last skip cr/lf */
		buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

		if (*buff == '}')
			jptr->buff[jptr->free_ptr++] = ',';
	}
	jptr->buff[jptr->free_ptr++] = '{';
	return WM_SUCCESS;
}

int json_close_object(json_str_t *jptr)
{
	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	jptr->buff[jptr->free_ptr++] = '}';

	return WM_SUCCESS;
}

int json_pop_array_object(json_str_t *jptr)
{
	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	jptr->buff[jptr->free_ptr++] = ']';

	return WM_SUCCESS;
}

int json_start_array(json_str_t *jptr)
{
	char *buff;
	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	/* From last skip cr/lf */
	buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

	if (*buff == ']')
		jptr->buff[jptr->free_ptr++] = ',';

	jptr->buff[jptr->free_ptr++] = '[';
	return WM_SUCCESS;
}

int json_close_array(json_str_t *jptr)
{
	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	jptr->buff[jptr->free_ptr++] = ']';
	return WM_SUCCESS;
}

int json_set_array_value(json_str_t *jptr, char *str, int value, float val,
			 json_data_types data)
{
	char *buff;

	if (!verify_json_start(jptr->buff))
		return WM_E_JSON_INVSTART;

	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	/* From last skip cr/lf */
	buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

	if (*buff != '[')	/* Element in object */
		jptr->buff[jptr->free_ptr++] = ',';

	switch (data) {
	case JSON_VAL_STR:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "\"%s\"", str);
		break;
	case JSON_VAL_INT:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "%d", value);
		break;
	case JSON_VAL_FLOAT:
		print_float(val, 2, &jptr->buff[jptr->free_ptr],
			    jptr->len - jptr->free_ptr);
		break;
	case JSON_VAL_BOOL:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "%s",
			 (value == 1) ? "true" : "false");
		break;
	default:
		json_e("Invalid case in array set");
	}

	jptr->free_ptr = strlen(jptr->buff);
	return WM_SUCCESS;
}

int json_set_object_value(json_str_t *jptr, const char *name,
			  const char *str, int64_t value, float val,
			  short precision, json_data_types data)
{
	char *buff;
	char tmp;

	if (!verify_json_start(jptr->buff))
		return -WM_E_JSON_INVSTART;

	if (verify_buffer_limit(jptr) < 0)
		return -WM_E_JSON_OBUF;

	/* From last skip cr/lf */
	buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

	if (*buff != '{')	/* Element in object */
		jptr->buff[jptr->free_ptr++] = ',';

	switch (data) {
	case JSON_VAL_STR:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "\"%s\":\"%s\"",
			 name, str);
		break;

	case JSON_VAL_OBJ:
		tmp = str[value];
		buff = str + value;
		*buff = 0;
		snprintf(&jptr->buff[jptr->free_ptr],
			jptr->len - jptr->free_ptr, "\"%s\":%s",
			name, str);
		*buff = tmp;
		break;

	case JSON_VAL_INT:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "\"%s\":%d", name,
			 (int) value);
		break;

	case JSON_VAL_UINT:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "\"%s\":%u", name,
			 (unsigned) value);
		break;

	case JSON_VAL_UINT_64:
		snprintf(&jptr->buff[jptr->free_ptr],
			jptr->len - jptr->free_ptr, "\"%s\":%llu", name,
			 value);
		break;

	case JSON_VAL_FLOAT:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "\"%s\":", name);
		jptr->free_ptr = strlen(jptr->buff);
		print_float(val, precision, &jptr->buff[jptr->free_ptr],
			    jptr->len - jptr->free_ptr);
		break;
	case JSON_VAL_BOOL:
		snprintf(&jptr->buff[jptr->free_ptr],
			 jptr->len - jptr->free_ptr, "\"%s\":%s",
			 name, (value == 1) ? "true" : "false");
		break;

	default:
		json_e("Invalid case in object set");
	}

	jptr->free_ptr = strlen(jptr->buff);
	return WM_SUCCESS;
}
