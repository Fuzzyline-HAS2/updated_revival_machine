#ifndef _REVIVAL_MACHINE_H_
#define _REVIVAL_MACHINE_H_

#include "library_and_pin.h"

//============================ Global Variable ============================
typedef enum MACHINE_STATE
{
    revival,
    board
} MACHINE_STATE;                       // 생명장치의 상태를 저장하긴 위한 열거형
MACHINE_STATE machine_state = revival; // 생명장치 사용인지, 현황판 사용인지 구분하는 변수
int tag_num;
bool login_complete;
String cur_tag_user;

typedef enum GameState
{
    setting,
    ready,
    ready_activate,
    activate
} GameState;
GameState game_state = setting;

//============================ Hardware Serial ============================
// HardwareSerial MySerial1(1);    // 사용X
HardwareSerial MySerial2(2); // Display

//================================ Wifi ==================================
HAS2_Wifi has2wifi;

void SettingFunc();
void ReadyFunc();
void ActivateFunc();
void ActivateRunOnce();
void DataChange();

//=============================== Display ===============================
String currner_page;

void DisplayCheck();
void NextionReceived(String nextion_string);
void SendCmd(String command);
void PageChange(String page);

//================================ Timer ================================
SimpleTimer login_timer;
SimpleTimer rfid_timer;
SimpleTimer wifi_timer;
SimpleTimer tag_receive_timer;

// 타이머 설정 관련 함수
int login_timer_id;
int rfid_timer_id;
int wifi_timer_id;
int tag_receive_timer_id;

int nsec_tag_num;
bool nsec_tag_bool;

void TimerInit();
void TimerRun();
void LoginTimerFunc();
void WifiTimerFunc();
void RfidTimerAssess();
void RfidTagTimerFunc();
void TagReceiveTimerFunc();

//*=============================== Sensor ===============================*
/**
 * @brief Revival Machine에 사용되는 센서, 모듈 세팅 [Neopixel, RFID]
 */
void SensorInit();

//================================ RFID ==================================
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void RfidInit(void);
void RfidLoop(void);
void CardChecking(uint8_t rfidData[32]);

int rfid_num = 1;
bool rfid_tag = false;
byte rfid_tag_count = 0;
int nsec_tag;

bool rfid_not_work = false;
bool send_nfc_err = false;

//=============================== Neopixel ===============================
#define NUMPIXELS_TOP 24
#define NUMPIXELS_MID 16
#define NUMPIXELS_BOT 8

Adafruit_NeoPixel pixels_top(NUMPIXELS_TOP, NEOPIXEL_TOP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_mid(NUMPIXELS_MID, NEOPIXEL_MID_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_bot(NUMPIXELS_BOT, NEOPIXEL_BOT_PIN, NEO_GRB + NEO_KHZ800);

// Neopixel 색상정보
int color_brightness = 20;

int white[3] = {color_brightness, color_brightness, color_brightness};
int red[3] = {color_brightness, 0, 0};
int yellow[3] = {color_brightness, color_brightness, 0};
int green[3] = {0, color_brightness, 0};
int purple[3] = {color_brightness, 0, color_brightness};
int blue[3] = {0, 0, color_brightness};

void NeopixelFail();

#endif
