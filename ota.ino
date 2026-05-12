/*
 * OTA 모듈 - SecureOTA 라이브러리를 이용한 HMAC-SHA256 보안 OTA 업데이트
 *
 * [사용 방법]
 * - device_state = "github" 수신 시 checkOTA() 호출 (game_state.ino)
 */

#include <SecureOTA.h>
#include "secrets.h"  // HMAC_SECRET

#define FIRMWARE_VER 16

SecureOTA ota(
  "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.bin",
  "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/version.txt",
  "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.sig",
  HMAC_SECRET,
  FIRMWARE_VER
);

void OtaInit() {
  ota.setLogStream(DebugSerial);
  ota.setOnSuccess([]() {
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "setting");
  });
  ota.setOnSkip([]() {
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "setting");
  });
}

// device_state == "github" 수신 시 호출 (game_state.ino)
void checkOTA() {
  ota.check();
}
