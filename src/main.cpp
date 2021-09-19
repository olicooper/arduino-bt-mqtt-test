#define CORE_DEBUG_LEVEL ARDUHAL_LOG_LEVEL_VERBOSE

#include <Arduino.h>

#include "sdkconfig.h" // used for log printing
#include "esp_system.h"
#include "freertos/FreeRTOS.h" //freeRTOS items to be used
#include "freertos/task.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "esp_log.h"

#include <BluetoothSerial.h>
#include <WiFi.h>
#include <mqtt_client.h>

#if true
#include "secrets.h"
#else
#define BT_DEVICE_NAME "MQTT-TEST"
#define WIFI_DEFAULT_SSID "ssid"
#define WIFI_DEFAULT_PASSWORD "pass"
#define MQTT_DEFAULT_URI "wss://username:password@mqtt.example.com"
#define MQTT_DEFAULT_CLIENT_ID "client1"
#endif

static int totalMem;
extern const uint8_t server_cert_pem_start[] asm("_binary_lets_encrypt_r3_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_lets_encrypt_r3_pem_end");
BluetoothSerial SerialBT;
esp_mqtt_client_handle_t mqttClient;


static void printmem(uint8_t num)
{
    const int free = esp_get_free_heap_size();

    ESP_LOGI(TAG, "MEM[%u]: t=%d,f=%d,lfbk=%d,used=%.2f%%", 
        num,
        totalMem, 
        free, 
        heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT), 
        ((totalMem - free) / (float)totalMem) * 100.00f);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "MQTTEV %s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        printmem(11);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        printmem(10);
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void setup() {
    // note: heap_caps_get_total_size() doesn't exist in arudino v1.0.6 so hard coding it.
    totalMem = 253584;//heap_caps_get_total_size(MALLOC_CAP_DEFAULT);

    printmem(1);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_DEFAULT_SSID, WIFI_DEFAULT_PASSWORD);
    printmem(2);

    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        printmem(3);

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.uri = MQTT_DEFAULT_URI;
        mqtt_cfg.client_id = MQTT_DEFAULT_CLIENT_ID;
        mqtt_cfg.buffer_size = 128;
        mqtt_cfg.network_timeout_ms = 5000;
        mqtt_cfg.keepalive = 60;
        mqtt_cfg.cert_pem = (const char *)server_cert_pem_start;

        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(mqttClient, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
        esp_mqtt_client_start(mqttClient);
        
        printmem(4);
    }

    SerialBT.enableSSP();
    SerialBT.begin(BT_DEVICE_NAME);
    printmem(5);
}

void loop() {
  // put your main code here, to run repeatedly:
}