# Python 테스트 하네스 초간단 가이드

## 1. 한 줄 설명
테스트 하네스는 장비 없이 Python으로 로직을 확인하는 테스트입니다.

## 2. 중요한 경로
`tests` 폴더 위치:

`C:\Users\ok\updated_revival_machine\updated_revival_machine\tests`

## 3. 실행 명령 (복붙)
```powershell
cd C:\Users\ok\updated_revival_machine\updated_revival_machine
py -3 -m pytest tests -v -p no:cacheprovider
```

## 4. 각 테스트가 보는 것 (짧게)
- `tests/test_data_change.py`: `game_state`, `device_state`, `manage_state` 변경 시 페이지/LED/Wi-Fi 동작 확인
- `tests/test_display.py`: 디스플레이 명령(`SendCmd`)과 로그인 입력(`NextionReceived`) 처리 확인
- `tests/test_timer.py`: 타이머 콜백(`LoginTimerFunc` 등) 동작 확인
- `tests/test_card_checking.py`: RFID 태그 처리 핵심 분기(칩 전송, 보드 로그인, 비활성 처리) 확인
- `tests/test_end_to_end_scenarios.py`: 활성화 -> 로그인 -> 카드 태그 전체 흐름 확인

## 5. 결과 보는 법
- `PASSED` = 성공
- `FAILED` = 코드 로직 문제
- `ERROR` = 경로/설치 문제

## 6. 자주 나는 오류
### `file or directory not found: tests`
안쪽 폴더로 이동해서 다시 실행:
```powershell
cd C:\Users\ok\updated_revival_machine\updated_revival_machine
py -3 -m pytest tests -v -p no:cacheprovider
```

### `No module named pytest`
```powershell
py -3 -m pip install pytest
```

### `Python`만 출력되고 멈춤
`python` 대신 `py -3` 사용:
```powershell
py -3 -m pytest tests -v -p no:cacheprovider
```

## 7. 파일 위치
- 테스트: `tests/`
- 하네스 코드: `harness/`
- 실행 스크립트: `scripts/test_harness.ps1`
