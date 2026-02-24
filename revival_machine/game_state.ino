#include "revival_machine.h"

/**
 * @brief DB gamestate가 setting 일 때 동작하는 코드
 */
void SettingFunc()
{
    game_state = setting;
    pixels_top.lightColor(white);
    pixels_mid.lightColor(white);
    pixels_bot.lightColor(white);
    SendCmd("page before_tagger");
}

/**
 * @brief DB gamestate가 ready 일 때 동작하는 코드
 */
void ReadyFunc()
{
    game_state = ready;
    pixels_top.lightColor(red);
    pixels_mid.lightColor(red);
    pixels_bot.lightColor(red);
    if ((String)(const char *)my["device_state"] == "ready_activate")
    {
        SendCmd("page disable");
    }
    else if ((String)(const char *)my["device_state"] == "ready")
    {
        SendCmd("page before_tagger");
    }
}

/**
 * @brief DB gamestate가 activate 일 때 동작하는 코드
 */
void ActivateFunc()
{
    // 디스플레이 변화 체크
    DisplayCheck();
    // Rfid 태그 체크
    RfidLoop();
}

void ActivateRunOnce()
{
    game_state = activate;

    SendCmd("sleep=0");
    if ((int)my["life_chip"] > 0)
    {
        SendCmd("page login");
        pixels_top.lightColor(yellow);
        pixels_mid.lightColor(yellow);
        pixels_bot.lightColor(yellow);
    }
    else
    {
        SendCmd("page no_life_chip");
        pixels_top.lightColor(blue);
        pixels_mid.lightColor(blue);
        pixels_bot.lightColor(blue);
    }
}

void DataChange()
{
    static StaticJsonDocument<1000> cur;

    String cmd;

    if ((String)(const char *)my["game_state"] != (String)(const char *)cur["game_state"])
    {
        if ((String)(const char *)my["game_state"] == "setting")
        {
            SettingFunc();
        }
        else if ((String)(const char *)my["game_state"] == "ready")
        {
            ReadyFunc();
        }
        else if ((String)(const char *)my["game_state"] == "activate")
        {
            ActivateRunOnce();
        }
    }

    if ((String)(const char *)my["device_state"] != (String)(const char *)cur["device_state"])
    {
        if ((String)(const char *)my["device_state"] == "ready_activate")
        {
            SendCmd("page disable");
        }
        else if ((String)(const char *)my["device_state"] == "ready")
        {
            SendCmd("page before_tagger");
        }
        else if ((String)(const char *)my["device_state"] == "player_win")
        {
            game_state = setting;
            SendCmd("page win");
        }
        else if ((String)(const char *)my["device_state"] == "player_lose")
        {
            game_state = setting;
            SendCmd("page lose");
        }
    }

    if ((String)(const char *)my["manage_state"] != (String)(const char *)cur["manage_state"])
    {
        if ((String)(const char *)my["manage_state"] == "mu")
        {
            // TODO 원하는 대상에게 줘야하나?
            has2wifi.Send((String)(const char *)my["device_name"], "device_state", "used");
            has2wifi.Send((String)(const char *)my["device_name"], "life_chip", "-1");
            // has2wifi.Send((String)(const char*)tag["device_name"], "life_chip", "+1");
            // has2wifi.Send((String)(const char*)tag["device_name"], "exp", "+90");

            Serial.println("page no_life_chip");
            SendCmd("page no_life_chip");
            pixels_bot.lightColor(blue);
            pixels_mid.lightColor(blue);
            pixels_top.lightColor(blue);
        }
    }

    Serial.println("Data Change");
    cur = my;
}