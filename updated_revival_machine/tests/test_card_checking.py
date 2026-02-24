from __future__ import annotations

from harness.model import BLUE, GameState, MachineState


def _payload(prefix: str) -> list[int]:
    result = [0] * 32
    for i, ch in enumerate(prefix[:4]):
        result[i] = ord(ch)
    return result


def test_card_checking_revival_player_transfer(logic):
    state = logic.state
    state.machine_state = MachineState.REVIVAL
    state.game_state = GameState.ACTIVATE
    state.my["life_chip"] = 2
    state.my["device_state"] = "activate"
    state.my["device_name"] = "revival_01"
    state.tag["role"] = "player"
    state.tag["life_chip"] = 0
    state.tag["max_life_chip"] = 3
    state.tag["device_name"] = "player_01"

    logic.card_checking(_payload("ABCD"))

    assert logic.display.commands[:2] == ["page lifechip_send", "collect.en=1"]
    assert "page no_life_chip" in logic.display.commands
    assert "collect_succ.en=1" in logic.display.commands
    assert ("revival_01", "device_state", "used") in logic.wifi.send_log
    assert ("revival_01", "life_chip", "-1") in logic.wifi.send_log
    assert ("player_01", "life_chip", "+1") in logic.wifi.send_log
    assert ("player_01", "exp", "+90") in logic.wifi.send_log
    assert logic.pixels_top.last_color == BLUE
    assert logic.pixels_mid.last_color == BLUE
    assert logic.pixels_bot.last_color == BLUE


def test_card_checking_board_tagger_login(logic):
    state = logic.state
    state.machine_state = MachineState.BOARD
    state.tag["role"] = "tagger"
    state.login_timer_id = logic.login_timer.setTimeout(7000, logic.login_timer_func)

    logic.card_checking(_payload("TAG1"))

    assert logic.display.commands[0] == "login_mov.en=1"
    assert logic.display.commands[1] == "page tagger_main"
    assert state.machine_state == MachineState.REVIVAL
    assert state.login_complete is True
    assert logic.login_timer.isEnabled(state.login_timer_id) is False


def test_card_checking_used_state_sends_disabled(logic):
    state = logic.state
    state.machine_state = MachineState.REVIVAL
    state.game_state = GameState.READY
    state.my["device_state"] = "used"
    state.my["life_chip"] = 1
    state.tag["role"] = "ghost"

    logic.card_checking(_payload("USED"))

    assert "disabled.en=1" in logic.display.commands
