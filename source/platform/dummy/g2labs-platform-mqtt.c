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

void platform_mqtt_set_on_connect_handler(platform_mqtt_on_connect_handler on_connected) {}

void platform_mqtt_set_on_disconnect_handler(platform_mqtt_on_disconnect_handler on_disconnected) {}

void platform_mqtt_set_on_message_handler(platform_mqtt_on_message_handler on_message) {}

void platform_mqtt_connect(const char* url,
                           const char* username,
                           const char* password,
                           const char* ca_certificate,
                           const char* client_certificate,
                           const char* client_key) {}

void platform_mqtt_disconnect() {}

void platform_mqtt_subscribe(const char* topic) {}

void platform_mqtt_send(const char* topic, const void* message, size_t message_length) {}