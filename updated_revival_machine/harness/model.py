from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum
from typing import Any


class MachineState(str, Enum):
    REVIVAL = "revival"
    BOARD = "board"


class GameState(str, Enum):
    SETTING = "setting"
    READY = "ready"
    READY_ACTIVATE = "ready_activate"
    ACTIVATE = "activate"


WHITE = (20, 20, 20)
RED = (20, 0, 0)
YELLOW = (20, 20, 0)
GREEN = (0, 20, 0)
PURPLE = (20, 0, 20)
BLUE = (0, 0, 20)


DEFAULT_MY = {
    "game_state": "setting",
    "device_state": "ready",
    "manage_state": "",
    "life_chip": 1,
    "device_name": "revival_machine_01",
}

DEFAULT_TAG = {
    "role": "player",
    "life_chip": 0,
    "max_life_chip": 3,
    "device_name": "player_01",
}

DEFAULT_SHIFT_MACHINE = {"selected_language": "KR"}


@dataclass
class HarnessState:
    my: dict[str, Any] = field(default_factory=lambda: dict(DEFAULT_MY))
    tag: dict[str, Any] = field(default_factory=lambda: dict(DEFAULT_TAG))
    shift_machine: dict[str, Any] = field(default_factory=lambda: dict(DEFAULT_SHIFT_MACHINE))
    machine_state: MachineState = MachineState.REVIVAL
    game_state: GameState = GameState.SETTING
    currner_page: str = ""
    login_complete: bool = False
    cur_tag_user: str = ""
    rfid_tag: bool = False
    restarted: bool = False
    login_timer_id: int = -1
    rfid_timer_id: int = -1
    wifi_timer_id: int = -1
    tag_receive_timer_id: int = -1
    tag_user_tag_num: int = 0
