from __future__ import annotations

import json
from pathlib import Path

import pytest

from harness.logic_adapter import RevivalLogic
from harness.model import HarnessState


def _fixture_path(name: str) -> Path:
    return Path(__file__).resolve().parents[1] / "harness" / "fixtures" / name


def _load_json(name: str) -> dict:
    with _fixture_path(name).open("r", encoding="utf-8") as f:
        return json.load(f)


@pytest.fixture
def state() -> HarnessState:
    return HarnessState(
        my=_load_json("base_my.json"),
        tag=_load_json("base_tag.json"),
    )


@pytest.fixture
def logic(state: HarnessState) -> RevivalLogic:
    return RevivalLogic(state=state)


@pytest.fixture
def prime(logic: RevivalLogic):
    def _prime() -> None:
        logic.data_change()
        logic.display.clear()
        logic.wifi.clear()

    return _prime
