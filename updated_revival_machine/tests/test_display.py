from __future__ import annotations

from harness.model import MachineState


def test_send_cmd_adds_english_page_prefix(logic):
    logic.state.shift_machine["selected_language"] = "EN"
    logic.send_cmd("page chip_login")

    assert logic.display.commands == ["page Echip_login"]


def test_send_cmd_keeps_non_page_command(logic):
    logic.state.shift_machine["selected_language"] = "EN"
    logic.send_cmd("sleep=0")

    assert logic.display.commands == ["sleep=0"]


def test_nextion_login_sets_timer_and_page(logic):
    state = logic.state
    state.my["life_chip"] = 1
    state.my["device_state"] = "activate"

    logic.nextion_received("login")

    assert state.machine_state == MachineState.BOARD
    assert state.login_timer_id > 0
    assert logic.login_timer.isEnabled(state.login_timer_id)
    assert logic.display.commands[-1] == "page chip_login"


def test_nextion_login_restarts_existing_timer(logic):
    state = logic.state
    state.my["life_chip"] = 1
    state.my["device_state"] = "activate"
    logic.nextion_received("login")
    first_id = state.login_timer_id
    logic.display.clear()

    logic.nextion_received("login")

    assert state.login_timer_id == first_id
    assert logic.login_timer.restart_calls == [first_id]
    assert logic.display.commands == []
