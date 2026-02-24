from __future__ import annotations

from harness.model import GameState, MachineState


def _payload(prefix: str) -> list[int]:
    result = [0] * 32
    for i, ch in enumerate(prefix[:4]):
        result[i] = ord(ch)
    return result


def test_activate_to_board_login_flow(logic, prime):
    state = logic.state
    prime()

    state.my["game_state"] = "activate"
    state.my["device_state"] = "activate"
    state.my["life_chip"] = 1
    logic.data_change()
    assert state.game_state == GameState.ACTIVATE

    logic.display.clear()
    logic.nextion_received("login")
    assert state.machine_state == MachineState.BOARD
    assert "page chip_login" in logic.display.commands

    logic.display.clear()
    state.tag["role"] = "player"
    logic.card_checking(_payload("PLY1"))

    assert logic.display.commands[0] == "login_mov.en=1"
    assert logic.display.commands[1] == "page player_main"
    assert state.machine_state == MachineState.REVIVAL
    assert state.login_complete is True
