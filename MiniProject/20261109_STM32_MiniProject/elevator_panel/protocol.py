"""STM32 <-> PC 엘리베이터 바이너리 패킷 프로토콜 (uart_protocol.c / uart_protocol.md와 같은 형식)

패킷:  AA | LEN | CMD | DATA ... | CHK
  - LEN = CMD 1바이트 + DATA 바이트 수
  - CHK = (LEN + CMD + DATA 합)의 하위 8비트 (시작 바이트 AA는 넣지 않음)
  - 응답 CMD = 요청 CMD | 0x80
"""
import time
from dataclasses import dataclass

PKT_START = 0xAA
PKT_MAX_LEN = 16          # 펌웨어와 같이: LEN은 1 이상, PKT_MAX_LEN 미만

# 요청 (PC -> STM32)
CMD_STATUS_REQ = 0x10     # DATA 없음
CMD_CALL_FLOOR = 0x20     # DATA 1: 층 1~3
CMD_EMERGENCY = 0x21      # DATA 없음: NORMAL -> EMERGENCY
CMD_ESCAPE = 0x22         # DATA 없음: EMERGENCY 탈출 (1층 복귀)
CMD_MANUAL_MOVE = 0x23    # DATA 1: 0 정지, 1 상승, 2 하강

# 응답 (STM32 -> PC)
RSP_STATUS = 0x90
RSP_CALL_FLOOR = 0xA0
RSP_EMERGENCY = 0xA1
RSP_ESCAPE = 0xA2
RSP_MANUAL_MOVE = 0xA3

RESULT_OK = 0x00
RESULT_REJECTED = 0x01
RESULT_BAD_PARAM = 0x02
RESULT_TEXT = {RESULT_OK: "OK", RESULT_REJECTED: "거절 (지금 상태에서는 불가)",
               RESULT_BAD_PARAM: "잘못된 값"}

MOVE_STOP, MOVE_UP, MOVE_DOWN = 0, 1, 2
MOVE_TEXT = {MOVE_STOP: "정지", MOVE_UP: "상승", MOVE_DOWN: "하강"}

# 상태 값
MODE_NORMAL, MODE_EMERGENCY = 0, 1
STATE_IDLE, STATE_RISING, STATE_FALLING, STATE_ARRIVE = 0, 1, 2, 3
STATE_TEXT = {STATE_IDLE: "정지", STATE_RISING: "상승 중", STATE_FALLING: "하강 중",
              STATE_ARRIVE: "도착"}
ZONE_TEXT = {0: "--", 1: "1F", 2: "1F~2F", 3: "2F", 4: "2F~3F", 5: "3F"}
DIST_INVALID = 0xFFFF

# 초음파 층 판단 경계 (펌웨어 distance.c와 같은 값, mm)
FLOOR_MM = {1: 95, 2: 58, 3: 28}     # 실측 위치


def response_cmd_for(request_cmd: int) -> int:
    return request_cmd | 0x80


def checksum(length: int, cmd: int, data: bytes) -> int:
    return (length + cmd + sum(data)) & 0xFF


def build_packet(cmd: int, data: bytes = b"") -> bytes:
    data = bytes(data)
    length = len(data) + 1
    if length >= PKT_MAX_LEN:
        raise ValueError(f"DATA가 너무 길어요 ({len(data)}바이트)")
    return bytes([PKT_START, length, cmd, *data, checksum(length, cmd, data)])


def hex_str(raw: bytes) -> str:
    return " ".join(f"{b:02X}" for b in raw)


@dataclass
class Status:
    """STATUS_RSP(0x90) 12바이트를 풀어 놓은 것."""
    mode: int
    escaping: bool
    state: int
    current_floor: int      # 1~3
    target_floor: int       # 1~3
    zone: int               # 0~5
    distance_mm: int        # DIST_INVALID 이면 측정값 없음
    temp: str               # "24.5"
    humid: str              # "40.0"

    @classmethod
    def parse(cls, data: bytes):
        if len(data) != 12:
            raise ValueError(f"상태 응답은 12바이트여야 해요 ({len(data)}바이트)")
        return cls(mode=data[0], escaping=bool(data[1]), state=data[2],
                   current_floor=data[3], target_floor=data[4], zone=data[5],
                   distance_mm=(data[6] << 8) | data[7],
                   temp=f"{data[8]}.{data[9]}", humid=f"{data[10]}.{data[11]}")

    @property
    def has_distance(self):
        return self.distance_mm != DIST_INVALID

    @property
    def moving(self):
        return self.state in (STATE_RISING, STATE_FALLING)

    def summary(self):
        mode = "EMERGENCY" if self.mode == MODE_EMERGENCY else "NORMAL"
        if self.mode == MODE_EMERGENCY:
            where = ZONE_TEXT.get(self.zone, "--")
            sub = "탈출 중" if self.escaping else "수동"
        elif self.moving:
            where = f"{self.current_floor}F→{self.target_floor}F"
            sub = STATE_TEXT[self.state]
        else:
            where = f"{self.current_floor}F"
            sub = STATE_TEXT.get(self.state, "?")
        dist = f"{self.distance_mm / 10:.1f}cm" if self.has_distance else "--cm"
        return f"{mode} · {sub} · {where} · {dist} · {self.temp}°C {self.humid}%"


class PacketParser:
    """수신 바이트를 하나씩 넣으면 완성된 패킷을 돌려주는 상태 머신.

    feed()는 (종류, cmd, data, raw) 튜플 목록을 돌려준다.
      종류 "ok"  : 체크섬이 맞는 패킷
      종류 "bad" : 체크섬이 틀린 패킷
    """
    WAIT_START, WAIT_LEN, WAIT_DATA, WAIT_CHK = range(4)
    BYTE_GAP_TIMEOUT = 0.1   # 패킷 도중 100ms 이상 끊기면 처음부터 다시

    def __init__(self):
        self.reset()

    def reset(self):
        self.state = self.WAIT_START
        self.length = 0
        self.buf = bytearray()
        self.last_byte_time = 0.0

    def feed(self, chunk: bytes):
        packets = []
        now = time.monotonic()
        if self.state != self.WAIT_START and now - self.last_byte_time > self.BYTE_GAP_TIMEOUT:
            self.reset()
        self.last_byte_time = now

        for b in chunk:
            if self.state == self.WAIT_START:
                if b == PKT_START:
                    self.state = self.WAIT_LEN
            elif self.state == self.WAIT_LEN:
                if b == 0 or b >= PKT_MAX_LEN:
                    self.state = self.WAIT_START
                else:
                    self.length = b
                    self.buf = bytearray()
                    self.state = self.WAIT_DATA
            elif self.state == self.WAIT_DATA:
                self.buf.append(b)
                if len(self.buf) >= self.length:
                    self.state = self.WAIT_CHK
            elif self.state == self.WAIT_CHK:
                cmd, data = self.buf[0], bytes(self.buf[1:])
                raw = bytes([PKT_START, self.length, *self.buf, b])
                kind = "ok" if checksum(self.length, cmd, data) == b else "bad"
                packets.append((kind, cmd, data, raw))
                self.state = self.WAIT_START
        return packets


if __name__ == "__main__":
    # 간단한 자체 테스트: python protocol.py   (예시는 uart_protocol.md와 같음)
    assert build_packet(CMD_STATUS_REQ) == bytes.fromhex("AA 01 10 11")
    assert build_packet(CMD_CALL_FLOOR, b"\x03") == bytes.fromhex("AA 02 20 03 25")
    assert build_packet(CMD_EMERGENCY) == bytes.fromhex("AA 01 21 22")
    assert build_packet(CMD_ESCAPE) == bytes.fromhex("AA 01 22 23")
    assert build_packet(CMD_MANUAL_MOVE, b"\x01") == bytes.fromhex("AA 02 23 01 26")
    assert response_cmd_for(CMD_MANUAL_MOVE) == RSP_MANUAL_MOVE

    p = PacketParser()
    stream = bytes.fromhex("00 AA 0D 90 00 00 00 02 03 03 01 1F 18 05 28 00 0A AA 02 A0 00 A2 AA 02 A0 00 00")
    got = p.feed(stream[:7]) + p.feed(stream[7:])
    assert [(k, c) for k, c, _, _ in got] == [("ok", 0x90), ("ok", 0xA0), ("bad", 0xA0)]
    st = Status.parse(got[0][2])
    assert (st.current_floor, st.target_floor, st.zone, st.distance_mm, st.temp, st.humid) == \
        (2, 3, 3, 287, "24.5", "40.0")
    print("protocol tests passed")
