/*
 * OTA 모듈 - 클라우드 펌웨어 업데이트
 *
 * GitHub 등 URL에서 펌웨어(.bin)를 다운로드하고 자동 업데이트합니다.
 * 설정은 OTA_Config.h 파일에서 변경하세요.
 *
 * [사용 방법]
 * - setup() 또는 RevivalMachineInit() 초반에 initOTA() 호출
 * - 필요 시 언제든지 checkOTA() 호출로 수동 업데이트 트리거 가능
 */

#include "public_key.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "OTA_Config.h"

#define USE_SHA256
#define USE_RSA

// =======================================================

// 서버의 버전 정보를 확인하는 함수
int checkServerVersion() {
  Serial.println("[OTA] 서버 버전 확인 중...");

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(30000);

  http.begin(client, String(version_url));
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setTimeout(30000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String versionStr = http.getString();
    versionStr.trim();
    int serverVersion = versionStr.toInt();

    Serial.printf("[OTA] 서버 버전: %d, 현재 버전: %d\n", serverVersion,
                  CURRENT_FIRMWARE_VERSION);

    http.end();
    client.stop();
    delay(500);
    return serverVersion;
  } else {
    Serial.printf("[OTA] 버전 확인 실패 (HTTP 코드: %d)\n", httpCode);
    http.end();
    client.stop();
    delay(500);
    return -1;
  }
}

// URL에서 펌웨어를 다운로드하고 OTA 업데이트를 실행하는 함수
void execOTA() {
  // URL 유효성 검사
  if (String(firmware_url).indexOf("http") < 0 ||
      String(firmware_url).indexOf("REPLACE") >= 0) {
    Serial.println("[OTA] 오류: OTA_Config.h에서 firmware_url을 설정해주세요!");
    return;
  }

  Serial.println("[OTA] 클라우드 OTA 업데이트를 시작합니다...");
  Serial.println("[OTA] 대상 URL: " + String(firmware_url));

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(30000);

  http.begin(client, String(firmware_url));
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.setTimeout(30000);

  int httpCode = http.GET();

  // [안전장치 1] HTTP 200 OK가 아니면 즉시 중단
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[OTA] 펌웨어 다운로드 실패 (HTTP 코드: %d)\n", httpCode);
    if (httpCode > 0) {
      Serial.printf("[OTA] 에러: %s\n", http.errorToString(httpCode).c_str());
    } else {
      Serial.println("[OTA] 연결 실패. 네트워크를 확인하세요.");
    }
    http.end();
    client.stop();
    return;
  }

  // [안전장치 2] Content-Length 검증
  int contentLength = http.getSize();
  Serial.printf("[OTA] 다운로드 크기: %d bytes\n", contentLength);

  if (contentLength <= 0 || contentLength > 2000000) {
    Serial.println("[OTA] 오류: 잘못된 파일 크기");
    http.end();
    client.stop();
    return;
  }

  // [안전장치 3] Update 시작 가능 여부 확인
  if (!Update.begin(contentLength)) {
    Serial.println("[OTA] OTA를 시작할 공간이 부족합니다.");
    http.end();
    client.stop();
    return;
  }

  Serial.println("[OTA] 업데이트 중... 잠시만 기다려주세요.");

  size_t written = Update.writeStream(http.getStream());

  // [안전장치 4] 완전히 다운로드되었는지 확인
  if (written != (size_t)contentLength) {
    Serial.printf("[OTA] 다운로드 불완전: %d / %d bytes\n", written,
                  contentLength);
    Update.abort();
    http.end();
    client.stop();
    return;
  }

  Serial.printf("[OTA] %d bytes 다운로드 완료\n", written);

  // [안전장치 5] Update 종료 및 검증
  if (!Update.end(true)) {
    Serial.printf("[OTA] 업데이트 실패: %d\n", Update.getError());
    Update.abort();
    http.end();
    client.stop();
    return;
  }

  // [안전장치 6] 최종 확인
  if (!Update.isFinished()) {
    Serial.println("[OTA] 업데이트가 완전히 종료되지 않았습니다.");
    http.end();
    client.stop();
    return;
  }

  Serial.println("[OTA] OTA 완료! 3초 후 재부팅합니다...");
  http.end();
  client.stop();
  delay(3000);

  ESP.restart(); // 모든 검증을 통과한 경우에만 재부팅
}

void initOTA() {
  Serial.println("\n[OTA] 초기화 시작...");
  Serial.print("[OTA] 와이파이 연결 중: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OTA] 와이파이 연결 성공!");
    Serial.print("[OTA] IP: ");
    Serial.println(WiFi.localIP());

    int serverVersion = checkServerVersion();

    if (serverVersion == -1) {
      Serial.println("[OTA] 버전 확인 실패. OTA 스킵");
    } else if (serverVersion != CURRENT_FIRMWARE_VERSION) {
      Serial.printf("[OTA] 버전 불일치! (현재: v%d → 서버: v%d)\n",
                    CURRENT_FIRMWARE_VERSION, serverVersion);
      Serial.println("[OTA] 5초 후 펌웨어 다운로드를 시작합니다...");
      delay(5000);
      checkOTA();
    } else {
      Serial.printf("[OTA] 최신 버전 (v%d). OTA 스킵\n",
                    CURRENT_FIRMWARE_VERSION);
    }
  } else {
    Serial.println("\n[OTA] 와이파이 연결 실패! 3초 후 재부팅합니다...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("[OTA] 초기화 완료\n");
}

// OTA 업데이트를 확인하고 실행하는 함수 (언제든지 호출 가능)
void checkOTA() { execOTA(); }
