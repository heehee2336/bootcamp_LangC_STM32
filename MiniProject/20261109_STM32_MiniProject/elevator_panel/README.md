# 엘리베이터 컨트롤 패널

STM32 Nucleo-F411RE 엘리베이터 키트를 UART 바이너리 패킷으로 감시·제어하는 PySide6 데스크톱 프로그램이에요.
Nucleo 컨트롤 패널과 같은 파스텔 하늘 템플릿을 썼어요.

## 실행

```
pip install -r requirements.txt
python main.py          # 실제 보드 (펌웨어 project_elevator_uart.zip)
python main.py --sim    # 보드 없이 가상 엘리베이터로 실행
python protocol.py      # 프로토콜 자체 테스트
```

## 화면

| 영역 | 내용 |
|---|---|
| 연결 | ST-Link(VID 0x0483) 포트 자동 감지 → "연결" 버튼 |
| 엘리베이터 | 모드 배지(NORMAL / EMERGENCY 깜빡임 / 탈출 중), 승강로 그림(초음파 거리로 카 위치), 현재 층 또는 "1F → 3F", 동작 |
| 조작 | NORMAL: 1F·2F·3F 호출, EMERGENCY 전환 / EMERGENCY: ▲상승 ▼하강(누르는 동안), 탈출·1층 복귀 |
| 센서 | 온도, 습도, 천장까지 거리(0.1cm), 초음파 위치 판단(1F, 1F~2F …) — 모드와 상관없이 항상 갱신 |
| 통신 로그 | TX/RX HEX + 해석, 상태 변화(EVT: 모드 변경, 출발, 도착)는 항상 표시 |

버튼은 지금 상태에서 펌웨어가 받아 주는 것만 켜져요 (예: 층 호출은 NORMAL 정지 중일 때만).

## 프로토콜 (uart_protocol.md 기준)

`AA | LEN | CMD | DATA | CHK`, CHK = (LEN + CMD + DATA 합) & 0xFF, 응답 CMD = 요청 | 0x80

| 요청 | DATA | 응답 |
|---|---|---|
| 0x10 상태 조회 | 없음 | 0x90 상태 12바이트 |
| 0x20 층 호출 | 층 1~3 | 0xA0 결과 |
| 0x21 EMERGENCY 전환 | 없음 | 0xA1 결과 |
| 0x22 탈출 | 없음 | 0xA2 결과 |
| 0x23 수동 조작 | 0 정지 / 1 상승 / 2 하강 | 0xA3 결과 |

## 동작 규칙

- 상태 조회는 0.2초마다 보내요.
- ▲/▼를 누르고 있는 동안 0.1초마다 수동 조작을 다시 보내고, 떼면 정지를 한 번 보내요.
  펌웨어는 0.3초 동안 명령이 없으면 스스로 멈춰요 (연결이 끊겨도 안전).
- 응답을 300ms 안에 못 받으면 오류 로그를 남기고 다시 보내지 않아요.
- 같은 종류의 요청은 응답을 받거나 타임아웃이 날 때까지 다시 보내지 않아요.

## 파일

| 파일 | 역할 |
|---|---|
| `main.py` | 실행 시작점 (글꼴, 스타일) |
| `protocol.py` | 패킷 생성, 체크섬, 수신 상태 머신, 상태 응답 해석 |
| `serial_link.py` | 포트 감지, pyserial 송수신, 가상 엘리베이터 |
| `main_window.py` | 화면 구성, 타이머, 버튼 활성화 규칙, 로그 |
| `widgets.py` | 하늘 배경, 카드, 승강로 그림, 모드 배지 |
| `style.qss` | 색과 테두리 |
| `fonts/` | Black Han Sans, IBM Plex Sans KR, JetBrains Mono (OFL) |
