"""메인 화면: 연결 영역, 엘리베이터 상태, 조작, 센서, 통신 로그."""
import html
import time
from collections import deque
from datetime import datetime

from PySide6.QtCore import QEvent, Qt, QTimer
from PySide6.QtGui import QTextCursor
from PySide6.QtWidgets import (QCheckBox, QFrame, QGridLayout, QHBoxLayout, QLabel,
                               QMainWindow, QPlainTextEdit, QPushButton, QScrollArea,
                               QSizePolicy, QVBoxLayout, QWidget)

import protocol as P
from serial_link import SerialLink, SimulatedLink, find_stlink_port, BAUD_RATE
from widgets import Card, ModeBadge, OutlinedTitle, ShaftView, SkyBackground, StatusDot

RESPONSE_TIMEOUT = 0.3        # 응답 타임아웃 300ms
STATUS_PERIOD_MS = 200        # 상태 조회 0.2초
MOVE_REPEAT_MS = 100          # 수동 조작 버튼을 누르는 동안 0.1초마다 재전송
PORT_SCAN_PERIOD_MS = 1000    # 연결 전 포트 확인 1초
MAX_LOG_ENTRIES = 3000

# 로그 분류 (sys, evt는 체크박스와 상관없이 항상 표시)
CAT_STATUS, CAT_CALL, CAT_MODE, CAT_MOVE, CAT_EVT, CAT_SYS = \
    "status", "call", "mode", "move", "evt", "sys"
ALWAYS_SHOWN = {CAT_EVT, CAT_SYS}
RSP_CATEGORY = {P.RSP_STATUS: CAT_STATUS, P.RSP_CALL_FLOOR: CAT_CALL,
                P.RSP_EMERGENCY: CAT_MODE, P.RSP_ESCAPE: CAT_MODE, P.RSP_MANUAL_MOVE: CAT_MOVE}
RSP_NAME = {P.RSP_CALL_FLOOR: "층 호출", P.RSP_EMERGENCY: "EMERGENCY 전환",
            P.RSP_ESCAPE: "탈출", P.RSP_MANUAL_MOVE: "수동 조작"}


def now_text(with_date=True):
    t = datetime.now()
    s = t.strftime("%Y-%m-%d %H:%M:%S.%f" if with_date else "%H:%M:%S.%f")
    return s[:-3]   # 밀리초까지


def describe_request(cmd, data):
    if cmd == P.CMD_STATUS_REQ:
        return "상태 조회", CAT_STATUS
    if cmd == P.CMD_CALL_FLOOR:
        return f"{data[0]}층 호출", CAT_CALL
    if cmd == P.CMD_EMERGENCY:
        return "EMERGENCY 전환 요청", CAT_MODE
    if cmd == P.CMD_ESCAPE:
        return "탈출 요청 (1층 복귀)", CAT_MODE
    if cmd == P.CMD_MANUAL_MOVE:
        return f"수동 조작: {P.MOVE_TEXT.get(data[0], '?')}", CAT_MOVE
    return f"알 수 없는 요청 0x{cmd:02X}", CAT_SYS


def label(text, name=None, parent=None):
    w = QLabel(text, parent)
    if name:
        w.setObjectName(name)
    return w


def card_head(card, title, sub):
    head = QHBoxLayout()
    head.addWidget(label(title, "cardTitle"))
    head.addWidget(label(sub, "cardSub"))
    head.addStretch()
    card.body.addLayout(head)
    return head


# ---------------------------------------------------------------- 엘리베이터 상태 카드
class ElevatorCard(Card):
    def __init__(self, parent=None):
        super().__init__(parent)
        head = card_head(self, "엘리베이터", "0.2초마다 상태 조회")
        self.badge = ModeBadge()
        head.addWidget(self.badge)

        body = QHBoxLayout()
        body.setSpacing(16)
        self.shaft = ShaftView()
        body.addWidget(self.shaft, 1)

        side = QVBoxLayout()
        side.setSpacing(10)
        box = QFrame(objectName="reading")
        lay = QVBoxLayout(box)
        lay.setContentsMargins(14, 8, 14, 10)
        lay.setSpacing(0)
        self.where_label = label("현재 층", "readingLabel")
        lay.addWidget(self.where_label)
        self.where = label("--", "floorValue")
        lay.addWidget(self.where)
        side.addWidget(box)

        box2 = QFrame(objectName="reading")
        lay2 = QVBoxLayout(box2)
        lay2.setContentsMargins(14, 8, 14, 10)
        lay2.setSpacing(2)
        lay2.addWidget(label("동작", "readingLabel"))
        self.action = label("--", "actionValue")
        self.action.setWordWrap(True)
        lay2.addWidget(self.action)
        side.addWidget(box2)
        side.addStretch()
        body.addLayout(side, 1)
        self.body.addLayout(body, 1)

        meta = QHBoxLayout()
        meta.addWidget(label("마지막 수신", "meta"))
        self.stamp = label("--:--:--.---", "metaValue")
        meta.addWidget(self.stamp)
        meta.addStretch()
        self.body.addLayout(meta)


# ---------------------------------------------------------------- 조작 카드
class ControlCard(Card):
    def __init__(self, parent=None):
        super().__init__(parent)
        card_head(self, "조작", "NORMAL / EMERGENCY")

        self.body.addWidget(label("층 호출 · NORMAL 정지 중일 때", "section"))
        floors = QHBoxLayout()
        floors.setSpacing(10)
        self.floor_btns = {}
        for n in (1, 2, 3):
            b = QPushButton(f"{n}F", objectName="floor", checkable=True)
            b.setCursor(Qt.PointingHandCursor)
            b.setAccessibleName(f"{n}층 호출")
            floors.addWidget(b)
            self.floor_btns[n] = b
        self.body.addLayout(floors)
        self.emergency_btn = QPushButton("EMERGENCY 전환", objectName="danger")
        self.emergency_btn.setCursor(Qt.PointingHandCursor)
        self.body.addWidget(self.emergency_btn)

        line = QFrame(objectName="divider")
        line.setFixedHeight(3)
        self.body.addWidget(line)

        self.body.addWidget(label("수동 조작 · EMERGENCY 일 때 (누르고 있는 동안 이동)", "section"))
        moves = QHBoxLayout()
        moves.setSpacing(10)
        self.up_btn = QPushButton("▲  상승", objectName="move")
        self.down_btn = QPushButton("▼  하강", objectName="move")
        for b in (self.up_btn, self.down_btn):
            b.setCursor(Qt.PointingHandCursor)
            moves.addWidget(b)
        self.body.addLayout(moves)
        self.escape_btn = QPushButton("탈출 · 1층 복귀", objectName="primary")
        self.escape_btn.setCursor(Qt.PointingHandCursor)
        self.body.addWidget(self.escape_btn)

        self.result = label("", "meta")
        self.result.setWordWrap(True)
        self.body.addWidget(self.result)
        self.body.addStretch()


# ---------------------------------------------------------------- 센서 카드
class SensorCard(Card):
    def __init__(self, parent=None):
        super().__init__(parent)
        card_head(self, "센서", "DHT11 · HC-SR04")
        grid = QGridLayout()
        grid.setSpacing(10)
        self.values = {}
        items = [("temp", "온도", "--.-", "°C"), ("humid", "습도", "--.-", "%RH"),
                 ("dist", "천장까지 거리", "--.-", "cm"), ("zone", "초음파 위치 판단", "--", "")]
        for i, (key, name, placeholder, unit) in enumerate(items):
            box = QFrame(objectName="reading")
            lay = QVBoxLayout(box)
            lay.setContentsMargins(14, 8, 14, 8)
            lay.setSpacing(0)
            lay.addWidget(label(name, "readingLabel"))
            row = QHBoxLayout()
            value = label(placeholder, "readingValue")
            row.addWidget(value)
            if unit:
                row.addWidget(label(unit, "readingUnit"), 0, Qt.AlignBottom)
            row.addStretch()
            lay.addLayout(row)
            grid.addWidget(box, i // 2, i % 2)
            self.values[key] = value
        self.body.addLayout(grid)
        self.body.addWidget(label("모드와 상관없이 상태 응답(0x90)으로 항상 갱신돼요", "meta"))
        self.body.addStretch()


# ---------------------------------------------------------------- 메인 창
class MainWindow(QMainWindow):
    def __init__(self, simulate=False):
        super().__init__()
        self.simulate = simulate
        self.setWindowTitle("엘리베이터 컨트롤 패널")
        self.setMinimumSize(560, 640)
        self.resize(1280, 980)

        self.link = SimulatedLink(self) if simulate else SerialLink(self)
        self.link.data_received.connect(self.on_bytes)
        self.link.connection_lost.connect(lambda reason: self.disconnect_port(reason, error=True))
        self.parser = P.PacketParser()
        self.connected = False
        self.port = None                       # (장치 이름, 설명)

        self.pending = {}                      # 응답 CMD -> (보낸 시각, 요청 설명)
        self.status = None                     # 마지막 상태 (P.Status)
        self.desired_move = P.MOVE_STOP        # 수동 조작 버튼으로 원하는 방향
        self.sent_move = P.MOVE_STOP           # 마지막으로 보낸 방향
        self.logs = deque(maxlen=MAX_LOG_ENTRIES)

        self._build_ui()

        self.scan_timer = QTimer(self, interval=PORT_SCAN_PERIOD_MS, timeout=self.scan_port)
        self.status_timer = QTimer(self, interval=STATUS_PERIOD_MS,
                                   timeout=lambda: self.send_request(P.CMD_STATUS_REQ))
        self.move_timer = QTimer(self, interval=MOVE_REPEAT_MS, timeout=self.pump_move)
        self.timeout_timer = QTimer(self, interval=50, timeout=self.check_timeouts)

        self.log(CAT_SYS, "SYS", None, "프로그램을 시작했어요. ST-Link 포트를 찾는 중이에요")
        self.set_conn_state("searching")
        self.scan_port()
        self.scan_timer.start()
        self.timeout_timer.start()

    # ------------------------------------------------------------ 화면 만들기
    def _build_ui(self):
        sky = SkyBackground()
        self.setCentralWidget(sky)
        root = QVBoxLayout(sky)
        root.setContentsMargins(0, 0, 0, 0)

        self.scroll = QScrollArea(widgetResizable=True, objectName="scroll")
        self.scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scroll.setFrameShape(QFrame.NoFrame)
        self.scroll.viewport().setAutoFillBackground(False)
        root.addWidget(self.scroll)

        content = QWidget(objectName="content")
        content.setAttribute(Qt.WA_TranslucentBackground)
        self.scroll.setWidget(content)
        page = QVBoxLayout(content)
        page.setContentsMargins(24, 20, 24, 28)
        page.setSpacing(22)

        # 제목
        header = QHBoxLayout()
        header.addWidget(OutlinedTitle("엘리베이터 컨트롤 패널"))
        header.addStretch()
        mode_note = "시뮬레이션 모드 · 가상 보드와 통신 중" if self.simulate else f"STM32 Nucleo-F411RE · {BAUD_RATE} 8N1"
        self.note = label(mode_note, "note")
        header.addWidget(self.note, 0, Qt.AlignVCenter)
        self.header = header
        page.addLayout(header)
        self.note_row = QHBoxLayout()          # 좁은 창에서는 안내 문구를 제목 아래로 내림
        page.addLayout(self.note_row)

        # 연결 영역
        conn = Card()
        row = QHBoxLayout()
        row.setSpacing(14)
        self.dot = StatusDot()
        row.addWidget(self.dot)
        texts = QVBoxLayout()
        texts.setSpacing(2)
        self.conn_title = label("", "connTitle")
        self.conn_sub = label("", "connSub")
        self.conn_sub.setWordWrap(True)
        texts.addWidget(self.conn_title)
        texts.addWidget(self.conn_sub)
        row.addLayout(texts, 1)
        self.conn_btn = QPushButton("연결", objectName="primary")
        self.conn_btn.setCursor(Qt.PointingHandCursor)
        self.conn_btn.setMinimumWidth(130)
        self.conn_btn.clicked.connect(self.on_conn_button)
        row.addWidget(self.conn_btn)
        conn.body.addLayout(row)
        page.addWidget(conn)

        # 엘리베이터 / 조작 / 센서 (창 너비에 따라 배치가 바뀜)
        self.elev_card = ElevatorCard()
        self.ctrl_card = ControlCard()
        self.sensor_card = SensorCard()
        for n, b in self.ctrl_card.floor_btns.items():
            b.clicked.connect(lambda _=False, f=n: self.call_floor(f))
        self.ctrl_card.emergency_btn.clicked.connect(lambda: self.send_request(P.CMD_EMERGENCY))
        self.ctrl_card.escape_btn.clicked.connect(self.request_escape)
        self.ctrl_card.up_btn.pressed.connect(lambda: self.set_move(P.MOVE_UP))
        self.ctrl_card.down_btn.pressed.connect(lambda: self.set_move(P.MOVE_DOWN))
        self.ctrl_card.up_btn.released.connect(lambda: self.set_move(P.MOVE_STOP))
        self.ctrl_card.down_btn.released.connect(lambda: self.set_move(P.MOVE_STOP))

        self.grid_host = QWidget()
        self.grid = QGridLayout(self.grid_host)
        self.grid.setContentsMargins(0, 0, 0, 0)
        self.grid.setSpacing(22)
        self._layout_mode = None
        page.addWidget(self.grid_host)

        # 로그
        log_card = Card()
        head = QHBoxLayout()
        head.addWidget(label("통신 로그", "cardTitle"))
        head.addStretch()
        self.filter_boxes = {}
        filters_host = QWidget()
        self.filters_layout = QHBoxLayout(filters_host)
        self.filters_layout.setContentsMargins(0, 0, 0, 0)
        self.filters_layout.setSpacing(8)
        for cat, text, on in [(CAT_CALL, "층 호출", True), (CAT_MODE, "모드 전환", True),
                              (CAT_MOVE, "수동 조작", False), (CAT_STATUS, "상태 조회", False)]:
            box = QCheckBox(text, objectName="filter", checked=on)
            box.setCursor(Qt.PointingHandCursor)
            box.toggled.connect(self.render_logs)
            self.filters_layout.addWidget(box)
            self.filter_boxes[cat] = box
        self.log_head = head
        self.filters_host = filters_host
        head.addWidget(filters_host)
        log_card.body.addLayout(head)
        self.filters_row = QHBoxLayout()      # 좁은 창에서는 체크박스를 이 줄로 내림
        log_card.body.addLayout(self.filters_row)

        self.log_view = QPlainTextEdit(objectName="log", readOnly=True)
        self.log_view.setLineWrapMode(QPlainTextEdit.NoWrap)
        self.log_view.setMaximumBlockCount(MAX_LOG_ENTRIES)
        self.log_view.setMinimumHeight(220)
        self.log_view.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        log_card.body.addWidget(self.log_view, 1)

        foot = QHBoxLayout()
        foot.addWidget(label("연결·오류·상태 변화(EVT) 로그는 체크박스와 상관없이 항상 표시돼요", "meta"))
        foot.addStretch()
        clear = QPushButton("로그 지우기", objectName="ghost")
        clear.setCursor(Qt.PointingHandCursor)
        clear.clicked.connect(self.clear_logs)
        foot.addWidget(clear)
        log_card.body.addLayout(foot)
        page.addWidget(log_card, 1)

        self.scroll.viewport().installEventFilter(self)
        self.relayout(self.width())
        self.update_controls()

    # ------------------------------------------------------------ 적응형 배치
    def eventFilter(self, obj, event):
        if obj is self.scroll.viewport() and event.type() == QEvent.Resize:
            self.relayout(event.size().width())
        return super().eventFilter(obj, event)

    def relayout(self, width):
        mode = "wide" if width >= 1100 else "medium" if width >= 700 else "narrow"
        if mode == self._layout_mode:
            return
        self._layout_mode = mode
        for w in (self.elev_card, self.ctrl_card, self.sensor_card):
            self.grid.removeWidget(w)
        for c in range(3):
            self.grid.setColumnStretch(c, 0)
        if mode == "wide":          # 엘리베이터 | 조작 | 센서
            self.grid.addWidget(self.elev_card, 0, 0)
            self.grid.addWidget(self.ctrl_card, 0, 1)
            self.grid.addWidget(self.sensor_card, 0, 2)
            self.grid.setColumnStretch(0, 11)
            self.grid.setColumnStretch(1, 8)
            self.grid.setColumnStretch(2, 8)
        elif mode == "medium":      # 엘리베이터 | 조작, 아래에 센서
            self.grid.addWidget(self.elev_card, 0, 0)
            self.grid.addWidget(self.ctrl_card, 0, 1)
            self.grid.addWidget(self.sensor_card, 1, 0, 1, 2)
            self.grid.setColumnStretch(0, 1)
            self.grid.setColumnStretch(1, 1)
        else:                       # 모두 한 줄씩 쌓기
            self.grid.addWidget(self.elev_card, 0, 0)
            self.grid.addWidget(self.ctrl_card, 1, 0)
            self.grid.addWidget(self.sensor_card, 2, 0)
            self.grid.setColumnStretch(0, 1)
        self.header.removeWidget(self.note)
        self.note_row.removeWidget(self.note)
        if mode == "wide":
            self.header.addWidget(self.note, 0, Qt.AlignVCenter)
        else:
            self.note_row.insertWidget(0, self.note)
            if self.note_row.count() == 1:
                self.note_row.addStretch()
        self.log_head.removeWidget(self.filters_host)
        self.filters_row.removeWidget(self.filters_host)
        if mode == "wide":
            self.log_head.addWidget(self.filters_host)
        else:
            self.filters_row.insertWidget(0, self.filters_host)

    # ------------------------------------------------------------ 연결 상태
    def set_conn_state(self, state):
        self.dot.set_state(state)
        if state == "searching":
            self.conn_title.setText("ST-Link 포트를 찾는 중…")
            self.conn_sub.setText("USB VID 0x0483 장치를 1초마다 확인하고 있어요")
            self.conn_btn.setText("연결")
            self.conn_btn.setEnabled(False)
            self.conn_btn.setProperty("danger", False)
        elif state == "found":
            device, desc = self.port
            self.conn_title.setText("ST-Link 포트를 찾았어요")
            self.conn_sub.setText(f"{device} · {desc} · {BAUD_RATE} 8N1")
            self.conn_btn.setText("연결")
            self.conn_btn.setEnabled(True)
            self.conn_btn.setProperty("danger", False)
        else:
            self.conn_title.setText(f"연결됨 · {self.port[0]}")
            self.conn_sub.setText(f"{BAUD_RATE} 8N1 · 0.2초마다 엘리베이터 상태를 받고 있어요")
            self.conn_btn.setText("연결 해제")
            self.conn_btn.setEnabled(True)
            self.conn_btn.setProperty("danger", True)
        self.conn_btn.style().unpolish(self.conn_btn)
        self.conn_btn.style().polish(self.conn_btn)
        active = state == "connected"
        for card in (self.elev_card, self.ctrl_card, self.sensor_card):
            card.set_active(active)

    def scan_port(self):
        if self.connected:
            return
        found = find_stlink_port(self.simulate)
        if found and found != self.port:
            self.port = found
            self.log(CAT_SYS, "SYS", None, f"ST-Link 포트를 찾았어요: {found[0]}")
            self.set_conn_state("found")
        elif not found and self.port:
            self.port = None
            self.set_conn_state("searching")

    def on_conn_button(self):
        if self.connected:
            self.disconnect_port("연결을 해제했어요")
        else:
            self.connect_port()

    def connect_port(self):
        if not self.port:
            return
        try:
            self.link.open(self.port[0])
        except Exception as e:     # 다른 프로그램이 포트를 쓰고 있을 때 등
            self.log(CAT_SYS, "SYS", None, f"{self.port[0]}에 연결하지 못했어요: {e}", error=True)
            return
        self.connected = True
        self.parser.reset()
        self.pending.clear()
        self.status = None
        self.desired_move = self.sent_move = P.MOVE_STOP
        self.scan_timer.stop()
        self.set_conn_state("connected")
        self.log(CAT_SYS, "SYS", None, f"{self.port[0]}에 연결했어요 ({BAUD_RATE} 8N1)")
        self.send_request(P.CMD_STATUS_REQ)
        self.status_timer.start()
        self.move_timer.start()
        self.update_controls()

    def disconnect_port(self, reason, error=False):
        if not self.connected:
            return
        if self.sent_move != P.MOVE_STOP:      # 움직이는 중이면 정지를 한 번 보내고 끊음
            self.link.write(P.build_packet(P.CMD_MANUAL_MOVE, [P.MOVE_STOP]))
        self.connected = False
        self.status_timer.stop()
        self.move_timer.stop()
        self.link.close()
        self.pending.clear()
        self.status = None
        self.desired_move = self.sent_move = P.MOVE_STOP
        self.log(CAT_SYS, "SYS", None, reason, error=error)
        self.port = None
        self.set_conn_state("searching")
        self.update_controls()
        self.scan_timer.start()
        self.scan_port()

    # ------------------------------------------------------------ 송신
    def send_request(self, cmd, data=b""):
        """요청을 보낸다. 같은 응답을 아직 기다리는 중이면 보내지 않고 False."""
        if not self.connected:
            return False
        rsp = P.response_cmd_for(cmd)
        if rsp in self.pending:
            return False
        raw = P.build_packet(cmd, data)
        text, cat = describe_request(cmd, data)
        self.pending[rsp] = (time.monotonic(), text)
        self.log(cat, "TX", raw, text)
        self.link.write(raw)
        return True

    def call_floor(self, floor):
        self.send_request(P.CMD_CALL_FLOOR, bytes([floor]))
        self.update_controls()

    def request_escape(self):
        self.set_move(P.MOVE_STOP)
        self.send_request(P.CMD_ESCAPE)

    def set_move(self, move):
        """▲/▼ 버튼을 누르면 상승/하강, 떼면 정지."""
        self.desired_move = move
        self.pump_move()

    def pump_move(self):
        """누르고 있는 동안 0.1초마다 같은 방향을 다시 보낸다 (펌웨어는 0.3초 동안 안 오면 정지).
        정지는 한 번만 보낸다. 앞 명령의 응답을 기다리는 중이면 다음 주기에 보낸다."""
        if not self.connected:
            return
        if self.desired_move == P.MOVE_STOP and self.sent_move == P.MOVE_STOP:
            return
        if self.send_request(P.CMD_MANUAL_MOVE, bytes([self.desired_move])):
            self.sent_move = self.desired_move

    # ------------------------------------------------------------ 수신
    def on_bytes(self, chunk):
        if not self.connected:
            return
        for kind, cmd, data, raw in self.parser.feed(chunk):
            if kind == "bad":
                self.log(CAT_SYS, "RX", raw, "체크섬 오류 · 패킷을 버렸어요", error=True)
                continue
            self.handle_packet(cmd, data, raw)

    def handle_packet(self, cmd, data, raw):
        cat = RSP_CATEGORY.get(cmd)
        if cat is None:
            self.log(CAT_SYS, "RX", raw, f"알 수 없는 응답 CMD 0x{cmd:02X}", error=True)
            return
        if self.pending.pop(cmd, None) is None:
            self.log(CAT_SYS, "RX", raw, "요청하지 않은 응답이에요 (타임아웃 뒤에 늦게 도착했을 수 있어요)", error=True)
            return

        if cmd == P.RSP_STATUS:
            try:
                st = P.Status.parse(data)
            except ValueError as e:
                self.log(CAT_SYS, "RX", raw, str(e), error=True)
                return
            self.log(cat, "RX", raw, st.summary())
            self.apply_status(st)
            return

        name = RSP_NAME[cmd]
        result = data[0] if len(data) == 1 else None
        text = P.RESULT_TEXT.get(result, "응답 길이 오류")
        self.log(cat, "RX", raw, f"{name} 응답: {text}", error=result != P.RESULT_OK)
        if cmd != P.RSP_MANUAL_MOVE or result != P.RESULT_OK:
            self.ctrl_card.result.setText(f"{now_text(False)[:8]}  {name} → {text}")
        if cmd == P.RSP_MANUAL_MOVE and result != P.RESULT_OK:
            self.desired_move = self.sent_move = P.MOVE_STOP
        self.send_request(P.CMD_STATUS_REQ)     # 결과를 화면에 빨리 반영
        self.update_controls()

    def check_timeouts(self):
        now = time.monotonic()
        for rsp, (sent, text) in list(self.pending.items()):
            if now - sent > RESPONSE_TIMEOUT:
                del self.pending[rsp]
                self.log(CAT_SYS, "SYS", None,
                         f"응답 없음 (타임아웃 {int(RESPONSE_TIMEOUT * 1000)}ms): {text}", error=True)
                self.update_controls()

    # ------------------------------------------------------------ 화면 갱신
    def apply_status(self, st):
        prev = self.status
        self.status = st
        self.log_changes(prev, st)

        emergency = st.mode == P.MODE_EMERGENCY
        e = self.elev_card
        if emergency:
            e.badge.set_mode("emergency", "탈출 중" if st.escaping else "EMERGENCY")
            e.where_label.setText("현재 위치 (초음파)")
            e.where.setText(P.ZONE_TEXT.get(st.zone, "--"))
            if st.escaping:
                e.action.setText("1층으로 하강 중 · 1층 포토인터럽터에서 정지 후 NORMAL 복귀")
            else:
                e.action.setText(f"수동 조작 · {P.MOVE_TEXT[self.sent_move]}"
                                 if self.sent_move != P.MOVE_STOP else "수동 조작 대기 · 조이스틱 또는 ▲/▼")
            direction = -1 if st.escaping else {P.MOVE_UP: 1, P.MOVE_DOWN: -1}.get(self.sent_move, 0)
        else:
            e.badge.set_mode("normal", "NORMAL")
            e.where_label.setText("현재 층")
            if st.moving:
                e.where.setText(f"{st.current_floor}F → {st.target_floor}F")
                e.action.setText(f"{st.target_floor}층으로 {P.STATE_TEXT[st.state]}")
                direction = 1 if st.state == P.STATE_RISING else -1
            else:
                e.where.setText(f"{st.current_floor}F")
                e.action.setText("정지 · 층 버튼이나 GUI로 호출하세요")
                direction = 0
        e.shaft.set_status(st.distance_mm if st.has_distance else None,
                           st.current_floor, st.target_floor, direction, emergency)
        e.stamp.setText(now_text(False))

        s = self.sensor_card.values
        s["temp"].setText(st.temp)
        s["humid"].setText(st.humid)
        s["dist"].setText(f"{st.distance_mm / 10:.1f}" if st.has_distance else "--.-")
        s["zone"].setText(P.ZONE_TEXT.get(st.zone, "--"))
        self.update_controls()

    def log_changes(self, prev, st):
        """상태가 바뀐 순간을 EVT 로그로 남긴다."""
        def mode_name(s):
            return "EMERGENCY" if s.mode == P.MODE_EMERGENCY else "NORMAL"
        if prev is None:
            self.log(CAT_EVT, "EVT", None, f"첫 상태: {st.summary()}")
            return
        if prev.mode != st.mode:
            self.log(CAT_EVT, "EVT", None, f"모드 변경: {mode_name(prev)} → {mode_name(st)}")
        if st.mode == P.MODE_EMERGENCY and st.escaping and not prev.escaping:
            self.log(CAT_EVT, "EVT", None, "탈출 시작 · 1층으로 하강")
        if st.mode == P.MODE_NORMAL and prev.mode == P.MODE_NORMAL:
            if st.moving and not prev.moving:
                self.log(CAT_EVT, "EVT", None,
                         f"출발: {st.current_floor}F → {st.target_floor}F ({P.STATE_TEXT[st.state]})")
            if prev.moving and not st.moving:
                self.log(CAT_EVT, "EVT", None, f"도착: {st.current_floor}F")

    def update_controls(self):
        """모드와 상태에 따라 누를 수 있는 버튼만 켠다."""
        st = self.status
        c = self.ctrl_card
        ok = self.connected and st is not None
        normal_idle = ok and st.mode == P.MODE_NORMAL and not st.moving
        manual = ok and st.mode == P.MODE_EMERGENCY and not st.escaping
        call_busy = P.RSP_CALL_FLOOR in self.pending
        for n, b in c.floor_btns.items():
            b.setEnabled(normal_idle and not call_busy and n != st.current_floor)
            b.setChecked(bool(ok and st.mode == P.MODE_NORMAL and st.moving and st.target_floor == n))
        c.emergency_btn.setEnabled(ok and st.mode == P.MODE_NORMAL)
        c.escape_btn.setEnabled(manual)
        for b in (c.up_btn, c.down_btn):
            if b.isEnabled() != manual:
                if not manual and b.isDown():
                    b.setDown(False)
                b.setEnabled(manual)
        if not manual and self.desired_move != P.MOVE_STOP:
            self.desired_move = P.MOVE_STOP
        c.up_btn.setProperty("active", self.sent_move == P.MOVE_UP and manual)
        c.down_btn.setProperty("active", self.sent_move == P.MOVE_DOWN and manual)
        for b in (c.up_btn, c.down_btn):
            b.style().unpolish(b)
            b.style().polish(b)

    # ------------------------------------------------------------ 로그
    def shown_categories(self):
        return {cat for cat, box in self.filter_boxes.items() if box.isChecked()} | ALWAYS_SHOWN

    @staticmethod
    def format_entry(entry):
        t, cat, direction, raw, text, error = entry
        hex_part = P.hex_str(raw) if raw else ""
        color = {"TX": "#1a45bd", "RX": "#0b8f9e", "SYS": "#e0761a", "EVT": "#c2185b"}[direction]
        msg_color = "#d3264b" if error else "#2a0b8a"
        weight = "700" if (error or direction == "EVT") else "400"
        pad = lambda s, n: html.escape(s.ljust(n)).replace(" ", "&nbsp;")
        return (f'<span style="color:#5a46a8">{pad(t, 25)}</span>'
                f'<span style="color:{color};font-weight:600">{pad(direction, 5)}</span>'
                f'<span style="color:#2a0b8a">{pad(hex_part, 36)}</span>'
                f'<span style="color:{msg_color};font-weight:{weight}">{html.escape(text)}</span>')

    def log(self, cat, direction, raw, text, error=False):
        entry = (now_text(), cat, direction, raw, text, error)
        self.logs.append(entry)
        if cat in self.shown_categories():
            bar = self.log_view.verticalScrollBar()
            at_bottom = bar.value() >= bar.maximum() - 4
            self.log_view.appendHtml(self.format_entry(entry))
            if at_bottom:
                bar.setValue(bar.maximum())

    def render_logs(self):
        """체크박스를 바꾸면 지나간 로그까지 다시 걸러서 보여 준다."""
        cats = self.shown_categories()
        lines = [self.format_entry(e) for e in self.logs if e[1] in cats]
        self.log_view.setUpdatesEnabled(False)
        self.log_view.clear()
        for line in lines:
            self.log_view.appendHtml(line)
        self.log_view.setUpdatesEnabled(True)
        self.log_view.moveCursor(QTextCursor.End)
        self.log_view.verticalScrollBar().setValue(self.log_view.verticalScrollBar().maximum())

    def clear_logs(self):
        self.logs.clear()
        self.log_view.clear()

    def closeEvent(self, event):
        if self.connected and self.sent_move != P.MOVE_STOP:
            self.link.write(P.build_packet(P.CMD_MANUAL_MOVE, [P.MOVE_STOP]))
        self.link.close()
        super().closeEvent(event)
