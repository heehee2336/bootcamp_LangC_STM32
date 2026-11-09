"""시리얼 연결 계층: ST-Link 포트 자동 감지, 연결/해제, 송수신.

- find_stlink_port(): USB VID 0x0483(STMicroelectronics) 포트를 찾는다.
- SerialLink      : pyserial로 실제 보드와 통신한다. 수신은 별도 스레드에서 하고
                    Qt 시그널로 메인(GUI) 스레드에 넘긴다.
- SimulatedLink   : 보드 없이 테스트할 때 쓰는 가짜 엘리베이터. (python main.py --sim)
"""
import random
import threading
import time

import serial
import serial.tools.list_ports
from PySide6.QtCore import QObject, QTimer, Signal

import protocol as P

STLINK_VID = 0x0483
BAUD_RATE = 115200
SIM_PORT_NAME = "SIM"


def find_stlink_port(simulate=False):
    """찾은 포트를 (장치 이름, 설명)으로 돌려준다. 없으면 None."""
    if simulate:
        return SIM_PORT_NAME, "가상 엘리베이터 보드 (시뮬레이션)"
    for port in serial.tools.list_ports.comports():
        if port.vid == STLINK_VID:
            return port.device, port.description
    return None


class SerialLink(QObject):
    data_received = Signal(bytes)
    connection_lost = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._ser = None
        self._thread = None
        self._running = False

    @property
    def is_open(self):
        return self._ser is not None

    def open(self, port_name):
        # 실패하면 serial.SerialException이 그대로 올라간다 (호출한 쪽에서 표시)
        self._ser = serial.Serial(port_name, BAUD_RATE, bytesize=8, parity="N",
                                  stopbits=1, timeout=0.05)
        self._running = True
        self._thread = threading.Thread(target=self._read_loop, daemon=True)
        self._thread.start()

    def close(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=0.5)
            self._thread = None
        if self._ser:
            try:
                self._ser.close()
            except serial.SerialException:
                pass
            self._ser = None

    def write(self, raw: bytes):
        if not self._ser:
            return
        try:
            self._ser.write(raw)
        except serial.SerialException as e:
            self._lost(f"송신 중 연결이 끊겼어요: {e}")

    def _read_loop(self):
        while self._running:
            try:
                chunk = self._ser.read(self._ser.in_waiting or 1)
            except (serial.SerialException, OSError, TypeError) as e:
                if self._running:
                    self._lost(f"USB 연결이 끊겼어요: {e}")
                return
            if chunk:
                self.data_received.emit(chunk)   # 스레드 간 시그널은 Qt가 안전하게 넘겨준다

    def _lost(self, reason):
        self._running = False
        self.connection_lost.emit(reason)


class SimulatedLink(QObject):
    """펌웨어(elevator.c, uart_protocol.c)처럼 움직이는 가짜 엘리베이터.

    카 위치는 천장까지 거리(mm)로 갖고 있고, 1F 95mm / 2F 58mm / 3F 28mm 에서
    포토인터럽터가 걸린다고 본다.
    """
    data_received = Signal(bytes)
    connection_lost = Signal(str)

    SPEED_MM_S = 12.0          # 카 이동 속도
    PC_MOVE_TIMEOUT = 0.3      # 펌웨어와 같이 300ms 동안 수동 명령이 없으면 정지
    TICK_MS = 50

    def __init__(self, parent=None):
        super().__init__(parent)
        self._open = False
        self._parser = P.PacketParser()
        # 펌웨어 초기 상태: EMERGENCY / 수동, 1층
        self.mode = P.MODE_EMERGENCY
        self.escaping = False
        self.state = P.STATE_IDLE
        self.current = 1
        self.target = 1
        self.pos_mm = float(P.FLOOR_MM[1])
        self.pc_move = P.MOVE_STOP
        self.pc_move_time = 0.0
        self.temp = 22.0
        self.humid = 45.0
        self._timer = QTimer(self, interval=self.TICK_MS, timeout=self._tick)

    @property
    def is_open(self):
        return self._open

    def open(self, port_name):
        self._open = True
        self._timer.start()

    def close(self):
        self._open = False
        self._timer.stop()

    def write(self, raw: bytes):
        for kind, cmd, data, _ in self._parser.feed(raw):
            if kind != "ok":
                continue                      # 펌웨어와 같이: 체크섬 오류면 응답 없음
            reply = self._handle(cmd, data)
            if reply:
                delay = random.randint(3, 12)   # 실제 보드처럼 약간의 지연
                QTimer.singleShot(delay, lambda r=reply: self._open and self.data_received.emit(r))

    # ---------------------------------------------------------- 명령 처리
    def _result(self, rsp, ok):
        return P.build_packet(rsp, [P.RESULT_OK if ok else P.RESULT_REJECTED])

    def _handle(self, cmd, data):
        if cmd == P.CMD_STATUS_REQ:
            return P.build_packet(P.RSP_STATUS, self._status_bytes())
        if cmd == P.CMD_CALL_FLOOR:
            if len(data) != 1 or not 1 <= data[0] <= 3:
                return P.build_packet(P.RSP_CALL_FLOOR, [P.RESULT_BAD_PARAM])
            ok = self.mode == P.MODE_NORMAL and self.state == P.STATE_IDLE
            if ok and data[0] != self.current:
                self.target = data[0]
                self.state = P.STATE_RISING if self.target > self.current else P.STATE_FALLING
            return self._result(P.RSP_CALL_FLOOR, ok)
        if cmd == P.CMD_EMERGENCY:
            ok = self.mode == P.MODE_NORMAL
            if ok:
                self.mode, self.escaping, self.pc_move = P.MODE_EMERGENCY, False, P.MOVE_STOP
            return self._result(P.RSP_EMERGENCY, ok)
        if cmd == P.CMD_ESCAPE:
            ok = self.mode == P.MODE_EMERGENCY and not self.escaping
            if ok:
                self.escaping, self.pc_move = True, P.MOVE_STOP
            return self._result(P.RSP_ESCAPE, ok)
        if cmd == P.CMD_MANUAL_MOVE:
            if len(data) != 1 or data[0] > P.MOVE_DOWN:
                return P.build_packet(P.RSP_MANUAL_MOVE, [P.RESULT_BAD_PARAM])
            ok = self.mode == P.MODE_EMERGENCY and not self.escaping
            if ok:
                self.pc_move, self.pc_move_time = data[0], time.monotonic()
            return self._result(P.RSP_MANUAL_MOVE, ok)
        return None                           # 알 수 없는 CMD -> 무응답

    def _zone(self):
        mm = round(self.pos_mm)
        if mm >= 93:
            return 1
        if mm >= 63:
            return 2
        if mm >= 56:
            return 3
        if mm >= 31:
            return 4
        return 5

    def _status_bytes(self):
        mm = int(round(self.pos_mm + random.uniform(-1.5, 1.5)))   # 초음파 흔들림
        t, h = round(self.temp, 1), round(self.humid, 1)
        return [self.mode, 1 if self.escaping else 0, self.state, self.current, self.target,
                self._zone(), mm >> 8, mm & 0xFF,
                int(t), round(t * 10) % 10, int(h), round(h * 10) % 10]

    # ---------------------------------------------------------- 움직임
    def _tick(self):
        step = self.SPEED_MM_S * self.TICK_MS / 1000      # 위로 = 천장과 가까워짐 = mm 감소
        self.temp = min(23.5, max(20.0, self.temp + random.uniform(-0.05, 0.05)))
        self.humid = min(60.0, max(35.0, self.humid + random.uniform(-0.2, 0.2)))

        if self.mode == P.MODE_NORMAL:
            if self.state in (P.STATE_RISING, P.STATE_FALLING):
                goal = P.FLOOR_MM[self.target]
                self.pos_mm += -step if self.state == P.STATE_RISING else step
                if (self.state == P.STATE_RISING and self.pos_mm <= goal) or \
                        (self.state == P.STATE_FALLING and self.pos_mm >= goal):
                    self.pos_mm = goal                   # 포토인터럽터 감지
                    self.current = self.target
                    self.state = P.STATE_IDLE
            return

        # EMERGENCY
        if self.escaping:
            self.pos_mm += step
            if self.pos_mm >= P.FLOOR_MM[1]:             # 1층 포토인터럽터
                self.pos_mm = P.FLOOR_MM[1]
                self.current = self.target = 1
                self.mode, self.escaping, self.state = P.MODE_NORMAL, False, P.STATE_IDLE
            return
        if self.pc_move != P.MOVE_STOP and time.monotonic() - self.pc_move_time > self.PC_MOVE_TIMEOUT:
            self.pc_move = P.MOVE_STOP
        if self.pc_move == P.MOVE_UP:
            self.pos_mm = max(20.0, self.pos_mm - step)
        elif self.pc_move == P.MOVE_DOWN:
            self.pos_mm = min(105.0, self.pos_mm + step)
