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

#include "updated_revival_machine.h"

//************************************************ Core1
//********************************************************************
/**
 * @brief Revival Machine Intialize
 */
void RevivalMachineInit() {
  nexInit(); // 디스플레이 세팅
  MySerial2.begin(9600, SERIAL_8N1, SERIAL2_RX_PIN,
                  SERIAL2_TX_PIN);              // 디스플레이 세팅
  has2wifi.Setup("badland_ruins", "Code3824@"); // 와이파이 세팅
  SensorInit(); // IoT Glove 사용 센서1, 모듈 세팅
  TimerInit();  // 타이머 세팅
  DataChange();

  // -------- QC Engine --------
  QCEngine &qc = QCEngine::getInstance();
  qc.begin(2000);                            // slow 규칙 체크 주기: 2초
  qc.addRule(new QCRule_HeapMemory());       // SYS_MEM_01  (slow)
  qc.addRule(new QCRule_ResetReason());      // SYS_RST_01  (slow)
  qc.addRule(new QCRule_WifiConnection());   // NET_WIFI_00 (fast)
  qc.addRule(new QCRule_WifiSignal());       // NET_WIFI_01 (slow)
  qc.addRule(new QCRule_RfidStatus());       // HW_RFID_01  (slow)
  qc.addRule(new QCRule_PinConflict());      // HW_PIN_01   (fast)
  qc.addRule(new QCRule_GpioCapability());   // HW_GPIO_01  (fast)
  qc.addRule(new QCRule_GameState());        // LOGIC_STATE_01 (slow)
  qc.addRule(new QCRule_StateConsistency()); // LOGIC_FSM_01  (slow)
  // ---------------------------
}

/**
 * @brief 아두이노 기본 문법 (전원이 켜지면 한번만 실행)
 */
void setup() {
  delay(1000);
  Serial.begin(115200);
  RevivalMachineInit();
}

/**
 * @brief 아두이노 기본 문법 (전원이 켜져있는동안 Core1에서 계속 실행)
 */
void loop() {
  TimerRun();
  QCEngine::getInstance()
      .tick(); // QC 규칙 실행 (fast: 매 루프 / slow: 2초마다)
  if (game_state != setting) {
    // 디스플레이 변화 체크
    DisplayCheck();
    // Rfid 태그 체크
    RfidLoop();
  }
}
