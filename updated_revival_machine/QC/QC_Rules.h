#ifndef QC_RULES_H
#define QC_RULES_H

#include "../library_and_pin.h"
#include "QC_Types.h"
#include <HAS2_Wifi.h>
#include <WiFi.h>
#include <cstdint>
#include <esp_system.h>

// 'my' is declared in HAS2_Wifi.h (included above)

// ---------------------------------------------------------
// Rule: Free Heap Memory
// ---------------------------------------------------------
class QCRule_HeapMemory : public IQCRule {
public:
  String getId() const override { return "SYS_MEM_01"; }
  String getName() const override { return "Free Heap Check"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    uint32_t freeHeap = ESP.getFreeHeap();
    String val = String(freeHeap) + " bytes";
    const uint32_t WARN_THRESHOLD = 20000;
    const uint32_t FAIL_THRESHOLD = 10000;
    if (freeHeap < FAIL_THRESHOLD) {
      return QCResult(QCLevel::FAIL, getId(), "System Free Heap",
                      "< " + String(FAIL_THRESHOLD), val,
                      "Check for memory leaks or reduce static allocation");
    } else if (freeHeap < WARN_THRESHOLD) {
      return QCResult(QCLevel::WARN, getId(), "System Free Heap",
                      "< " + String(WARN_THRESHOLD), val,
                      "Monitor memory usage closely");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: WiFi Signal Strength
// ---------------------------------------------------------
class QCRule_WifiSignal : public IQCRule {
public:
  String getId() const override { return "NET_WIFI_01"; }
  String getName() const override { return "WiFi Signal Strength"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    if (WiFi.status() != WL_CONNECTED) {
      return QCResult();
    }
    int32_t rssi = WiFi.RSSI();
    String val = String(rssi) + " dBm";
    const int32_t WARN_THRESHOLD = -75;
    const int32_t FAIL_THRESHOLD = -85;
    if (rssi < FAIL_THRESHOLD) {
      return QCResult(QCLevel::FAIL, getId(), "WiFi RSSI",
                      "< " + String(FAIL_THRESHOLD), val,
                      "Move device closer to AP or check antenna");
    } else if (rssi < WARN_THRESHOLD) {
      return QCResult(QCLevel::WARN, getId(), "WiFi RSSI",
                      "< " + String(WARN_THRESHOLD), val,
                      "Signal weak, potential packet loss");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: WiFi Connection Status
// ---------------------------------------------------------
class QCRule_WifiConnection : public IQCRule {
public:
  String getId() const override { return "NET_WIFI_00"; }
  String getName() const override { return "WiFi Connection Status"; }
  bool isFastCheck() const override { return true; }

  QCResult check() override {
    if (WiFi.status() != WL_CONNECTED) {
      return QCResult(QCLevel::FAIL, getId(), "WiFi Status", "Connected",
                      "Disconnected",
                      "Check Router/Credentials or Reset Device");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Last Reset Reason (Brownout / WDT / Panic)
// ---------------------------------------------------------
class QCRule_ResetReason : public IQCRule {
public:
  String getId() const override { return "SYS_RST_01"; }
  String getName() const override { return "Reset Reason Check"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    static bool reported = false;
    if (reported)
      return QCResult();

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_BROWNOUT) {
      reported = true;
      return QCResult(QCLevel::FAIL, getId(), "Last Reset Reason",
                      "No brownout", "ESP_RST_BROWNOUT",
                      "Check 5V/3.3V rail, motor surge noise, shared GND");
    }
    bool isWdtReset = (reason == ESP_RST_WDT || reason == ESP_RST_INT_WDT);
#ifdef ESP_RST_TASK_WDT
    isWdtReset = isWdtReset || (reason == ESP_RST_TASK_WDT);
#endif
    if (isWdtReset) {
      reported = true;
      return QCResult(QCLevel::WARN, getId(), "Last Reset Reason",
                      "No watchdog reset", String((int)reason),
                      "Check blocking code and peripheral hangs");
    }
    if (reason == ESP_RST_PANIC) {
      reported = true;
      return QCResult(QCLevel::WARN, getId(), "Last Reset Reason",
                      "No panic reset", "ESP_RST_PANIC",
                      "Check null pointer / memory corruption");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Game State Consistency
// ---------------------------------------------------------
class QCRule_GameState : public IQCRule {
public:
  String getId() const override { return "LOGIC_STATE_01"; }
  String getName() const override { return "Game State Consistency"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    // Skip check if 'my' has no data yet (device not registered or backend not
    // responding)
    if (my.size() == 0) {
      return QCResult(); // PASS - no data to validate yet
    }

    if (my.containsKey("game_state")) {
      String gState = (const char *)my["game_state"];
      bool gValid = (gState == "setting" || gState == "activate" ||
                     gState == "ready" || gState == "");
      if (!gValid) {
        return QCResult(QCLevel::WARN, getId(), "game_state Value",
                        "Known State", gState,
                        "Check Logic for invalid state assignment");
      }
    }

    if (my.containsKey("device_state")) {
      String dState = (const char *)my["device_state"];
      bool dValid =
          (dState == "setting" || dState == "activate" || dState == "ready" ||
           dState == "ready_activate" || dState == "used" ||
           dState == "player_win" || dState == "player_lose" || dState == "");
      if (!dValid) {
        return QCResult(QCLevel::WARN, getId(), "device_state Value",
                        "Known State", dState,
                        "Check Logic for invalid state assignment");
      }
    }

    if (my.containsKey("manage_state")) {
      String mState = (const char *)my["manage_state"];
      bool mValid = (mState == "mu" || mState == "");
      if (!mValid) {
        return QCResult(QCLevel::WARN, getId(), "manage_state Value",
                        "Known State", mState,
                        "Check Logic for invalid state assignment");
      }
    }

    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Dual RFID Initialization Status
// ---------------------------------------------------------
// NOTE: revival_machine has a single PN532. RfidInit() retries forever (goto)
//       so init always succeeds before loop() starts.
//       This rule reports the first runtime PN532 failure detected in
//       RfidLoop().
class QCRule_RfidStatus : public IQCRule {
public:
  String getId() const override { return "HW_RFID_01"; }
  String getName() const override { return "PN532 Runtime Status"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    extern bool rfid_not_work;
    static bool reported = false;

    // Once reported, suppress repeated logs (processResult also throttles,
    // but this prevents the FAIL from firing again after the cable is reseated)
    if (reported)
      return QCResult();

    if (rfid_not_work) {
      reported = true;
      return QCResult(QCLevel::FAIL, getId(), "PN532 Status", "ACK from PN532",
                      "No response",
                      "Check SPI wiring (SCK/MISO/MOSI/SS) and PN532 power");
    }
    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: Static Pin Conflict Check
// ---------------------------------------------------------
// Checks pins actually defined in library_and_pin.h for this project.
// (Neopixel x3, PN532 SPI x4, Serial2 x2)
class QCRule_PinConflict : public IQCRule {
public:
  String getId() const override { return "HW_PIN_01"; }
  String getName() const override { return "Pin Conflict Check"; }
  bool isFastCheck() const override { return true; }

  QCResult check() override {
    // 1. Neopixel 핀 3개가 서로 겹치지 않는지 확인
    if (NEOPIXEL_TOP_PIN == NEOPIXEL_MID_PIN ||
        NEOPIXEL_TOP_PIN == NEOPIXEL_BOT_PIN ||
        NEOPIXEL_MID_PIN == NEOPIXEL_BOT_PIN) {
      String val = "TOP:" + String(NEOPIXEL_TOP_PIN) +
                   " MID:" + String(NEOPIXEL_MID_PIN) +
                   " BOT:" + String(NEOPIXEL_BOT_PIN);
      return QCResult(QCLevel::FAIL, getId(), "Neopixel Pin Conflict",
                      "Unique pin per strip", val,
                      "Assign unique GPIO to each Neopixel strip");
    }

    // 2. PN532 SS 핀이 Neopixel 핀들과 겹치지 않는지 확인
    if (PN532_SS == NEOPIXEL_TOP_PIN || PN532_SS == NEOPIXEL_MID_PIN ||
        PN532_SS == NEOPIXEL_BOT_PIN) {
      return QCResult(QCLevel::FAIL, getId(), "PN532 SS / Neopixel Conflict",
                      "Unique GPIO per peripheral",
                      "PN532_SS==" + String(PN532_SS),
                      "Move PN532_SS to a pin not used by Neopixel");
    }

    // 3. Serial2(Nextion) TX 핀이 PN532 SPI 핀들과 겹치지 않는지 확인
    if (SERIAL2_TX_PIN == PN532_SCK || SERIAL2_TX_PIN == PN532_MISO ||
        SERIAL2_TX_PIN == PN532_MOSI || SERIAL2_TX_PIN == PN532_SS) {
      return QCResult(QCLevel::FAIL, getId(), "Serial2 TX / PN532 SPI Conflict",
                      "Unique GPIO per peripheral",
                      "SERIAL2_TX==" + String(SERIAL2_TX_PIN),
                      "Move Serial2 TX to a pin not used by PN532 SPI");
    }

    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: GPIO Capability Mismatch Check
// ---------------------------------------------------------
// GPIO34~39 on ESP32 are input-only (no internal pull-up/down).
// This project uses SERIAL2_RX_PIN = 39 (input-only, used for Nextion RX).
// Check that none of the OUTPUT pins were accidentally assigned to 34~39.
class QCRule_GpioCapability : public IQCRule {
public:
  String getId() const override { return "HW_GPIO_01"; }
  String getName() const override { return "GPIO Capability Check"; }
  bool isFastCheck() const override { return true; }

  QCResult check() override {
    // Neopixel pins must NOT be input-only GPIOs (they need output drive)
    auto isInputOnly = [](int pin) { return pin >= 34 && pin <= 39; };

    if (isInputOnly(NEOPIXEL_TOP_PIN) || isInputOnly(NEOPIXEL_MID_PIN) ||
        isInputOnly(NEOPIXEL_BOT_PIN)) {
      String val = "TOP:" + String(NEOPIXEL_TOP_PIN) +
                   " MID:" + String(NEOPIXEL_MID_PIN) +
                   " BOT:" + String(NEOPIXEL_BOT_PIN);
      return QCResult(QCLevel::FAIL, getId(), "Neopixel on Input-Only GPIO",
                      "Output-capable GPIO (0~33)", val,
                      "Move Neopixel pin away from GPIO34~39");
    }

    // PN532 SS/SCK/MOSI must NOT be input-only GPIOs
    if (isInputOnly(PN532_SS) || isInputOnly(PN532_SCK) ||
        isInputOnly(PN532_MOSI)) {
      return QCResult(QCLevel::FAIL, getId(), "PN532 SPI on Input-Only GPIO",
                      "Output-capable GPIO (0~33)",
                      "SCK:" + String(PN532_SCK) + " MOSI:" +
                          String(PN532_MOSI) + " SS:" + String(PN532_SS),
                      "Move PN532 output pins away from GPIO34~39");
    }

    return QCResult();
  }
};

// ---------------------------------------------------------
// Rule: FSM vs Server State Consistency
// ---------------------------------------------------------
// Checks that the local GameState FSM matches what the server's
// device_state implies. Based on transitions in game_state.ino:
//   device_state "activate"                   → game_state == activate
//   device_state "ready" / "ready_activate"   → game_state == ready
//   device_state "setting"/"player_win"/etc.  → game_state == setting
class QCRule_StateConsistency : public IQCRule {
public:
  String getId() const override { return "LOGIC_FSM_01"; }
  String getName() const override { return "FSM State Consistency"; }
  bool isFastCheck() const override { return false; }

  QCResult check() override {
    extern GameState game_state;
    if (my.size() == 0)
      return QCResult();

    if (!my.containsKey("device_state"))
      return QCResult();

    String dState = (const char *)my["device_state"];
    bool mismatch = false;

    // Server says activate → local FSM must be activate
    if (dState == "activate" && game_state != activate) {
      mismatch = true;
    }
    // Server says ready or ready_activate → local FSM must be ready
    else if ((dState == "ready" || dState == "ready_activate") &&
             game_state != ready) {
      mismatch = true;
    }
    // Server says setting → local FSM must be setting
    else if (dState == "setting" && game_state != setting) {
      mismatch = true;
    }

    if (mismatch) {
      // Convert game_state enum to readable string for the log
      const char *fsm = (game_state == activate)         ? "activate"
                        : (game_state == ready)          ? "ready"
                        : (game_state == ready_activate) ? "ready_activate"
                                                         : "setting";
      String detail = "Server:" + dState + " FSM:" + String(fsm);
      return QCResult(QCLevel::WARN, getId(), "FSM vs Server", "Consistent",
                      detail, "State mismatch between server and device");
    }

    return QCResult();
  }
};

#endif // QC_RULES_H
