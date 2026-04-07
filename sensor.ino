#include "updated_revival_machine.h"

//****************************************** Initialize
//******************************************
void NeopixelSet(int color[3]) {
  uint32_t c = Adafruit_NeoPixel::Color(color[0], color[1], color[2]);
  pixels_top.fill(c);
  pixels_top.show();
  pixels_mid.fill(c);
  pixels_mid.show();
  pixels_bot.fill(c);
  pixels_bot.show();
}

void SensorInit() {
  // Neopixel Init
  pixels_top.begin();
  pixels_mid.begin();
  pixels_bot.begin();

  // RFID Init
  RfidInit();
}

//********************************************* RFID
//*********************************************
/**
 * @brief RFID(=PN532) 세팅
 */
void RfidInit() {
RestartPn532:
  nfc.begin(); // nfc 함수 시작
  if (!(nfc.getFirmwareVersion())) {
    Serial.print("!!!RFID 연결실패!!!");
    has2wifi.Send((String)(const char *)my["device_name"], "device_state",
                  "PN532");
    goto RestartPn532;
  } else {
    nfc.SAMConfig(); // configure board to read RFID tags
    Serial.println("RFID 연결성공");
  }
}

/**
 * @brief RFID 태그 인식
 */
void RfidLoop() {
  if (!rfid_tag) {
    rfid_tag = true;
    rfid_timer_id = rfid_timer.setTimeout(1000, RfidTagTimerFunc);
  } else {
    return;
  }
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength; // Length of the UID (4 or 7 bytes depending on ISO14443A
                     // card type)
  uint8_t data[32];
  char user_data[5];
  byte pn532_packetbuffer11[64];
  pn532_packetbuffer11[0] = 0x00;
  if (nfc.sendCommandCheckAck(pn532_packetbuffer11,
                              1)) { // rfid 통신 가능한 상태인지 확인
    rfid_not_work = false;          // QC: PN532 정상 응답
    if (nfc.startPassiveTargetIDDetection(
            PN532_MIFARE_ISO14443A)) { // rfid에 tag 찍혔는지 확인용 //데이터
                                       // 들어오면 uid정보 가져오기
      if (nfc.ntag2xx_ReadPage(
              7, data)) // ntag 데이터에 접근해서 불러와서 data행열에 저장
        CardChecking(data);
    }
  } else {
    rfid_not_work = true; // QC: PN532 응답 없음 (연결 불량 의심)
  }
}

/**
 * @brief RFID에 태그된 NFC의 데이터에 따른 코드 동작
 *
 * @param rfidData 태그된 NFC의 데이터
 */
void CardChecking(uint8_t rfidData[32]) // 어떤 카드가 들어왔는지 확인용
{ // TODO N초태그에 맞게 동작하는 코드 추가
  String tagUser = "";
  static int tagUser_tag_num = 0;

  for (int i = 0; i < 4; i++) // GxPx 데이터만 배열에서 추출해서 string으로 저장
    tagUser += (char)rfidData[i];
  Serial.println("tag_user_data : " + tagUser);

  if (tagUser == "MMMM") {
    ESP.restart();
  }

  // 1. 태그한 플레이어의 역할과 생명칩갯수, 최대생명칩갯수 등 읽어오기
  // 1-1. 태그한 플레이어가 이전과 동일하다면 wifi receive를 하지 않음
  has2wifi.Receive(tagUser);
  if (++tagUser_tag_num > 4) {
    cur_tag_user = "";
  }

  // 현황판 모드로 사용 후 다시 메인화면으로 돌아가는 코드
  if (login_complete) {
    login_complete = false;
    if ((int)my["life_chip"] > 0) {
      if (game_state == activate) {
        SendCmd("page login");
      } else if (game_state == ready) {
        SendCmd("page disable");
      }
    } else {
      SendCmd("page no_life_chip");
    }
    delay(2000);
    return;
  }

  // 2. 술래인지, 플레이어인지 구분
  if (machine_state == revival) {
    // 3. 술래인지, 플레이어인지 구분
    if ((int)my["life_chip"] > 0 &&
        (int)tag["life_chip"] < (int)tag["max_life_chip"]) {
      if (game_state == activate &&
          (String)(const char *)my["device_state"] != "used") {
        if ((String)(const char *)tag["role"] == "player") {
          SendCmd("page lifechip_send");
          SendCmd("collect.en=1");
          delay(4000);
          Serial.println("chip_complete");
          has2wifi.Send((String)(const char *)my["device_name"], "device_state",
                        "used");
          has2wifi.Send((String)(const char *)my["device_name"], "life_chip",
                        "-1");
          has2wifi.Send((String)(const char *)tag["device_name"], "life_chip",
                        "+1");
          has2wifi.Send((String)(const char *)tag["device_name"], "exp", "+90");

          Serial.println("page no_life_chip");
          SendCmd("page no_life_chip");
          SendCmd("collect_succ.en=1");
          NeopixelSet(blue);
          delay(2000);
          return;
        }
      }
    }

    if ((String)(const char *)my["device_state"] == "used" ||
        (String)(const char *)my["device_state"] == "ready_activate") {
      Serial.println("mp3_disabled");
      SendCmd("disabled.en=1");
    }
  }

  else if (machine_state == board) {
    SendCmd("login_mov.en=1");
    delay(4000);
    if ((String)(const char *)tag["role"] == "player" ||
        (String)(const char *)tag["role"] == "ghost" ||
        (String)(const char *)tag["role"] == "revival") {
      SendCmd("page player_main");
      Serial.println("Player Login");
    } else if ((String)(const char *)tag["role"] == "tagger") {
      SendCmd("page tagger_main");
      Serial.println("Tagger Login");
    }
    machine_state = revival;
    login_timer.deleteTimer(login_timer_id);
    delay(2000);
    login_complete = true;
  }
}