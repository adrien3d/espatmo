#include "espatmo.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
//#include "protocol_examples_common.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_crt_bundle.h"

static const char *TAG = "espatmo";


void EspAtmo::auth(void)
{
    int ret, len;
    char req[512];
    char buf[4096];

    if (latest_auth.expiration == 0) {
        sprintf(req, "POST %s HTTP/1.0\r\n grant_type: password\r\n client_id: %s\r\n client_secret: %s\r\n username: %s\r\n password: %s\r\n scope: %s\r\n", URL_AUTH_REQ, latest_auth.client_id, latest_auth.client_secret, CONFIG_NETATMO_USERNAME, CONFIG_NETATMO_PASSWORD, latest_auth.scope);
    } else {
        // Check current timestamp, and renew only if latest_auth.expiration < now
        sprintf(req, "POST %s HTTP/1.0\r\n grant_type: refresh_token\r\n client_id: %s\r\n client_secret: %s\r\n refresh_token: %s\r\n scope: %s\r\n", URL_AUTH_REQ, latest_auth.client_id, latest_auth.client_secret, latest_auth.refresh_token, latest_auth.scope);
    }

    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    struct esp_tls *tls = esp_tls_conn_http_new(URL_AUTH_REQ, &cfg);

    if (tls != NULL) {
        ESP_LOGI(TAG, "Connection established...");
    } else {
        ESP_LOGE(TAG, "Connection failed...");
        esp_tls_conn_delete(tls);
        return;
    }

    size_t written_bytes = 0;
    while (written_bytes < strlen(req)) {
        ret = esp_tls_conn_write(tls, req + written_bytes, strlen(req) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
            esp_tls_conn_delete(tls);
            return;
        }
    }

    ESP_LOGI(TAG, "Reading HTTP response...");

    while(1) {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ)
            continue;

        if (ret < 0) {
            ESP_LOGE(TAG, "esp_tls_conn_read returned -0x%x", -ret);
            break;
        }

        if (ret == 0) {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGD(TAG, "%d bytes read", len);
        /* Print response directly to stdout as it is read */
        for (int i = 0; i < len; i++) {
            putchar(buf[i]);
        }
    }
    esp_tls_conn_delete(tls);

    /*
        char text[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";
        cJSON * root   = cJSON_Parse(text);
        cJSON * format = cJSON_GetObjectItem(root,"format"); 
        cJSON * type   = cJSON_GetObjectItem(format,"type");
        int framerate = cJSON_GetObjectItem(format,"frame rate")->valueint;
        char * rendered = cJSON_Print(root);
        printf("%s\n", rendered);
        printf("rate = %d, type = %s \n", framerate, type->valuestring);
        free(rendered);
        cJSON_Delete(root);
    */

    return;
}
