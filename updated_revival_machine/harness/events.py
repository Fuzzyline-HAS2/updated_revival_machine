from __future__ import annotations

from typing import Any, Iterable

from .logic_adapter import RevivalLogic


def apply_event_sequence(
    logic: RevivalLogic,
    sequence: Iterable[tuple[str, tuple[Any, ...], dict[str, Any]]],
) -> None:
    for method_name, args, kwargs in sequence:
        method = getattr(logic, method_name)
        method(*args, **kwargs)
