#include "revival_machine.h"

/**
 * @brief millis 기반 타이머 세팅
 */
void TimerInit()
{
  login_timer.disable(login_timer_id);
  wifi_timer_id = wifi_timer.setInterval(500, WifiTimerFunc);
}

void TimerRun()
{
  login_timer.run();
  wifi_timer.run();
  rfid_timer.run();
  tag_receive_timer.run();
}

void LoginTimerFunc()
{
  machine_state = revival;
  if ((int)my["life_chip"] > 0)
  {
    if (game_state == activate)
    {
      SendCmd("page chip");
    }
    else if (game_state == ready)
    {
      SendCmd("page disable");
    }
  }
  else
  {
    SendCmd("page no_life_chip");
  }
}

void WifiTimerFunc()
{
  has2wifi.Loop(DataChange);
}

/**
 * @brief RFID가 연속적으로 찍히지 않게 하기위해 플래그
 */
void RfidTagTimerFunc()
{
  rfid_tag = false;
}

void TagReceiveTimerFunc()
{
  cur_tag_user = " ";
}