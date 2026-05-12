/*
 * OTA 모듈 - SecureOTA 라이브러리를 이용한 HMAC-SHA256 보안 OTA 업데이트
 *
 * [사용 방법]
 * - device_state = "github" 수신 시 checkOTA() 호출 (game_state.ino)
 */

#include <SecureOTA.h>
#include "secrets.h"  // HMAC_SECRET

#define FIRMWARE_VER 17

SecureOTA ota(
  "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.bin",
  "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/version.txt",
  "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.sig",
  HMAC_SECRET,
  FIRMWARE_VER
);

void OtaInit() {
  ota.setLogStream(DebugSerial);
  // 업데이트 성공 시: 재부팅 직전에 콜백으로 setting 전송 (check()가 return 안 되므로)
  ota.setOnSuccess([]() {
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "setting");
  });
}

// device_state == "github" 수신 시 호출 (game_state.ino)
void checkOTA() {
  bool skipped = false;
  ota.setOnSkip([&skipped]() { skipped = true; });
  ota.check();
  // check()가 return됐다 = 업데이트 없음 (skip 또는 에러)
  // skip일 때만 setting으로 변경 (에러 시엔 재시도 가능하게 냅둠)
  if (skipped) {
    TLOGLN("[OTA] 최신 버전 — device_state setting으로 변경");
    has2wifi.Send((String)(const char *)my["device_name"], "device_state", "setting");
  }
}
