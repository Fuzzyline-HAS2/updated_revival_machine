/**
 * @file updated_temple.ino
 * @author YuBin Kim
 * @brief
 * @version 0.1
 * @date 2022-11-24 ~ 2022-11-26
 *
 * @copyright Copyright (c) 2022
 */

#define FIRMWARE_VER 3
#define PARTITION_VER 1
#include "updated_revival_machine.h"

//************************************************ Core1 ********************************************************************
/**
 * @brief Temple Intialize
 */
void TempleInit()
{
  Serial.println("[1] WiFi connecting...");
  // has2wifi.Setup("KT_GiGA_6C64", "ed46zx1198");                     j// 와이파이 세팅
  has2wifi.Setup("badland_shoot", "Code3824@");
  HardwareDebugSerial.printf("[Heap before TelnetInit] %d bytes free\n", ESP.getFreeHeap());
  TelnetInit();
  HardwareDebugSerial.printf("[Heap after TelnetInit] %d bytes free\n", ESP.getFreeHeap());
  Serial.println("[2] WiFi done");
  ota.setLogStream(Serial);
  ota.setOnSuccess([]() {
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "setting");
  });
  ota.setOnSkip([]() {
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "setting");
  });
    ota.setPartitionUpdate(
        "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/partitions.bin",
        "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/partitions.sig",
        "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/partition_version.txt",
        PARTITION_VER
    );
  Serial.println("[3] OTA set");
  nexInit();                                                         // 디스플레이 세팅
  MySerial2.begin(9600, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN); // 디스플레이 세팅
  Serial.println("[4] Display done");
  SensorInit();                                                      // IoT Glove 사용 센서, 모듈 세팅
  Serial.println("[5] Sensor done");
  TimerInit();                                                       // 타이머 세팅
  Serial.println("[6] Timer done");
}

/**
 * @brief 아두이노 기본 문법 (전원이 켜지면 한번만 실행)
 */
void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("=== SETUP START ===");
  TempleInit();
  Serial.println("=== TEMPLEINIT DONE ===");
  DataChange();
  Serial.println("=== DATACHANGE DONE ===");
  NextionInit();
  Serial.println("=== NEXTIONINIT DONE ===");
}

/**
 * @brief 아두이노 기본 문법 (전원이 켜져있는동안 Core1에서 계속 실행)
 */
void loop()
{
  TimerRun();
  NeoFunc();
  if (activate_bool)
  {
    ActivateFunc();
  }
}
