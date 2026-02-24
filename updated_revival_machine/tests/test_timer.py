from __future__ import annotations

from harness.model import GameState, MachineState


def test_login_timer_func_activate_with_life_chip(logic):
    state = logic.state
    state.game_state = GameState.ACTIVATE
    state.my["life_chip"] = 1

    logic.login_timer_func()

    assert state.machine_state == MachineState.REVIVAL
    assert logic.display.commands[-1] == "page chip"


def test_login_timer_func_without_life_chip(logic):
    state = logic.state
    state.game_state = GameState.READY
    state.my["life_chip"] = 0

    logic.login_timer_func()

    assert logic.display.commands[-1] == "page no_life_chip"


def test_rfid_tag_timer_func_clears_flag(logic):
    logic.state.rfid_tag = True

    logic.rfid_tag_timer_func()

    assert logic.state.rfid_tag is False


def test_wifi_timer_func_registers_data_change_callback(logic):
    logic.wifi_timer_func()

    assert len(logic.wifi.loop_callbacks) == 1
    assert logic.wifi.loop_callbacks[0].__name__ == "data_change"
