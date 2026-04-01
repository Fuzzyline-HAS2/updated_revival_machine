#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

// ==========================================
// OTA 사용자 설정 (OTA CONFIGURATION)
// ==========================================

// 1. 와이파이 설정 (revival machine 네트워크와 동일하게 유지)
// HAS2_Wifi.h가 ssid/password를 전역 선언하므로 충돌 방지를 위해 ota_ 접두사 사용
const char *ota_ssid = "badland_ruins";
const char *ota_password = "Code3824@";

// 2. 펌웨어 다운로드 주소
// 펌웨어 파일(.bin)이 있는 GitHub raw 주소를 입력해주세요.
const char *firmware_url = "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.bin";

// 버전 정보 파일 URL (version.txt)
const char *version_url = "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/version.txt";

// 서명 파일 URL (update.sig) - 32바이트 HMAC-SHA256 서명
const char *signature_url = "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.sig";

// 3. HMAC 서명 비밀키 (secrets.h에서 관리 — GitHub에 올라가지 않음)
#include "secrets.h"

// 4. 현재 펌웨어 버전 (서버의 version.txt 숫자와 비교됨)
#define CURRENT_FIRMWARE_VERSION 7

// ==========================================
#endif
