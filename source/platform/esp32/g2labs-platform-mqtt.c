/*
 * MIT License
 *
 * Copyright (c) 2023 G2Labs Grzegorz Grzęda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "g2labs-platform-mqtt.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// #define LOG_MODULE_NAME "platform-mqtt"
// #define LOG_MODULE_LEVEL (LOG_MODULE_LEVEL_INFO)
// #include <log.h>

#define MQTT_MAX_HANDLER_COUNT 16

static char local_uri[200] = {0};
static char local_username[200] = {0};
static char local_password[200] = {0};

static platform_mqtt_on_connect_handler on_connect_handler = NULL;
static platform_mqtt_on_disconnect_handler on_disconnect_handler = NULL;
static platform_mqtt_on_message_handler on_message_handler = NULL;

static void handle_on_connected(void) {
    if (on_connect_handler) {
        on_connect_handler();
    }
}

static void handle_on_disconnected(void) {
    if (on_disconnect_handler) {
        on_disconnect_handler();
    }
}

static void handle_on_message(const char* topic, size_t topic_size, void* message, size_t message_size) {
    if (on_message_handler) {
        on_message_handler(topic, topic_size, message, message_size);
    }
}

static esp_mqtt_client_handle_t local_client = NULL;

static void log_error_if_nonzero(const char* message, int error_code) {
    if (error_code != 0) {
        // E("Last error %s: 0x%x", message, error_code);
    }
}
static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            // D("MQTT_EVENT_CONNECTED");
            handle_on_connected();
            break;
        case MQTT_EVENT_DISCONNECTED:
            // D("MQTT_EVENT_DISCONNECTED");
            handle_on_disconnected();
            break;
        case MQTT_EVENT_DATA:
            // D("MQTT_EVENT_DATA");
            // D("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            // D("DATA=%.*s\r\n", event->data_len, event->data);
            handle_on_message(event->topic, event->topic_len, event->data, event->data_len);
            break;
        case MQTT_EVENT_ERROR:
            // E("MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",
                                     event->error_handle->esp_transport_sock_errno);
                // I("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            // I("Other event id:%d", event->event_id);
            break;
    }
}

void platform_mqtt_set_on_connect_handler(platform_mqtt_on_connect_handler on_connected) {
    on_connect_handler = on_connected;
}
void platform_mqtt_set_on_disconnect_handler(platform_mqtt_on_disconnect_handler on_disconnected) {
    on_disconnect_handler = on_disconnected;
}
void platform_mqtt_set_on_message_handler(platform_mqtt_on_message_handler on_message) {
    on_message_handler = on_message;
}

void platform_mqtt_connect(const char* url,
                           const char* username,
                           const char* password,
                           const char* ca_certificate,
                           const char* client_certificate,
                           const char* client_key) {
    strcpy(local_uri, url);
    strcpy(local_username, username);
    strcpy(local_password, password);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = local_uri,
        .credentials.username = local_username,
        .credentials.client_id = local_username,
        .credentials.authentication.certificate = client_certificate,
        .credentials.authentication.key = client_key,
        .broker.verification.certificate = ca_certificate,
    };
    mqtt_cfg.broker.address.uri = local_uri;
    local_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(local_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(local_client);
}

void platform_mqtt_disconnect() {
    esp_mqtt_client_disconnect(local_client);
}

void platform_mqtt_subscribe(const char* topic) {
    esp_mqtt_client_subscribe(local_client, topic, 0);
}

void platform_mqtt_send(const char* topic, const void* message, size_t message_length) {
    esp_mqtt_client_publish(local_client, topic, (const char*)message, message_length, 0, 0);
}
