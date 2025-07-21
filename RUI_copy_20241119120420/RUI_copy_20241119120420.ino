// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME680.h>
// #include <RUI.h>


// #define SEALEVELPRESSURE_HPA (1010.0)  
// #define PIN_VBAT A0                    

// uint32_t vbat_pin = PIN_VBAT; 


// #define VBAT_MV_PER_LSB (0.73242188F)  
// #define VBAT_DIVIDER (0.4F)         
// #define VBAT_DIVIDER_COMP (1.73)       

// #define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

// Adafruit_BME680 bme; 

// /** Flag to track BLE UART connection status */
// bool g_BleUartConnected = false;


// uint8_t nodeDeviceEUI[8] = { 0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0B, 0xC3, 0x04 };
// uint8_t nodeAppEUI[8] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xF5, 0x7F };
// uint8_t nodeAppKey[16] = { 0x26, 0xD3, 0xDF, 0x13, 0x3D, 0xA0, 0xCE, 0x9E, 0x0A, 0xDC, 0xC9, 0x1C, 0xAB, 0x0C, 0xDA, 0x95 };

// void bme680_init() {
//     Wire.begin();
//     if (!bme.begin(0x76)) {
//         RUI_LOG("Could not find a valid BME680 sensor, check wiring!");
//         return;
//     }
//     bme.setTemperatureOversampling(BME680_OS_8X);
//     bme.setHumidityOversampling(BME680_OS_2X);
//     bme.setPressureOversampling(BME680_OS_4X);
//     bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
//     bme.setGasHeater(320, 150);  
// }

// float readVBAT(void) {
//     float raw = analogRead(vbat_pin);
//     return raw * REAL_VBAT_MV_PER_LSB;
// }

// uint8_t mvToPercent(float mvolts) {
//     if (mvolts < 3300) {
//         return 0;
//     }
//     if (mvolts < 3600) {
//         mvolts -= 3300;
//         return mvolts / 30;
//     }
//     if (mvolts < 4200) {
//         return 10 + ((mvolts - 3600) * 0.15);
//     }
//     return 100;
// }

// void setup() {
  
//     pinMode(LED_BUILTIN, OUTPUT);
//     digitalWrite(LED_BUILTIN, HIGH);

//     // Initialize Serial for debug output
//     RUI_LOG("Starting RUI3 LoRaWAN Example");

//     bme680_init();

//     RUI_LORA.init();
//     RUI_LORA.setDeviceEUI(nodeDeviceEUI);
//     RUI_LORA.setAppEUI(nodeAppEUI);
//     RUI_LORA.setAppKey(nodeAppKey);
//     RUI_LORA.setClass(A);
//     RUI_LORA.setAdr(true);  // Enable Adaptive Data Rate (ADR)
//     RUI_LORA.setConfirmed(true);  
//     RUI_LORA.join();

//     // Start the LoRaWAN connection
//     RUI_LOG("Joining LoRaWAN Network...");
//     if (RUI_LORA.join() != RUI_OK) {
//         RUI_LOG("Failed to join network!");
//         return;
//     }
//     RUI_LOG("Joined LoRaWAN Network");
//     readVBAT();
// }


// void loop() {
//     // Perform sensor reading from the BME680
//     if (!bme.performReading()) {
//         RUI_LOG("Failed to perform reading");
//         return;
//     }

//     // Read battery voltage and calculate percentage
//     float batteryVoltage_mV = readVBAT();
//     uint8_t batteryPercent = mvToPercent(batteryVoltage_mV);

//     // Prepare sensor data for LoRaWAN transmission
//     String sensorData = 
//         "Battery Voltage: " + String(batteryVoltage_mV / 1000.0, 3) + " V\n" +
//         "Battery Percentage: " + String(batteryPercent) + " %\n" +
//         "Temperature: " + String(bme.temperature) + " °C\n" +
//         "Pressure: " + String(bme.pressure / 100.0) + " hPa\n" +
//         "Humidity: " + String(bme.humidity) + " %\n" +
//         "Gas Resistance: " + String(bme.gas_resistance / 1000.0) + " KOhms\n\n";

//     // Send sensor data over LoRaWAN if connected
//     RUI_LOG("Sending data over LoRaWAN: %s", sensorData.c_str());
//     RUI_LORA.send(sensorData.c_str(), sensorData.length());

//     // Wait before sending next data
//     delay(5000);  // Delay before next sensor reading
// }


#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <RUI.h>

#define SEALEVELPRESSURE_HPA (1010.0)  
#define PIN_VBAT A0                    

uint32_t vbat_pin = PIN_VBAT; 

#define VBAT_MV_PER_LSB (0.73242188F)  
#define VBAT_DIVIDER (0.4F)         
#define VBAT_DIVIDER_COMP (1.73)       

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

Adafruit_BME680 bme; 

/** Flag to track BLE UART connection status */
bool g_BleUartConnected = false;

uint8_t nodeDeviceEUI[8] = { 0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0B, 0xC3, 0x04 };
uint8_t nodeAppEUI[8] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xF5, 0x7F };
uint8_t nodeAppKey[16] = { 0x26, 0xD3, 0xDF, 0x13, 0x3D, 0xA0, 0xCE, 0x9E, 0x0A, 0xDC, 0xC9, 0x1C, 0xAB, 0x0C, 0xDA, 0x95 };

void bme680_init() {
    Wire.begin();
    if (!bme.begin(0x76)) {
        RUI_LOG("Could not find a valid BME680 sensor, check wiring!");
        while (1); // Halt execution if sensor not found
    }
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);  
}

float readVBAT(void) {
    float raw = analogRead(vbat_pin);
    return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
    if (mvolts < 3300) {
        return 0;
    }
    if (mvolts < 3600) {
        mvolts -= 3300;
        return mvolts / 30;
    }
    if (mvolts < 4200) {
        return 10 + ((mvolts - 3600) * 0.15);
    }
    return 100;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // Initialize Serial for debug output
    RUI_LOG("Starting RUI3 LoRaWAN Example");

    bme680_init();

    // Initialize LoRaWAN
    RUI_LORA.init();
    RUI_LORA.setDeviceEUI(nodeDeviceEUI);
    RUI_LORA.setAppEUI(nodeAppEUI);
    RUI_LORA.setAppKey(nodeAppKey);
    RUI_LORA.setClass(A);
    RUI_LORA.setAdr(true);  // Enable Adaptive Data Rate (ADR)
    RUI_LORA.setConfirmed(true);  

    // Start the LoRaWAN connection
    RUI_LOG("Joining LoRaWAN Network...");
    if (RUI_LORA.join() != RUI_OK) {
        RUI_LOG("Failed to join network!");
        while (1); // Halt execution if failed to join
    }
    RUI_LOG("Joined LoRaWAN Network");
}

void loop() {
    // Perform sensor reading from the BME680
    if (!bme.performReading()) {
        RUI_LOG("Failed to perform reading");
        return;
    }

    // Read battery voltage and calculate percentage
    float batteryVoltage_mV = readVBAT();
    uint8_t batteryPercent = mvToPercent(batteryVoltage_mV);

    // Prepare sensor data for LoRaWAN transmission
    String sensorData = 
        "Battery Voltage: " + String(batteryVoltage _mV / 1000.0, 3) + " V\n" +
        "Battery Percentage: " + String(batteryPercent) + " %\n" +
        "Temperature: " + String(bme.temperature) + " °C\n" +
        "Pressure: " + String(bme.pressure / 100.0) + " hPa\n" +
        "Humidity: " + String(bme.humidity) + " %\n" +
        "Gas Resistance: " + String(bme.gas_resistance / 1000.0) + " KOhms\n\n";

    // Send sensor data over LoRaWAN if connected
    RUI_LOG("Sending data over LoRaWAN: %s", sensorData.c_str());
    RUI_LORA.send(sensorData.c_str(), sensorData.length());

    // Wait before sending next data
    delay(5000);  // Delay before next sensor reading
}
