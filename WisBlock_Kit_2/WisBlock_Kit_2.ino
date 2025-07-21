#include <Adafruit_TinyUSB.h>

#include "rak1901.h"
#include "rak1902.h"
#include <Arduino.h>


void uplinkRoutine();

#define SMART_FARM_PERIOD   (20000)

#define SMART_FARM_BAND     (RAK_REGION_EU868)
#define SMART_FARM_DEVEUI   {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0B, 0xC3 , 0x04}
#define SMART_FARM_APPEUI   {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xF5, 0x7F}
#define SMART_FARM_APPKEY   {0x26, 0xD3, 0xDF, 0x13, 0x3D, 0xA0, 0xCE, 0x9E, 0x0A, 0xDC, 0xC9, 0x1C, 0xAB, 0x0C, 0xDA, 0x95}

/** Temperature & Humidity sensor **/
rak1901 th_sensor;
/** Air Pressure sensor **/
rak1902 p_sensor;

/** Packet buffer for sending */
uint8_t collected_data[64] = { 0 };

void recvCallback(SERVICE_LORA_RECEIVE_T *data) {
    if (data->BufferSize > 0) {
        Serial.println("Downlink received!");

        for (int i = 0; i < data->BufferSize; i++) {
            Serial.printf("%x ", data->Buffer[i]);
            if (data->Buffer[i] == 0x02) {
                if (LoRaWAN.setDeviceClass(CLASS_C)) {
                    Serial.println("\nSwitched to Class C");
                } else {
                    Serial.println("\nFailed to switch to Class C");
                }
            } else if (data->Buffer[i] == 0x01) {
                if (LoRaWAN.setDeviceClass(CLASS_A)) {
                    Serial.println("\nSwitched to Class A");
                } else {
                    Serial.println("\nFailed to switch to Class A");
                }
            }
        }
        Serial.println("\nDownlink processing complete!");
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("RAKwireless Smart Farm Example");

    uint8_t node_device_eui[8] = SMART_FARM_DEVEUI;
    uint8_t node_app_eui[8] = SMART_FARM_APPEUI;
    uint8_t node_app_key[16] = SMART_FARM_APPKEY;

    if (!LoRaWAN.init()) {
        Serial.println("Failed to initialize LoRaWAN");
        return;
    }

    LoRaWAN.setRegion(SMART_FARM_BAND);
    LoRaWAN.setOTAA(node_device_eui, node_app_eui, node_app_key);
    LoRaWAN.setDeviceClass(CLASS_A);

    if (!LoRaWAN.join()) {
        Serial.println("Failed to join the network");
        return;
    }

    Wire.begin();
    Serial.printf("RAK1901 init %s\r\n", th_sensor.init() ? "success" : "fail");
    Serial.printf("RAK1902 init %s\r\n", p_sensor.init() ? "success" : "fail");

    LoRaWAN.onReceive(recvCallback);
    LoRaWAN.setAdaptiveDataRate(true);
    LoRaWAN.setConfirmedMessageRetries(1);

    Serial.println("LoRaWAN setup complete.");
}

void uplinkRoutine() {
    th_sensor.update();

    float temp_f = th_sensor.temperature();
    float humid_f = th_sensor.humidity();
    float press_f = p_sensor.pressure(MILLIBAR);

    Serial.printf("T %.2f H %.2f P %.2f\r\n", temp_f, humid_f, press_f);

    uint16_t t = (uint16_t)(temp_f * 10.0);
    uint16_t h = (uint16_t)(humid_f * 2);
    uint32_t pre = (uint32_t)(press_f * 10);

    uint8_t data_len = 0;
    collected_data[data_len++] = 0x01;
    collected_data[data_len++] = 0x67;
    collected_data[data_len++] = (uint8_t)(t >> 8);
    collected_data[data_len++] = (uint8_t)t;
    collected_data[data_len++] = 0x02;
    collected_data[data_len++] = 0x68;
    collected_data[data_len++] = (uint8_t)h;
    collected_data[data_len++] = 0x03;
    collected_data[data_len++] = 0x73;
    collected_data[data_len++] = (uint8_t)((pre & 0x0000FF00) >> 8);
    collected_data[data_len++] = (uint8_t)(pre & 0x000000FF);

    Serial.println("Data Packet:");
    for (int i = 0; i < data_len; i++) {
        Serial.printf("0x%02X ", collected_data[i]);
    }
    Serial.println("");

    if (LoRaWAN.send(collected_data, data_len, true)) {
        Serial.println("Data sent successfully.");
    } else {
        Serial.println("Data sending failed.");
    }
}

void loop() {
    LoRaWAN.process();
    delay(SMART_FARM_PERIOD);
    uplinkRoutine();
}
