from __future__ import annotations

import copy
from typing import Iterable

from .fakes import FakeDisplay, FakePixelStrip, FakeTimer, FakeWifi
from .model import BLUE, RED, WHITE, YELLOW, GameState, HarnessState, MachineState


def _as_int(value: object, default: int = 0) -> int:
    try:
        return int(value)  # type: ignore[arg-type]
    except (TypeError, ValueError):
        return default


def _as_str(value: object) -> str:
    if value is None:
        return ""
    return str(value)


class RevivalLogic:
    def __init__(
        self,
        state: HarnessState | None = None,
        wifi: FakeWifi | None = None,
        display: FakeDisplay | None = None,
        pixels_top: FakePixelStrip | None = None,
        pixels_mid: FakePixelStrip | None = None,
        pixels_bot: FakePixelStrip | None = None,
        login_timer: FakeTimer | None = None,
        rfid_timer: FakeTimer | None = None,
        wifi_timer: FakeTimer | None = None,
        tag_receive_timer: FakeTimer | None = None,
    ) -> None:
        self.state = state or HarnessState()
        self.wifi = wifi or FakeWifi()
        self.display = display or FakeDisplay()
        self.pixels_top = pixels_top or FakePixelStrip("top")
        self.pixels_mid = pixels_mid or FakePixelStrip("mid")
        self.pixels_bot = pixels_bot or FakePixelStrip("bot")
        self.login_timer = login_timer or FakeTimer()
        self.rfid_timer = rfid_timer or FakeTimer()
        self.wifi_timer = wifi_timer or FakeTimer()
        self.tag_receive_timer = tag_receive_timer or FakeTimer()
        self._cur_snapshot: dict[str, object] = {}

    def _set_pixels(self, color: tuple[int, int, int]) -> None:
        self.pixels_top.lightColor(color)
        self.pixels_mid.lightColor(color)
        self.pixels_bot.lightColor(color)

    def send_cmd(self, command: str) -> None:
        language = _as_str(self.state.shift_machine.get("selected_language"))
        if command.startswith("page") and language == "EN":
            cmd = "page E" + command[5:]
        else:
            cmd = command
        self.display.send_command(cmd)

    def page_change(self, page: str) -> None:
        if self.state.currner_page != page:
            self.send_cmd("page " + page)
            self.state.currner_page = page

    def setting_func(self) -> None:
        self.state.game_state = GameState.SETTING
        self._set_pixels(WHITE)
        self.send_cmd("page before_tagger")

    def ready_func(self) -> None:
        self.state.game_state = GameState.READY
        self._set_pixels(RED)
        device_state = _as_str(self.state.my.get("device_state"))
        if device_state == "ready_activate":
            self.send_cmd("page disable")
        elif device_state == "ready":
            self.send_cmd("page before_tagger")

    def activate_run_once(self) -> None:
        self.state.game_state = GameState.ACTIVATE
        self.send_cmd("sleep=0")
        if _as_int(self.state.my.get("life_chip")) > 0:
            self.send_cmd("page login")
            self._set_pixels(YELLOW)
        else:
            self.send_cmd("page no_life_chip")
            self._set_pixels(BLUE)

    def data_change(self) -> None:
        my = self.state.my
        cur = self._cur_snapshot

        if _as_str(my.get("game_state")) != _as_str(cur.get("game_state")):
            game_state = _as_str(my.get("game_state"))
            if game_state == "setting":
                self.setting_func()
            elif game_state == "ready":
                self.ready_func()
            elif game_state == "activate":
                self.activate_run_once()

        if _as_str(my.get("device_state")) != _as_str(cur.get("device_state")):
            device_state = _as_str(my.get("device_state"))
            if device_state == "ready_activate":
                self.send_cmd("page disable")
            elif device_state == "ready":
                self.send_cmd("page before_tagger")
            elif device_state == "player_win":
                self.state.game_state = GameState.SETTING
                self.send_cmd("page win")
            elif device_state == "player_lose":
                self.state.game_state = GameState.SETTING
                self.send_cmd("page lose")

        if _as_str(my.get("manage_state")) != _as_str(cur.get("manage_state")):
            manage_state = _as_str(my.get("manage_state"))
            if manage_state == "mu":
                device_name = _as_str(my.get("device_name"))
                self.wifi.Send(device_name, "device_state", "used")
                self.wifi.Send(device_name, "life_chip", "-1")
                self.send_cmd("page no_life_chip")
                self._set_pixels(BLUE)

        self._cur_snapshot = copy.deepcopy(my)

    def nextion_received(self, nextion_string: str) -> None:
        if nextion_string == "login":
            self.state.machine_state = MachineState.BOARD
            if not self.login_timer.isEnabled(self.state.login_timer_id):
                self.state.login_timer_id = self.login_timer.setTimeout(7000, self.login_timer_func)
                if _as_int(self.state.my.get("life_chip")) > 0:
                    device_state = _as_str(self.state.my.get("device_state"))
                    if device_state == "activate":
                        self.send_cmd("page chip_login")
                    elif device_state == "ready_activate":
                        self.send_cmd("page disable_login")
                else:
                    self.send_cmd("page no_chip_login")
            else:
                self.login_timer.restartTimer(self.state.login_timer_id)
        if nextion_string == "iogin_on":
            return

    def login_timer_func(self) -> None:
        self.state.machine_state = MachineState.REVIVAL
        if _as_int(self.state.my.get("life_chip")) > 0:
            if self.state.game_state == GameState.ACTIVATE:
                self.send_cmd("page chip")
            elif self.state.game_state == GameState.READY:
                self.send_cmd("page disable")
        else:
            self.send_cmd("page no_life_chip")

    def wifi_timer_func(self) -> None:
        self.wifi.Loop(self.data_change)

    def rfid_tag_timer_func(self) -> None:
        self.state.rfid_tag = False

    def tag_receive_timer_func(self) -> None:
        self.state.cur_tag_user = " "

    def card_checking(self, rfid_data: Iterable[int]) -> None:
        values = list(rfid_data)
        tag_user = "".join(chr(int(v)) for v in values[:4])
        if tag_user == "MMMM":
            self.state.restarted = True

        self.wifi.Receive(tag_user)
        self.state.tag_user_tag_num += 1
        if self.state.tag_user_tag_num > 4:
            self.state.cur_tag_user = ""

        if self.state.login_complete:
            self.state.login_complete = False
            if _as_int(self.state.my.get("life_chip")) > 0:
                if self.state.game_state == GameState.ACTIVATE:
                    self.send_cmd("page login")
                elif self.state.game_state == GameState.READY:
                    self.send_cmd("page disable")
            else:
                self.send_cmd("page no_life_chip")
            return

        if self.state.machine_state == MachineState.REVIVAL:
            my_life = _as_int(self.state.my.get("life_chip"))
            tag_life = _as_int(self.state.tag.get("life_chip"))
            tag_max = _as_int(self.state.tag.get("max_life_chip"))
            device_state = _as_str(self.state.my.get("device_state"))
            tag_role = _as_str(self.state.tag.get("role"))
            if my_life > 0 and tag_life < tag_max:
                if self.state.game_state == GameState.ACTIVATE and device_state != "used":
                    if tag_role == "player":
                        my_name = _as_str(self.state.my.get("device_name"))
                        tag_name = _as_str(self.state.tag.get("device_name"))
                        self.send_cmd("page lifechip_send")
                        self.send_cmd("collect.en=1")
                        self.wifi.Send(my_name, "device_state", "used")
                        self.wifi.Send(my_name, "life_chip", "-1")
                        self.wifi.Send(tag_name, "life_chip", "+1")
                        self.wifi.Send(tag_name, "exp", "+90")
                        self.send_cmd("page no_life_chip")
                        self.send_cmd("collect_succ.en=1")
                        self._set_pixels(BLUE)
                        return

            if device_state in {"used", "ready_activate"}:
                self.send_cmd("disabled.en=1")
            return

        if self.state.machine_state == MachineState.BOARD:
            self.send_cmd("login_mov.en=1")
            role = _as_str(self.state.tag.get("role"))
            if role in {"player", "ghost", "revival"}:
                self.send_cmd("page player_main")
            elif role == "tagger":
                self.send_cmd("page tagger_main")
            self.state.machine_state = MachineState.REVIVAL
            self.login_timer.deleteTimer(self.state.login_timer_id)
            self.state.login_complete = True
