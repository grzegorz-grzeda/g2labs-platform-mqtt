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
#include <MQTTClient.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MQTT_MAX_HANDLER_COUNT 16

static MQTTClient client;
static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

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

static void connection_lost(void* context, char* cause) {
    handle_on_disconnected();
}

static int message_arrived(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    handle_on_message(topicName, strlen(topicName), message->payload, (size_t)message->payloadlen);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
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
    MQTTClient_create(&client, url, username, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if (ca_certificate && client_certificate && client_key) {
        MQTTClient_SSLOptions ssl = MQTTClient_SSLOptions_initializer;
        ssl.struct_version = 4;
        ssl.sslVersion = 3;
        ssl.trustStore = ca_certificate;
        ssl.keyStore = client_certificate;
        ssl.privateKey = client_key;
        conn_opts.ssl = &ssl;
    } else {
        conn_opts.ssl = NULL;
    }
    MQTTClient_setCallbacks(client, NULL, connection_lost, message_arrived, NULL);
    if (MQTTClient_connect(client, &conn_opts) == 0) {
        handle_on_connected();
    } else {
        handle_on_disconnected();
    }
}

void platform_mqtt_disconnect() {
    int return_code = MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    if (return_code == 0) {
        handle_on_disconnected();
    }
}

void platform_mqtt_subscribe(const char* topic) {
    MQTTClient_subscribe(client, topic, 0);
}

void platform_mqtt_send(const char* topic, const void* message, size_t message_length) {
    MQTTClient_publish(client, topic, message_length, (const char*)message, 0, 0, NULL);
}