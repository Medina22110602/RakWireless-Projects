#include <Adafruit_TinyUSB.h>

#include "rak1901.h"
#include "rak1902.h"

void uplink_routine();

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

        // Initialize the progress bar
        Serial.print("Progress: [");
        for (int i = 0; i < 10; i++) {
            Serial.print(" "); // Empty progress bar
        }
        Serial.print("] 0%\r");
        Serial.flush();

        // Process received data
        for (int i = 0; i < data->BufferSize; i++) {
            Serial.printf("%x ", data->Buffer[i]);

            // Check for downlink commands
            if (data->Buffer[i] == 0x02) {
                // Switch to Class C
                if (api.lorawan.deviceClass.set(RAK_LORA_CLASS_C)) {
                    Serial.println("\nSwitched to Class C");
                    
                    // Update the progress bar to 1%
                    Serial.print("Progress: [");
                    Serial.print("#"); // 1% completed
                    for (int j = 1; j < 10; j++) {
                        Serial.print(" "); // Remaining 90% empty
                    }
                    Serial.print("] 1%\r");
                    Serial.flush();
                } else {
                    Serial.println("\nFailed to switch to Class C");
                }
            } else if (data->Buffer[i] == 0x01) {
                // Switch to Class A
                if (api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
                    Serial.println("\nSwitched to Class A");
                } else {
                    Serial.println("\nFailed to switch to Class A");
                }
            }
        }

        Serial.println("Downlink processing complete!");
    }
}



void setup()
{
    Serial.begin(115200, RAK_CUSTOM_MODE);
    Serial.printf("Set device class to Class_A  %s\r\n", api.lorawan.deviceClass.set(0) ? "Success" : "Fail");

    delay(2000);

    Serial.println("RAKwireless Smart Farm Example");
    Serial.println("------------------------------------------------------");

    // OTAA Device EUI MSB first
    uint8_t node_device_eui[8] = SMART_FARM_DEVEUI;
    // OTAA Application EUI MSB first
    uint8_t node_app_eui[8] = SMART_FARM_APPEUI;
    // OTAA Application Key MSB first
    uint8_t node_app_key[16] = SMART_FARM_APPKEY;

    if (!api.system.lpm.set(1)) {
        Serial.printf("LoRaWan Smart Farm - set low power mode is incorrect! \r\n");
        return;
    }

    if (!api.lorawan.nwm.set()) {
        Serial.printf("LoRaWan Smart Farm - set network working mode is incorrect! \r\n");
        return;
    }

    if (!api.lorawan.appeui.set(node_app_eui, 8)) {
        Serial.printf("LoRaWan Smart Farm - set application EUI is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.appkey.set(node_app_key, 16)) {
        Serial.printf("LoRaWan Smart Farm - set application key is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.deui.set(node_device_eui, 8)) {
        Serial.printf("LoRaWan Smart Farm - set device EUI is incorrect! \r\n");
        return;
    }

    if (!api.lorawan.band.set(SMART_FARM_BAND)) {
        Serial.printf("LoRaWan Smart Farm - set band is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
        Serial.printf("LoRaWan Smart Farm - set device class is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.njm.set(RAK_LORA_OTAA))    // Set the network join mode to OTAA
    {
        Serial.printf("LoRaWan Smart Farm - set network join mode is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.join())    // Join to Gateway
    {
        Serial.printf("LoRaWan Smart Farm - join fail! \r\n");
        return;
    }

    Serial.println("++++++++++++++++++++++++++");
    Serial.println("RUI3 Environment Sensing");
    Serial.println("++++++++++++++++++++++++++");

    Wire.begin();            // Start I2C Bus
    Serial.printf("RAK1901 init %s\r\n", th_sensor.init() ? "success" : "fail");    // Check if RAK1901 init success
    Serial.printf("RAK1902 init %s\r\n", p_sensor.init() ? "success" : "fail");    // Check if RAK1902 init success

    /** Wait for Join success */
    while (api.lorawan.njs.get() == 0) {
        Serial.print("Wait for LoRaWAN join...");
        api.lorawan.join();
        delay(10000);
    }

    if (!api.lorawan.adr.set(true)) {
        Serial.printf("LoRaWan Smart Farm - set adaptive data rate is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.rety.set(1)) {
        Serial.printf("LoRaWan Smart Farm - set retry times is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.cfm.set(1)) {
        Serial.printf("LoRaWan Smart Farm - set confirm mode is incorrect! \r\n");
        return;
    }

    /** Check LoRaWan Status*/
    Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");    // Check Duty Cycle status
    Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED");    // Check Confirm status
    uint8_t assigned_dev_addr[4] = { 0 };
    api.lorawan.daddr.get(assigned_dev_addr, 4);
    Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);    // Check Device Address
    Serial.printf("Uplink period is %ums\r\n", SMART_FARM_PERIOD);
    Serial.println("");
    api.lorawan.registerRecvCallback(recvCallback);
    api.lorawan.registerJoinCallback(joinCallback);
    api.lorawan.registerSendCallback(sendCallback);
    if (api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)uplink_routine, RAK_TIMER_PERIODIC) != true) {
        Serial.printf("LoRaWan Smart Farm - Creating timer failed.\r\n");
        return;
    }
    if (api.system.timer.start(RAK_TIMER_0, SMART_FARM_PERIOD, NULL) != true) {
        Serial.printf("LoRaWan Smart Farm - Starting timer failed.\r\n");
        return;
    }
}

void uplink_routine()
{
    /** Get sensor RAK1901 values */
    th_sensor.update();

    float temp_f = th_sensor.temperature();
    float humid_f = th_sensor.humidity();
    float press_f = p_sensor.pressure(MILLIBAR);
    Serial.printf("T %.2f H %.2f P %.2f\r\n", temp_f, humid_f, press_f);

    uint16_t t = (uint16_t)(temp_f * 10.0);
    uint16_t h = (uint16_t)(humid_f * 2);
    uint32_t pre = (uint32_t)(press_f * 10);

    /** Cayenne Low Power Payload */
    uint8_t data_len = 0;
    collected_data[data_len++] = 0x01;    // Data Channel: 1
    collected_data[data_len++] = 0x67;    // Type: Temperature Sensor
    collected_data[data_len++] = (uint8_t)(t >> 8);
    collected_data[data_len++] = (uint8_t)t;
    collected_data[data_len++] = 0x02;    // Data Channel: 2
    collected_data[data_len++] = 0x68;    // Type: Humidity Sensor
    collected_data[data_len++] = (uint8_t)h;
    collected_data[data_len++] = 0x03;    // Data Channel: 3
    collected_data[data_len++] = 0x73;    // Type: Barometer
    collected_data[data_len++] = (uint8_t)((pre & 0x0000FF00) >> 8);
    collected_data[data_len++] = (uint8_t)(pre & 0x000000FF);

    Serial.println("Data Packet:");
    for (int i = 0; i < data_len; i++) {
        Serial.printf("0x%02X ", collected_data[i]);
    }
    Serial.println("");

    /** Send the data package */
    if (api.lorawan.send(data_len, (uint8_t*)&collected_data, 2, true, 1)) {
        Serial.println("Sending is requested");
    } else {
        Serial.println("Sending failed");
    }
}

void loop()
{
    // Check if data is available in the serial input
    if (Serial.available()) {
        char input = Serial.read(); // Read a character from the serial

        if (input == 'A') {
            // Switch to Class A
            if (api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
                Serial.println("Switched to Class A");
            } else {
                Serial.println("Failed to switch to Class A");
            }
        }
        else if (input == 'C') {
            // Switch to Class C
            if (api.lorawan.deviceClass.set(RAK_LORA_CLASS_C)) {
                Serial.println("Switched to Class C");
            } else {
                Serial.println("Failed to switch to Class C");
            }
        }
    }

    switch (api.lorawan.deviceClass.get()) {
        case 0:
            Serial.println("Device is in Class A");
            break;

        case 2:
            Serial.println("Device is in Class C");
            break;

        default:
            Serial.println("Device is in an unknown class");
            break;
    }

}
