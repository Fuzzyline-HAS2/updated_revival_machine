#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

// ==========================================
// OTA 사용자 설정 (OTA CONFIGURATION)
// ==========================================

// 1. 와이파이 설정 (revival machine 네트워크와 동일하게 유지)
const char *ssid = "badland_ruins";
const char *password = "Code3824@";

// 2. 펌웨어 다운로드 주소
// 펌웨어 파일(.bin)이 있는 GitHub raw 주소를 입력해주세요.
// 예시: "https://raw.githubusercontent.com/<유저명>/<레포명>/main/update.bin"
const char *firmware_url = "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/update.bin";

// 버전 정보 파일 URL (version.txt)
const char *version_url = "https://raw.githubusercontent.com/Fuzzyline-HAS2/updated_revival_machine/main/version.txt";

// 3. 현재 펌웨어 버전 (서버의 version.txt 숫자와 비교됨)
#define CURRENT_FIRMWARE_VERSION 1

// ==========================================
#endif
