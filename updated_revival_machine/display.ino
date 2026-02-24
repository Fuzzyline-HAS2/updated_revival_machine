#include "revival_machine.h"

/**
 * @brief 디스플레이에 변화를 주거나 변화가 있을시 실행
 */
void DisplayCheck()
{
    while (Serial2.available() > 0)
    {
        String nextion_string = Serial2.readStringUntil(' ');
        Serial.print("nextion_string :");
        Serial.println(nextion_string);
        NextionReceived(nextion_string);
    }
}
/**
 * @brief 디스플레이에서 오는 Serial을 확인
 *
 * @param nextion_string Serial 문자열 데이터
 */
void NextionReceived(String nextion_string)
{
    Serial.println(nextion_string);
    if (nextion_string == "login")
    {
        machine_state = board;
        Serial.println((String)(const char *)my["device_state"]);
        if (!login_timer.isEnabled(login_timer_id))
        {
            login_timer_id = login_timer.setTimeout(7000, LoginTimerFunc);
            if ((int)my["life_chip"] > 0)
            {
                if ((String)(const char *)my["device_state"] == "activate")
                {
                    SendCmd("page chip_login");
                }
                else if ((String)(const char *)my["device_state"] == "ready_activate")
                {
                    SendCmd("page disable_login");
                }
            }
            else
            {
                SendCmd("page no_chip_login");
            }
        }
        else
        {
            login_timer.restartTimer(login_timer_id);
        }
    }
    if (nextion_string == "iogin_on")
    {
        delay(3000);
    }
}

/**
 * @brief Nextion Display 구문(코드)를 입력하고 싶을때 사용
 */
void SendCmd(String command)
{
    String cmd = "";
    if (command.startsWith("page") && (String)(const char *)shift_machine["selected_language"] == "EN")
    {
        cmd = "page E" + command.substring(5);
    }
    else
    {
        cmd = command;
    }
    sendCommand(cmd.c_str());
}

/**
 * @brief 동일한 페이지를 디스플레이에 띄우지 않기 하기 위한 함수
 *
 * @param page 띄우고 싶은 페이지
 */
void PageChange(String page)
{
    if (currner_page != page)
    {
        SendCmd("page " + page);
        currner_page = page;
    }
}
