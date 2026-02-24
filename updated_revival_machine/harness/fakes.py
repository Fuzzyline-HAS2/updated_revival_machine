from __future__ import annotations

from dataclasses import dataclass
from typing import Callable


class FakeDisplay:
    def __init__(self) -> None:
        self.commands: list[str] = []

    def send_command(self, command: str) -> None:
        self.commands.append(command)

    def clear(self) -> None:
        self.commands.clear()


class FakeWifi:
    def __init__(self) -> None:
        self.setup_log: list[str] = []
        self.send_log: list[tuple[str, str, str]] = []
        self.receive_log: list[str] = []
        self.loop_callbacks: list[Callable[[], None]] = []

    def Setup(self, ssid: str) -> None:
        self.setup_log.append(ssid)

    def Send(self, device_name: str, key: str, value: str) -> None:
        self.send_log.append((str(device_name), str(key), str(value)))

    def Receive(self, tag_user: str) -> None:
        self.receive_log.append(str(tag_user))

    def Loop(self, callback: Callable[[], None]) -> None:
        self.loop_callbacks.append(callback)

    def clear(self) -> None:
        self.setup_log.clear()
        self.send_log.clear()
        self.receive_log.clear()
        self.loop_callbacks.clear()


class FakePixelStrip:
    def __init__(self, name: str) -> None:
        self.name = name
        self.last_color: tuple[int, int, int] | None = None
        self.history: list[tuple[int, int, int]] = []

    def begin(self) -> None:
        return None

    def lightColor(self, color: tuple[int, int, int] | list[int]) -> None:
        normalized = tuple(int(v) for v in color)
        self.last_color = normalized
        self.history.append(normalized)


@dataclass
class _TimerEntry:
    interval_ms: int
    due_ms: int
    callback: Callable[[], None]
    repeat: bool
    enabled: bool = True


class FakeTimer:
    def __init__(self) -> None:
        self._entries: dict[int, _TimerEntry] = {}
        self._next_id = 1
        self.now_ms = 0
        self.restart_calls: list[int] = []
        self.fire_log: list[int] = []

    def setTimeout(self, delay_ms: int, callback: Callable[[], None]) -> int:
        timer_id = self._next_id
        self._next_id += 1
        interval = max(0, int(delay_ms))
        self._entries[timer_id] = _TimerEntry(
            interval_ms=interval,
            due_ms=self.now_ms + interval,
            callback=callback,
            repeat=False,
        )
        return timer_id

    def setInterval(self, interval_ms: int, callback: Callable[[], None]) -> int:
        timer_id = self._next_id
        self._next_id += 1
        interval = max(1, int(interval_ms))
        self._entries[timer_id] = _TimerEntry(
            interval_ms=interval,
            due_ms=self.now_ms + interval,
            callback=callback,
            repeat=True,
        )
        return timer_id

    def isEnabled(self, timer_id: int) -> bool:
        entry = self._entries.get(timer_id)
        return bool(entry and entry.enabled)

    def restartTimer(self, timer_id: int) -> None:
        entry = self._entries.get(timer_id)
        if not entry:
            return
        entry.enabled = True
        entry.due_ms = self.now_ms + entry.interval_ms
        self.restart_calls.append(timer_id)

    def disable(self, timer_id: int) -> None:
        entry = self._entries.get(timer_id)
        if entry:
            entry.enabled = False

    def deleteTimer(self, timer_id: int) -> None:
        self._entries.pop(timer_id, None)

    def run(self) -> None:
        self.advance(0)

    def advance(self, step_ms: int) -> None:
        self.now_ms += max(0, int(step_ms))
        fired = True
        while fired:
            fired = False
            for timer_id, entry in list(self._entries.items()):
                if not entry.enabled:
                    continue
                if self.now_ms < entry.due_ms:
                    continue
                entry.callback()
                self.fire_log.append(timer_id)
                fired = True
                if entry.repeat:
                    entry.due_ms += entry.interval_ms
                else:
                    entry.enabled = False
