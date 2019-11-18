
#include <hsf.h>
#include "lwip/sockets.h"
#include "httpsd.h"

#include <cyassl/openssl/ssl.h>
#include <cyassl/internal.h>
#include <cyassl/cyassl_config.h>

#define HTTPS_SERVER_PORT 443

#define HTTPS_RECV_TIMEOUT 3000
#define HTTPS_RECV_BUF_LEN 2048
static char https_recv_buf[HTTPS_RECV_BUF_LEN];

#define HttpRspDataFormat      "HTTP/1.1 200 OK\r\n"\
"Content-type: text/html\r\n"\
"Content-length: 72\r\n"\
"\r\n"\
"<html><body>Hello (TLS) World, System RunTime:  %02d:%02d:%02d !</body></html>"


const char ca_crt[]=
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDDDCCAfSgAwIBAgIDAQAgMA0GCSqGSIb3DQEBBQUAMD4xCzAJBgNVBAYTAlBM\r\n"
"MRswGQYDVQQKExJVbml6ZXRvIFNwLiB6IG8uby4xEjAQBgNVBAMTCUNlcnR1bSBD\r\n"
"QTAeFw0wMjA2MTExMDQ2MzlaFw0yNzA2MTExMDQ2MzlaMD4xCzAJBgNVBAYTAlBM\r\n"
"MRswGQYDVQQKExJVbml6ZXRvIFNwLiB6IG8uby4xEjAQBgNVBAMTCUNlcnR1bSBD\r\n"
"QTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAM6xwS7TT3zNJc4YPk/E\r\n"
"jG+AanPIW1H4m9LcuwBcsaD8dQPugfCI7iNS6eYVM42sLQnFdvkrOYCJ5JdLkKWo\r\n"
"ePhzQ3ukYbDYWMzhbGZ+nPMJXlVjhNWo7/OxLjBos8Q82KxujZlakE403Daaj4GI\r\n"
"ULdtlkIJ89eVgw1BS7Bqa/j8D35in2fE7SZfECYPCE/wpFcozo+47UX2bu4lXapu\r\n"
"Ob7kky/ZR6By6/qmW6/KUz/iDsaWVhFu9+lmqSbYf5VT7QqFiLpPKaVCjF62/IUg\r\n"
"AKpoC6EahQGcxEZjgoi2IrHu/qpGWX7PNSzVttpd90gzFFS269lvzs2I1qsb2pY7\r\n"
"HVkCAwEAAaMTMBEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQUFAAOCAQEA\r\n"
"uI3O7+cUus/usESSbLQ5PqKEbq24IXfS1HeCh+YgQYHu4vgRt2PRFze+GXYkHAQa\r\n"
"TOs9qmdvLdTN/mUxcMUbpgIKumB7bVjCmkn+YzILa+M6wKyrO7Do0wlRjBCDxjTg\r\n"
"xSvgGrZgFCdsMneMvLJymM/NzD+5yCRCFNZX/OYmQ6kd5YCQzgNUKD73P9P4Te1q\r\n"
"CjqTE5s7FCMTY5w/0YcneeVMUeMBrYVdGjux1XMQpNPyvG5k9VpWkKjHDkx0Dy5x\r\n"
"O/fIR/RpbxXyEV6DHpx8Uq79AtoSqFlnGNu8cN2bsWntgM6JQEhqDjXKKWYVIZQs\r\n"
"6GAqm4VKQPNriiTsBhYscw==\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEtDCCA5ygAwIBAgIRAJOShUABZXFflH8oj+/JmygwDQYJKoZIhvcNAQELBQAw\r\n"
"PjELMAkGA1UEBhMCUEwxGzAZBgNVBAoTElVuaXpldG8gU3AuIHogby5vLjESMBAG\r\n"
"A1UEAxMJQ2VydHVtIENBMB4XDTA4MTAyMjEyMDczN1oXDTI3MDYxMDEwNDYzOVow\r\n"
"fjELMAkGA1UEBhMCUEwxIjAgBgNVBAoTGVVuaXpldG8gVGVjaG5vbG9naWVzIFMu\r\n"
"QS4xJzAlBgNVBAsTHkNlcnR1bSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEiMCAG\r\n"
"A1UEAxMZQ2VydHVtIFRydXN0ZWQgTmV0d29yayBDQTCCASIwDQYJKoZIhvcNAQEB\r\n"
"BQADggEPADCCAQoCggEBAOP7faNyusLwyRSH9WsBTuFuQAe6bSddf/dbLbNax1Ff\r\n"
"q6QypmGHtm4PhtIwApf412lXoRg5XWpkecYBWaw8MUo4fNIE0kso6CBfOweizE1z\r\n"
"2/OuT8dW1Vqnlon686to1COGWSfPCSe8rG5ygxwwct/gounS4XR1Gb0qnnsVVAQb\r\n"
"10M5rVUoxeIau/TA5K44STPMdoWfOUXSpJ7yEoxR+HzkLX/1rF/rFp+xLdG6zJFC\r\n"
"d0wlyZA4b9vwzPuOHpdZPtVgTuYFKO1JeRNLukjbL/ly0znK/h/YNHL1tEDPMQHD\r\n"
"7N4RLRddH7hQ0V4Zp2neBzMoylCV+adUy1SGUEWp+UkCAwEAAaOCAWswggFnMA8G\r\n"
"A1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFAh2zcsH/yT2xc3tu5C84oQ3RnX3MFIG\r\n"
"A1UdIwRLMEmhQqRAMD4xCzAJBgNVBAYTAlBMMRswGQYDVQQKExJVbml6ZXRvIFNw\r\n"
"LiB6IG8uby4xEjAQBgNVBAMTCUNlcnR1bSBDQYIDAQAgMA4GA1UdDwEB/wQEAwIB\r\n"
"BjAsBgNVHR8EJTAjMCGgH6AdhhtodHRwOi8vY3JsLmNlcnR1bS5wbC9jYS5jcmww\r\n"
"aAYIKwYBBQUHAQEEXDBaMCgGCCsGAQUFBzABhhxodHRwOi8vc3ViY2Eub2NzcC1j\r\n"
"ZXJ0dW0uY29tMC4GCCsGAQUFBzAChiJodHRwOi8vcmVwb3NpdG9yeS5jZXJ0dW0u\r\n"
"cGwvY2EuY2VyMDkGA1UdIAQyMDAwLgYEVR0gADAmMCQGCCsGAQUFBwIBFhhodHRw\r\n"
"Oi8vd3d3LmNlcnR1bS5wbC9DUFMwDQYJKoZIhvcNAQELBQADggEBAI3m/UBmo0yc\r\n"
"p6uh2oTdHDAH5tvHLeyDoVbkHTwmoaUJK+h9Yr6ydZTdCPJ/KEHkgGcCToqPwzXQ\r\n"
"1aknKOrS9KsGhkOujOP5iH3g271CgYACEnWy6BdxqyGVMUZCDYgQOdNv7C9C6kBT\r\n"
"Yr/rynieq6LVLgXqM6vp1peUQl4E7Sztapx6lX0FKgV/CF1mrWHUdqx1lpdzY70a\r\n"
"QVkppV4ig8OLWfqaova9ML9yHRyZhpzyhTwd9yaWLy75ArG1qVDoOPqbCl60BMDO\r\n"
"TjksygtbYvBNWFA0meaaLNKQ1wmB1sCqXs7+0vehukvZ1oaOGR+mBkdCcuBWCgAc\r\n"
"eLmNzJkEN0k=\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEtTCCA52gAwIBAgIRAO8FGnQaHZQJ/KXkZA+NPJswDQYJKoZIhvcNAQELBQAw\r\n"
"fjELMAkGA1UEBhMCUEwxIjAgBgNVBAoTGVVuaXpldG8gVGVjaG5vbG9naWVzIFMu\r\n"
"QS4xJzAlBgNVBAsTHkNlcnR1bSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEiMCAG\r\n"
"A1UEAxMZQ2VydHVtIFRydXN0ZWQgTmV0d29yayBDQTAeFw0xNjExMDkwODMzNDRa\r\n"
"Fw0yNjExMDkwODMzNDRaMEQxCzAJBgNVBAYTAkNOMRowGAYDVQQKDBFXb1NpZ24g\r\n"
"Q0EgTGltaXRlZDEZMBcGA1UEAwwQV29TaWduIE9WIFNTTCBDQTCCASIwDQYJKoZI\r\n"
"hvcNAQEBBQADggEPADCCAQoCggEBAKRzU7QtbSdi6uUiqewzx81eEdrg0RROHTs1\r\n"
"eXndSwxxUAVDC+FPYvpgWc+bYMVjUJQEIP+SNzsIGvB/YoabRoN7cLBDzPTgYnW8\r\n"
"Pl/wYWXuGNyr1E7bV9Fec37HlvhE39Ntwp31gjMFwTOZ7Zw0QzS7w9PjO4A4anwb\r\n"
"maBJgrRa3GFSgoJ+WIr5brQ6hEgm7rKRNPx6L9Sj2aSl/EWRPPv73j5xeWGcgOPp\r\n"
"U+8eZmqpX+XfCl34o5OQJWi/F7bACetVhvFtWGuLNcZ0eYwU13jOEx3NNsILzIYP\r\n"
"oWJztxd3aPkQOX6cNbJGTvLRcfmGDM0ASq3/BsCrR0o/ruCcd6cCAwEAAaOCAWYw\r\n"
"ggFiMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFKETVNxWcywngsrIhO/u\r\n"
"vwD9X6tWMB8GA1UdIwQYMBaAFAh2zcsH/yT2xc3tu5C84oQ3RnX3MA4GA1UdDwEB\r\n"
"/wQEAwIBBjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwNQYDVR0fBC4w\r\n"
"LDAqoCigJoYkaHR0cDovL3N1YmNhLmNybC5jZXJ0dW0ucGwvY3RuY2EuY3JsMGsG\r\n"
"CCsGAQUFBwEBBF8wXTAoBggrBgEFBQcwAYYcaHR0cDovL3N1YmNhLm9jc3AtY2Vy\r\n"
"dHVtLmNvbTAxBggrBgEFBQcwAoYlaHR0cDovL3JlcG9zaXRvcnkuY2VydHVtLnBs\r\n"
"L2N0bmNhLmNlcjA5BgNVHSAEMjAwMC4GBFUdIAAwJjAkBggrBgEFBQcCARYYaHR0\r\n"
"cDovL3d3dy5jZXJ0dW0ucGwvQ1BTMA0GCSqGSIb3DQEBCwUAA4IBAQCLBeq0MMgd\r\n"
"qULSuAua1YwHNgbFAAnMXd9iiSxbIKoSfYKsrFggNCFX73ex4b64iIhQ2BBr82/B\r\n"
"MNpC4rEvnr1x0oFv8DBO1GYimQaq8E9hjnO1UYYEPelVsykOpnDLklTsBZ4vhhq/\r\n"
"hq1mbs+6G+vsAjO9jVnuxP6toOTNBqvURRumMF0P165MoFdh0kzSjUts+1d8Llnb\r\n"
"DJaZht0O19k1ZdBBmPD3cwbTI+tChOELAVt4Nb5dDGPWqSxc5Nl2j95T3aK1KL2d\r\n"
"2vV16DSVShJIz04QHatcJlNZLJDbSu70c5fPU8YiJdRpfkubANAmwcDB+uNhtYz+\r\n"
"zEji0KnE2oNA\r\n"
"-----END CERTIFICATE-----\r\n";

const char srv_crt[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFzDCCBLSgAwIBAgIQYOpHNOXV2pp+elrMgh3zmjANBgkqhkiG9w0BAQsFADBE\r\n"
"MQswCQYDVQQGEwJDTjEaMBgGA1UECgwRV29TaWduIENBIExpbWl0ZWQxGTAXBgNV\r\n"
"BAMMEFdvU2lnbiBPViBTU0wgQ0EwHhcNMTcwNDA3MDk1NDM4WhcNMTkwNDA3MDk1\r\n"
"NDM4WjB5MQswCQYDVQQGEwJDTjEqMCgGA1UECgwh5p2t5bee5Lmd6Ziz5bCP5a62\r\n"
"55S15pyJ6ZmQ5YWs5Y+4MRIwEAYDVQQHDAnmna3lt57luIIxEjAQBgNVBAgMCea1\r\n"
"meaxn+ecgTEWMBQGA1UEAwwNKi5qb3lvdW5nLmNvbTCCASIwDQYJKoZIhvcNAQEB\r\n"
"BQADggEPADCCAQoCggEBAL7GeYdC5D/9TbESGs8Yl9rL+XmtyO/RXCRqBtC7Q63l\r\n"
"kkgNB6n4xbKjiOUatMJoHjPkdtfAfRSypQ26NIb624wiPt5Bm/MGsplEyLcSpsXX\r\n"
"pQvm/PgugcsN53GxHflODNwgxJJa0rs7LhTdC0Fw8W5letBDUSAuaqrgszgWC3qR\r\n"
"IV1Yc7JoOxmespdsRnUYwM2RahdawFPuuBHKNAYNUxByKv2X7ahsUUhAoyn3wHfG\r\n"
"e+X9zn3JDFSxoHSpj31iwDjT+I1SDQFTgxLCg1aHM6lEBkVkIQ1nhEUDooNvEcBD\r\n"
"YOqxkJ/ki2rkZu2lx9KdwKafJeK3R/H87FHM1QxJs9ECAwEAAaOCAoMwggJ/MAwG\r\n"
"A1UdEwEB/wQCMAAwPAYDVR0fBDUwMzAxoC+gLYYraHR0cDovL3dvc2lnbi5jcmwu\r\n"
"Y2VydHVtLnBsL3dvc2lnbi1vdmNhLmNybDB3BggrBgEFBQcBAQRrMGkwLgYIKwYB\r\n"
"BQUHMAGGImh0dHA6Ly93b3NpZ24tb3ZjYS5vY3NwLWNlcnR1bS5jb20wNwYIKwYB\r\n"
"BQUHMAKGK2h0dHA6Ly9yZXBvc2l0b3J5LmNlcnR1bS5wbC93b3NpZ24tb3ZjYS5j\r\n"
"ZXIwHwYDVR0jBBgwFoAUoRNU3FZzLCeCysiE7+6/AP1fq1YwHQYDVR0OBBYEFOIn\r\n"
"e5SAjiEt/it2wZWozMeZixsjMA4GA1UdDwEB/wQEAwIFoDCCASAGA1UdIASCARcw\r\n"
"ggETMAgGBmeBDAECAjCCAQUGDCqEaAGG9ncCBQEMAjCB9DCB8QYIKwYBBQUHAgIw\r\n"
"geQwHxYYQXNzZWNvIERhdGEgU3lzdGVtcyBTLkEuMAMCAQEagcBVc2FnZSBvZiB0\r\n"
"aGlzIGNlcnRpZmljYXRlIGlzIHN0cmljdGx5IHN1YmplY3RlZCB0byB0aGUgQ0VS\r\n"
"VFVNIENlcnRpZmljYXRpb24gUHJhY3RpY2UgU3RhdGVtZW50IChDUFMpIGluY29y\r\n"
"cG9yYXRlZCBieSByZWZlcmVuY2UgaGVyZWluIGFuZCBpbiB0aGUgcmVwb3NpdG9y\r\n"
"eSBhdCBodHRwczovL3d3dy5jZXJ0dW0ucGwvcmVwb3NpdG9yeS4wHQYDVR0lBBYw\r\n"
"FAYIKwYBBQUHAwEGCCsGAQUFBwMCMCUGA1UdEQQeMByCDSouam95b3VuZy5jb22C\r\n"
"C2pveW91bmcuY29tMA0GCSqGSIb3DQEBCwUAA4IBAQBLFfHvCEpQLvXTlW0OzGad\r\n"
"NqbM3ho+0V+2DzYKWO5Q3b9VM1wYlwMXC5EJnyOhKpQQz85AMFhVU7Yp/nGAk4gZ\r\n"
"QhnqiTtz6wulQjeB2S19XmS6ZbXvb1xJGmCNI0Lesi1Fpi4Y6pmDgMW5vXBjNtyE\r\n"
"8CE/fjceOQ750OfmrJeYdCTkYwEELDLGTgtd4x5VCGZwvrCWXSmFv1AaOnHHvSgi\r\n"
"9NCz89bmOylhgfTMOfGi1tkUnpG8e6m++bQnPwiuSM/kGeNyjIWQz8QKmTy+Hj2K\r\n"
"8wTxbuxVyJcS/hLx5QYN0wDD20Jl4pkD9Img5iICxK2wTkldAzDh9PPG60Y2VhFw\r\n"
"-----END CERTIFICATE-----\r\n";

const char server_key_rsa[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEowIBAAKCAQEAvsZ5h0LkP/1NsRIazxiX2sv5ea3I79FcJGoG0LtDreWSSA0H\r\n"
"qfjFsqOI5Rq0wmgeM+R218B9FLKlDbo0hvrbjCI+3kGb8waymUTItxKmxdelC+b8\r\n"
"+C6Byw3ncbEd+U4M3CDEklrSuzsuFN0LQXDxbmV60ENRIC5qquCzOBYLepEhXVhz\r\n"
"smg7GZ6yl2xGdRjAzZFqF1rAU+64Eco0Bg1TEHIq/ZftqGxRSECjKffAd8Z75f3O\r\n"
"fckMVLGgdKmPfWLAONP4jVINAVODEsKDVoczqUQGRWQhDWeERQOig28RwENg6rGQ\r\n"
"n+SLauRm7aXH0p3App8l4rdH8fzsUczVDEmz0QIDAQABAoIBABTJUeEJyg5p1l8C\r\n"
"gr7JZnRd0LUwGQp+VIyYK91u5gkq0CU/HDRqKXpVJLuXzRW4m//Z9xP7gnVwoJjO\r\n"
"Dt6dMzJB4fk8C1avxmJ6UudVpiu82xwgozQUdyptfagO3R32oeKTRG/Q6xCg+lym\r\n"
"GA2dlYTKlFbgWtTHkfJD8OsdxcrpGJKESkeHyj7Pe2cLYUFRSwhM7R+2MM6o0Lev\r\n"
"KsEc53GnKYRAPA65nqzqsfYkdfHqemxnIMVtQnv5S+PQvFctNyd4N0rCvq+rO5lV\r\n"
"FJZnKdFp7hTIqraa2ikbYczfEJtJqNVumg1jtZd2/wqTqZX+syVf86AGljCY634T\r\n"
"4+3r83UCgYEA6DrfJcLR9sSzZJQasScqBS17Y8ufsviono+JcJ6EjHoix0cz09Qz\r\n"
"1phX+dS/NLRnXscgBFqwDb+11ahrDe+Zk274geXw11Vnx8wOxBu9HfeTfbKupXtn\r\n"
"xWpe3v6smCegAachwmU+qy+PtJ29NCHv3jPdMVMc3giPzDxupNnJ3fUCgYEA0k1d\r\n"
"cmcAtNPErVRpWbIdgV+ulJmId2GLFJ/J/hc7dkC/uKCu9rlIEaAvHLlKpbkxVpx5\r\n"
"2ZGdaWiHGnsOWB0dS6GcVtQMWCLSeWOy5f5hkogxTxaFyJp7+ebEvuuuuyeAjEMW\r\n"
"yuLnO3A3XJBcS/cmtsCxe2MGPE2kguD3sPPPWO0CgYAKlZxUEUQWljKC9AkfZ+SP\r\n"
"ZdpI2yAFPTYaO/qMqnzqFir9oC4pinNuZXUlCuBIG6zWcgbwi92YUtOL0GhIs1Hx\r\n"
"mU40RXGrrtXTEmbd41HESuNrCDjTjANXyRhX242sRaoSSTbNsGWh0cIf/kyAUsem\r\n"
"/gAn3lzjdcpwCZUVaGO9/QKBgDDWHHQ7rTNQ1iR2LLzSmv13LcB+Yu1uLnEMpkim\r\n"
"Nn3AjYUfc2ICJtVJAIfIE6imd+moDrfSk95tkqo4l10o7aupjPew6GjzNTNRTrMQ\r\n"
"PrJbhC/ciBbMoSuoRlobuwjlJHnt/nYA+TeZWJHbSEOv0kKwPsbreQA5+5EVEw/5\r\n"
"nlYFAoGBAMp7n2i/V6/U+8GuUOL6DD6IVww/MDsJNRlX9FwFk9aECwSswPo8I9hy\r\n"
"7cD7ry4LQlMzhqHkvzTTsUb87L5oxXXljbjpf0NHYwo82/MoEhXbalDugLqC6x6c\r\n"
"vjVTTowObAmEdbtLrd4KNLKDUFX2lg/xRSohREGnEuDFPxoynBEm\r\n"
"-----END RSA PRIVATE KEY-----\r\n";


static CYASSL_CTX* ctx = NULL;
static CYASSL* ssl = NULL;

static int ssl_server_init(void)
{
	static int init_flag = 0;
	if(init_flag == 0)
	{
		init_flag = 1;
		CyaSSL_Init();
	}

	InitMemoryTracker();//for debug, it can show how many memory used in SSL
	CyaSSL_Debugging_ON();//for debug

	ctx = CyaSSL_CTX_new(CyaTLSv1_2_server_method());
	if (ctx == NULL)
	{
		u_printf("unable to get ctx\r\n");
		return -1;
	}

	//if(SSL_SUCCESS != CyaSSL_CTX_load_verify_buffer(ctx, (const unsigned char*)ca_crt, (long)sizeof(ca_crt),  SSL_FILETYPE_PEM))
	if(SSL_SUCCESS != CyaSSL_CTX_use_certificate_chain_buffer(ctx, (const unsigned char*)ca_crt, (long)sizeof(ca_crt)))
	{
		u_printf("unable to load verify buffer\r\n");
		
		CyaSSL_CTX_free(ctx);
		ctx = NULL;
		return -1;
	}

	if(SSL_SUCCESS != CyaSSL_CTX_use_certificate_buffer(ctx, (const unsigned char*)srv_crt, (long)sizeof(srv_crt), SSL_FILETYPE_PEM)) 
	{
		u_printf("unable to load certificate buffer\r\n");
		
		CyaSSL_CTX_free(ctx);
		ctx = NULL;
		return -1;
	}
	
	if(SSL_SUCCESS != CyaSSL_CTX_use_PrivateKey_buffer(ctx, (const unsigned char*)server_key_rsa, (long)sizeof(server_key_rsa), SSL_FILETYPE_PEM))
	{
		u_printf("unable to load privatekey buffer\r\n");
		
		CyaSSL_CTX_free(ctx);
		ctx = NULL;
		return -1;
	}

	ssl = CyaSSL_new(ctx);
	if (ssl == NULL)
	{
		u_printf("unable to get SSL object");
		
		CyaSSL_CTX_free(ctx);
		ctx = NULL;
		return -1;
	}

	return HF_SUCCESS;
}

static int ssl_server_accept(int fd)
{
	CyaSSL_set_fd(ssl, fd);
	if (CyaSSL_accept(ssl) != SSL_SUCCESS)
 	{
		int  err = CyaSSL_get_error(ssl, 0);
		char buffer[80];
		HF_Debug(DEBUG_LEVEL_LOW, "err = %d, %s\n", err,CyaSSL_ERR_error_string(err, buffer));
		HF_Debug(DEBUG_LEVEL_LOW, "CyaSSL_accept failed");
		return -1;
	}
	else
		HF_Debug(DEBUG_LEVEL_LOW, "CyaSSL_accept successed ----------------------------------------\r\n");

	return HF_SUCCESS;
}

static int ssl_server_destroy(void)
{
	u_printf("ssl_server_destroy\r\n");
	
	if(ssl)
	{
		CyaSSL_shutdown(ssl);
		CyaSSL_free(ssl);
	}
	ssl = NULL;
	
	if(ctx)
		CyaSSL_CTX_free(ctx);
	ctx = NULL;
	
	CyaSSL_Debugging_OFF();//close debug
	ShowMemoryTracker();//peek into how memory was used
	
	return HF_SUCCESS;
}

static int http_get_alldata_len(char *data, int len)
{
	char *p1 = NULL, *p2 = NULL;
	int headLen=0, bodyLen = 0;
	
	p1 = strstr(data,"\r\n\r\n");
	if(p1 == NULL)
	{
		return -1;
	}
	headLen = p1-data + 4;
	
	p1 = strstr(data,"Content-Length");
	if(p1 == NULL)
	{
		p1 = strstr(data,"Content-length");
		if(p1 == NULL)
			return -1;
	}
	else
	{
		p2 = p1+strlen("Content-Length")+ 2; 
		bodyLen = atoi(p2);
		return bodyLen;
	}
	
	return headLen + bodyLen;
}

static int https_recv_data(int fd, char *buffer, int len, int timeout_ms)
{
	fd_set fdR;
	struct timeval timeout;
	int ret, tmpLen, contenLen, recvLen = 0;
	
	while(1)
	{
		FD_ZERO(&fdR);
		FD_SET(fd,&fdR);
		if(recvLen <= 0)
		{
			timeout.tv_sec = timeout_ms/1000;
			timeout.tv_usec = timeout_ms%1000*1000;
		}
		else
		{
			//fast close
			timeout.tv_sec = 0;
			timeout.tv_usec =500*1000;
		}
		
		ret = select(fd+1, &fdR, NULL, NULL, &timeout);	
		if (ret <= 0)
		{
			break;
		}
		else if(FD_ISSET(fd, &fdR))
		{
			tmpLen = CyaSSL_read(ssl, buffer+recvLen, len-recvLen);
			if(tmpLen <= 0)
				break;

			recvLen += tmpLen;
			contenLen = http_get_alldata_len(buffer, recvLen);
			if((contenLen > 0) && (recvLen > contenLen))//recv all data
				break;
		}
	}

	//ignore favicon.ico
	if((recvLen > strlen("GET /favicon.ico")) && (memcmp(buffer, "GET /favicon.ico", strlen("GET /favicon.ico"))==0))
		return 0;

	//send HTML response
	if(recvLen > 0)
	{
		int hour, min, secnod;
		hour = (hfsys_get_time()/1000)/3600;
		min = (hfsys_get_time()/1000 - hour*3600)/60;
		secnod = (hfsys_get_time()/1000 - hour*3600 - min*60);

		len = snprintf(buffer, len, HttpRspDataFormat, hour, min, secnod);
		CyaSSL_write(ssl, buffer, len);
		msleep(1000);//wait send data out
	}
	
	return recvLen;
}

static void https_server(void)
{
	struct sockaddr_in local_addr, remote_addr;
	int listenfd, remotefd;
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		u_printf("HTTPS: socket creat failed!\r\n");
		hfthread_destroy(NULL);
		return ;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(HTTPS_SERVER_PORT);
	local_addr.sin_len = sizeof(local_addr);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listenfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
	{
		u_printf("HTTPS: socket bind failed!\r\n");
		hfthread_destroy(NULL);
		return ;
	}
	if(listen(listenfd, 10) == -1)
	{
		u_printf("HTTPS: socket listen failed!\r\n");
		hfthread_destroy(NULL);
		return;
	}

	u_printf("HTTPS: https_server start, port=%d\r\n", HTTPS_SERVER_PORT);
	
	int len = sizeof(remote_addr);
	while(1)
	{
		if(ssl_server_init() != HF_SUCCESS)
			break;
		
		remotefd = accept(listenfd, (struct sockaddr *)&remote_addr, (socklen_t *)(&len));
		if(remotefd >= 0)
		{
			if(ssl_server_accept(remotefd) == HF_SUCCESS)
			{
				memset(https_recv_buf, 0, HTTPS_RECV_BUF_LEN);
				https_recv_data(remotefd, https_recv_buf, HTTPS_RECV_BUF_LEN-1, HTTPS_RECV_TIMEOUT);
			}
		}

		ssl_server_destroy();
		close(remotefd);
	}

	u_printf("HTTPS: https_server exit!\r\n");
	hfthread_destroy(NULL);
	return;
}

int start_https_server_test(void)
{
	if(hfthread_create((PHFTHREAD_START_ROUTINE)https_server, "https_server", 2048, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS)
	{
		return -1;
	}

	return HF_SUCCESS;
}

