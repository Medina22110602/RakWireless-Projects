#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <RUI.h>

#define SEALEVELPRESSURE_HPA (1010.0)  
#define PIN_VBAT A0                    

// Pin definition for battery voltage
uint32_t vbat_pin = PIN_VBAT; 

// Constants for voltage measurement
#define VBAT_MV_PER_LSB (0.73242188F)  
#define VBAT_DIVIDER (0.4F)         
#define VBAT_DIVIDER_COMP (1.73)       

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

Adafruit_BME680 bme;  // Create an instance of the BME680 sensor

/** Flag to track BLE UART connection status */
bool g_BleUartConnected = false;

void bme680_init() {
    Wire.begin();
    if (!bme.begin(0x76)) {
        RUI_LOG("Could not find a valid BME680 sensor, check wiring!");
        return;
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
    // Initialize LED for status indication
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // Initialize Serial for debug output
    RUI_LOG("Starting RUI3 BLE UART Example");

    // Initialize sensor
    bme680_init();

    // Initialize BLE
    RUI_BLE.begin();  // Initialize BLE for RUI3

    RUI_BLE.onConnect([]() {
        g_BleUartConnected = true;
        RUI_LOG("BLE client connected");
    });

    RUI_BLE.onDisconnect([]() {
        g_BleUartConnected = false;
        RUI_LOG("BLE client disconnected");
    });

    // Start BLE advertising
    RUI_BLE.advertise("RAK4631_UART");

    // Set up analog read resolution and reference
    analogReference(AR_INTERNAL_3_0);
    analogReadResolution(12);

    // Initialize battery voltage reading
    readVBAT();
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

    // Prepare timestamp and sensor data string
    String timestamp = String(millis() / 1000);
    String sensorData = 
        "Battery Voltage: " + String(batteryVoltage_mV / 1000.0, 3) + " V\n" +
        "Battery Percentage: " + String(batteryPercent) + " %\n" +
        "Temperature: " + String(bme.temperature) + " °C\n" +
        "Pressure: " + String(bme.pressure / 100.0) + " hPa\n" +
        "Humidity: " + String(bme.humidity) + " %\n" +
        "Gas Resistance: " + String(bme.gas_resistance / 1000.0) + " KOhms\n\n";

    // Send data over BLE UART if connected
    if (g_BleUartConnected) {
        RUI_BLE.print(sensorData);  // Send sensor data to BLE client
    }

    delay(5000);  // Delay before next sensor reading

    // Forward anything received from USB Serial to BLE UART
    if (Serial.available() && g_BleUartConnected) {
        RUI_BLE.print(Serial.readString());
    }

    // Forward anything received from BLE UART to USB Serial
    if (RUI_BLE.available()) {
        Serial.print(RUI_BLE.readString());
    }
}
