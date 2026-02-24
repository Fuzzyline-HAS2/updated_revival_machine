/**
 * @file revival_machine.ino
 * @author YuBin Kim
 * @brief HAS2_revival_machine
 * @version 0.2
 * @date 2022-11-25 ~ 2022-02-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "revival_machine.h"

//************************************************ Core1 ********************************************************************
/**
 * @brief Revival Machine Intialize
 */
void RevivalMachineInit()
{
  nexInit();                                                         // 디스플레이 세팅
  MySerial2.begin(9600, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN); // 디스플레이 세팅
  has2wifi.Setup("badland");                                            // 와이파이 세팅
  SensorInit();                                                      // IoT Glove 사용 센서1, 모듈 세팅
  TimerInit();                                                       // 타이머 세팅
  DataChange();
}

/**
 * @brief 아두이노 기본 문법 (전원이 켜지면 한번만 실행)
 */
void setup()
{
  delay(1000);
  Serial.begin(115200);
  RevivalMachineInit();
}

/**
 * @brief 아두이노 기본 문법 (전원이 켜져있는동안 Core1에서 계속 실행)
 */
void loop()
{
  TimerRun();
  if (game_state != setting)
  {
    // 디스플레이 변화 체크
    DisplayCheck();
    // Rfid 태그 체크
    RfidLoop();
  }
}
