from __future__ import annotations

from harness.model import BLUE, RED, YELLOW, GameState


def test_ready_transition_updates_page_and_pixels(logic, prime):
    state = logic.state
    prime()

    state.my["game_state"] = "ready"
    state.my["device_state"] = "ready"
    logic.data_change()

    assert state.game_state == GameState.READY
    assert logic.display.commands[-1] == "page before_tagger"
    assert logic.pixels_top.last_color == RED
    assert logic.pixels_mid.last_color == RED
    assert logic.pixels_bot.last_color == RED


def test_activate_transition_with_life_chip(logic, prime):
    state = logic.state
    prime()

    state.my["game_state"] = "activate"
    state.my["life_chip"] = 2
    logic.data_change()

    assert state.game_state == GameState.ACTIVATE
    assert logic.display.commands[:2] == ["sleep=0", "page login"]
    assert logic.pixels_top.last_color == YELLOW
    assert logic.pixels_mid.last_color == YELLOW
    assert logic.pixels_bot.last_color == YELLOW


def test_device_state_player_win_forces_setting(logic, prime):
    state = logic.state
    prime()

    state.game_state = GameState.ACTIVATE
    state.my["device_state"] = "player_win"
    logic.data_change()

    assert state.game_state == GameState.SETTING
    assert logic.display.commands[-1] == "page win"


def test_manage_state_mu_sends_wifi_and_no_chip_page(logic, prime):
    state = logic.state
    prime()

    state.my["device_name"] = "revival_01"
    state.my["manage_state"] = "mu"
    logic.data_change()

    assert ("revival_01", "device_state", "used") in logic.wifi.send_log
    assert ("revival_01", "life_chip", "-1") in logic.wifi.send_log
    assert logic.display.commands[-1] == "page no_life_chip"
    assert logic.pixels_top.last_color == BLUE
    assert logic.pixels_mid.last_color == BLUE
    assert logic.pixels_bot.last_color == BLUE
