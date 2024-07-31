#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WiFi.h>

#include <EFLogging.h>
#include <EFLed.h>
#include <EFPrideFlags.h>
#include <EFTouch.h>

#include "constants.h"
#include "secrets.h"

#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI

RTC_DATA_ATTR int bootCount = 0;


EFLed leds;
EFTouch touch;

uint8_t ledFlipper = 0;

volatile uint8_t boopColorIdx = 0;


void setupCpu() {
    LOG_INFO(("Initial CPU frequency: " + String(getCpuFrequencyMhz())).c_str());
    setCpuFrequencyMhz(80);
    LOG_INFO(("Set CPU frequency to: " + String(getCpuFrequencyMhz())).c_str());
}

void connectToWifi(const char* ssid, const char* password) {
    LOG_INFO(("Connecting to WiFi network: " + String(ssid)).c_str());
    leds.setDragonNose(CRGB::Red);

    WiFi.begin(ssid, password);
    WiFi.setSleep(true);
    while (WiFi.status() != WL_CONNECTED) {
        LOG_INFO(".");
        delay(200);
    }
    LOG_INFO("Connected!");

    LOG_INFO(("IP address: " + WiFi.localIP().toString()).c_str());
    LOG_INFO(("MAC address: " + WiFi.macAddress()).c_str());

    leds.setDragonNose(CRGB::Green);
}

void setupOTA(const char* password) {
    leds.setDragonMuzzle(CRGB::Yellow);
    LOG_INFO("Initializing OTA ... ");

    if (password) {
        ArduinoOTA.setPassword(password);
    }

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type = "flash";
            } else {  // U_SPIFFS
                type = "filesystem";
            }

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            LOG_INFO("Start updating " + type);
            leds.setDragonMuzzle(CRGB::Blue);
        })
        .onEnd([]() {
            LOG_INFO("\nEnd");
            for (uint8_t i = 0; i < 3; i++) {
                leds.setDragonMuzzle(CRGB::Green);
                delay(500);
                leds.setDragonMuzzle(CRGB::Black);
                delay(500);
            }
            leds.clear();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            LOGF_INFO("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            LOGF_ERROR("Error[%u]: ", error);
            leds.setDragonMuzzle(CRGB::Red);
            if (error == OTA_AUTH_ERROR) {
                LOG_WARNING("Auth Failed");
                leds.setDragonMuzzle(CRGB::Purple);
            } else if (error == OTA_BEGIN_ERROR) {
                LOG_ERROR("Begin Failed");
            } else if (error == OTA_CONNECT_ERROR) {
                LOG_ERROR("Connect Failed");
            } else if (error == OTA_RECEIVE_ERROR) {
                LOG_ERROR("Receive Failed");
            } else if (error == OTA_END_ERROR) {
                LOG_ERROR("End Failed");
            }
        });

    ArduinoOTA.begin();

    LOG_INFO("Done.");
    leds.setDragonMuzzle(CRGB::Green);
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : LOG_DEBUG("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : LOG_DEBUG("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : LOG_DEBUG("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : LOG_DEBUG("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : LOG_DEBUG("Wakeup caused by ULP program"); break;
    default: LOGF_DEBUG("Wakeup was not caused by deep sleep: %d\r\n", wakeup_reason); break;
  }
}

void isr_noseboop() {
    boopColorIdx = (boopColorIdx + 1) % 8;
}

void setup() {
    delay(3000);
    USBSerial.begin(9600);
    delay(3000);
    LOG_INFO("Booting ...");
    LOGF_DEBUG("Boot count: %d\r\n", bootCount++);
    print_wakeup_reason();

    setupCpu();
    leds = EFLed(20);
    leds.clear();
    connectToWifi(WIFI_SSID, WIFI_PASSWORD);
    setupOTA(OTA_SECRET);

    // Touchy stuffy
    touch = EFTouch();
    touch.calibrate();
    touch.attachInterruptNose(isr_noseboop);
}

uint8_t deepsleepcounter = 10;
uint8_t flagidx = 0;
uint32_t brightness = 0;

void loop() {
    // Check for OTA
    ArduinoOTA.handle();

    // Blink a LED
    if (ledFlipper == 0) {
        LOG_DEBUG(".");
        deepsleepcounter--;

        switch (flagidx) {
            case 0: leds.setEFBar(EFPrideFlags::LGBT); break;
            case 1: leds.setEFBar(EFPrideFlags::LGBTQI); break;
            case 2: leds.setEFBar(EFPrideFlags::Bisexual); break;
            case 3: leds.setEFBar(EFPrideFlags::Polyamorous); break;
            case 4: leds.setEFBar(EFPrideFlags::Polysexual); break;
            case 5: leds.setEFBar(EFPrideFlags::Transgender); break;
            case 6: leds.setEFBar(EFPrideFlags::Pansexual); break;
            case 7: leds.setEFBar(EFPrideFlags::Asexual); break;
            case 8: leds.setEFBar(EFPrideFlags::Genderfluid); break;
            case 9: leds.setEFBar(EFPrideFlags::Genderqueer); break;
            case 10: leds.setEFBar(EFPrideFlags::Nonbinary); break;
            case 11: leds.setEFBar(EFPrideFlags::Intersex); break;
        }
        flagidx = (flagidx + 1) % 12;
    }
    leds.setDragonEye(ledFlipper++ < 127 ? CRGB::Green : CRGB::Black);

    // Touch LEDs
    uint8_t touchy = touch.readFingerprint();
    if (touchy) {
        for (uint8_t i = 0; i < EFLED_EFBAR_NUM; i++) {
            if (touchy) {
                leds.setEFBar(i , CRGB::Purple);
                touchy--;
            } else {
                leds.setEFBar(i, CRGB::Black);
            }
        }
    }

    switch (boopColorIdx) {
        case 0: leds.setDragonEarTop(CRGB::Red); break;
        case 1: leds.setDragonEarTop(CRGB::Orange); break;
        case 2: leds.setDragonEarTop(CRGB::Yellow); break;
        case 3: leds.setDragonEarTop(CRGB::Green); break;
        case 4: leds.setDragonEarTop(CRGB::Teal); break;
        case 5: leds.setDragonEarTop(CRGB::Blue); break;
        case 6: leds.setDragonEarTop(CRGB::BlueViolet); break;
        case 7: leds.setDragonEarTop(CRGB::Purple); break;
    }

    uint8_t newbrightness = (brightness % 202) < 101 ? brightness % 101 : 100 - (brightness % 101);
    // USBSerial.print("Set brightness = ");
    // USBSerial.print(newbrightness);
    leds.setBrightness(newbrightness);
    // USBSerial.print("     -> read ");
    // USBSerial.println(leds.getBrightness());
    brightness++;

    // Wait for next iteration
    delay(10);

    // if (deepsleepcounter == 0) {
    //     USBSerial.println("Putting the ESP into deep sleep for 3 seconds ...");
    //     esp_sleep_enable_timer_wakeup(3000000);
    // 
    //     USBSerial.flush();
    //     esp_deep_sleep_start();
    // }
}
