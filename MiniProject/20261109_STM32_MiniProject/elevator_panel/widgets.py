"""직접 그리는 작은 위젯들: 하늘 배경, 카드, 상태 점, 제목, 승강로 그림, 모드 배지."""
import math

from PySide6.QtCore import Qt, QPointF, QRectF, QTimer
from PySide6.QtGui import QColor, QFont, QLinearGradient, QPainter, QPainterPath, QPen
from PySide6.QtWidgets import QFrame, QVBoxLayout, QWidget

# 시안과 같은 색
SKY_TOP = QColor("#d9c6f6")
SKY_BOTTOM = QColor("#f8cbf2")
INK = QColor("#2a0b8a")
ROYAL = QColor("#1a45bd")
CYAN = QColor("#17d3e8")
CYAN_SOFT = QColor("#a9eef7")
WHITE = QColor("#ffffff")
MUTED = QColor("#ece3fa")
LED_ON = QColor("#ff2e55")
OK_GREEN = QColor("#10a37f")

DISPLAY_FONT = "Black Han Sans"
BODY_FONT = "IBM Plex Sans KR"
MONO_FONT = "JetBrains Mono"


class SkyBackground(QWidget):
    """라벤더→핑크 하늘, 오른쪽 위 파란 덩어리, 하늘색 테두리 구름."""

    PUFFS = [(0, 0, 1), (0.9, -0.35, 1.15), (1.95, -0.1, 0.95),
             (2.7, 0.25, 0.75), (-0.75, 0.3, 0.75), (1.1, 0.45, 1)]

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        w, h = self.width(), self.height()

        grad = QLinearGradient(0, 0, 0, h)
        grad.setColorAt(0, SKY_TOP)
        grad.setColorAt(1, SKY_BOTTOM)
        p.fillRect(self.rect(), grad)

        blob = QPainterPath()
        blob.moveTo(w * 0.62, 0)
        blob.lineTo(w, 0)
        blob.lineTo(w, h * 0.3)
        blob.cubicTo(w * 0.93, h * 0.3, w * 0.9, h * 0.12, w * 0.8, h * 0.15)
        blob.cubicTo(w * 0.74, h * 0.05, w * 0.67, h * 0.09, w * 0.66, h * 0.07)
        blob.closeSubpath()
        p.fillPath(blob, ROYAL)

        s = max(40.0, min(w, h) * 0.09)
        self._cloud(p, w * 0.04, h * 0.02, s * 0.9, CYAN)
        self._cloud(p, -s * 0.4, h * 0.82, s * 1.5, CYAN_SOFT)
        self._cloud(p, w * 0.42, h * 1.02, s * 1.3, CYAN)
        self._cloud(p, w * 0.88, h * 0.86, s * 1.4, CYAN_SOFT)

    def _cloud(self, p, x, y, s, stroke):
        path = QPainterPath()
        path.setFillRule(Qt.WindingFill)
        for dx, dy, r in self.PUFFS:
            path.addEllipse(QPointF(x + dx * s, y + dy * s), r * s, r * s)
        path = path.simplified()          # 동그라미들을 하나의 구름 모양으로 합침
        p.setPen(QPen(stroke, 4))
        p.setBrush(WHITE)
        p.drawPath(path)


class Card(QWidget):
    """흰 테두리 + 남색 테두리 두 겹의 카드. self.body에 내용을 넣는다.

    그림자와 '연결 전 흐리게' 효과는 QGraphicsEffect 대신 직접 그린다.
    (효과를 겹쳐 쓰면 안쪽 위젯이 바뀌어도 화면이 바로 다시 그려지지 않는 문제가 있었음)
    """
    SHADOW_X, SHADOW_Y = 8, 10

    def __init__(self, parent=None):
        super().__init__(parent)
        wrap = QVBoxLayout(self)
        wrap.setContentsMargins(0, 0, self.SHADOW_X, self.SHADOW_Y)   # 그림자 자리
        self.frame = QFrame(self)
        self.frame.setObjectName("cardOuter")
        wrap.addWidget(self.frame)
        outer = QVBoxLayout(self.frame)
        outer.setContentsMargins(5, 5, 5, 5)
        self.inner = QFrame(self.frame)
        self.inner.setObjectName("card")
        outer.addWidget(self.inner)
        self.body = QVBoxLayout(self.inner)
        self.body.setContentsMargins(18, 16, 18, 16)
        self.body.setSpacing(12)

        self._veil = _Veil(self.frame)      # 연결 전에 카드 위를 덮는 반투명 막
        self._veil.hide()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._veil.setGeometry(self.frame.rect())

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.frame.geometry()).translated(self.SHADOW_X, self.SHADOW_Y)
        p.setPen(Qt.NoPen)
        p.setBrush(QColor(42, 11, 138, 45))
        p.drawRoundedRect(r, 22, 22)

    def set_active(self, active: bool):
        """연결 전에는 흐리게 보이고 누를 수 없게."""
        self.inner.setEnabled(active)
        self._veil.setVisible(not active)
        if not active:
            self._veil.setGeometry(self.frame.rect())
            self._veil.raise_()


class _Veil(QWidget):
    """카드를 흐리게 덮고 마우스 입력을 막는 막."""

    def __init__(self, parent):
        super().__init__(parent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        p.setPen(Qt.NoPen)
        p.setBrush(QColor(255, 255, 255, 140))
        p.drawRoundedRect(QRectF(self.rect()), 22, 22)

    def mousePressEvent(self, event):
        event.accept()


class StatusDot(QWidget):
    """연결 상태 점. 포트를 찾는 중에는 깜빡인다."""
    COLORS = {"searching": CYAN_SOFT, "found": CYAN, "connected": OK_GREEN}

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFixedSize(26, 26)
        self.state = "searching"
        self._phase = 0.0
        self._timer = QTimer(self, interval=60, timeout=self._tick)
        self._timer.start()

    def set_state(self, state):
        self.state = state
        self.update()

    def _tick(self):
        if self.state == "searching":
            self._phase = (self._phase + 0.12) % (2 * math.pi)
            self.update()

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        r = 8.0
        if self.state == "searching":
            r += 2.0 * (0.5 + 0.5 * math.sin(self._phase))
        p.setPen(QPen(INK, 3))
        p.setBrush(self.COLORS[self.state])
        p.drawEllipse(QPointF(13, 13), r, r)


class OutlinedTitle(QWidget):
    """'비콘' 글자처럼 남색 글자 + 흰 테두리 + 남색 그림자로 그린 제목."""

    def __init__(self, text, size=34, parent=None):
        super().__init__(parent)
        self.text = text
        self.font = QFont(DISPLAY_FONT, size)
        self.font.setLetterSpacing(QFont.AbsoluteSpacing, 1)
        path = QPainterPath()
        path.addText(0, 0, self.font, text)
        self._box = path.boundingRect()
        self.setFixedSize(int(self._box.width()) + 16, int(self._box.height()) + 14)

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        path = QPainterPath()
        path.addText(4 - self._box.left(), 4 - self._box.top(), self.font, self.text)
        p.fillPath(path.translated(5, 5), INK)               # 그림자
        p.strokePath(path, QPen(WHITE, 7, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
        p.fillPath(path, INK)


class ShaftView(QWidget):
    """엘리베이터 승강로 그림. 카 위치는 천장까지 거리(mm)로 그린다.

    set_status(pos_mm, current, target, moving_dir, emergency)
      pos_mm     : 천장까지 거리(mm), None이면 현재 층 위치에 점선으로 그림
      moving_dir : +1 상승, -1 하강, 0 정지
    """
    TOP_MM, BOTTOM_MM = 18.0, 105.0                   # 그림의 위/아래 끝이 나타내는 거리
    FLOORS = {3: 28, 2: 58, 1: 95}                     # 실측 층 위치 (mm)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(170, 300)
        self.pos_mm = None
        self.current = 1
        self.target = 1
        self.direction = 0
        self.emergency = False
        self._blink = True
        self._timer = QTimer(self, interval=500, timeout=self._toggle)
        self._timer.start()

    def _toggle(self):
        self._blink = not self._blink
        if self.emergency:
            self.update()

    def set_status(self, pos_mm, current, target, direction, emergency):
        self.pos_mm, self.current, self.target = pos_mm, current, target
        self.direction, self.emergency = direction, emergency
        self.update()

    def _y(self, mm, top, height):
        mm = min(self.BOTTOM_MM, max(self.TOP_MM, mm))
        return top + (mm - self.TOP_MM) / (self.BOTTOM_MM - self.TOP_MM) * height

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        w, h = self.width(), self.height()
        label_w = 46
        shaft = QRectF(label_w + 6, 6, w - label_w - 14, h - 12)

        # 승강로
        p.setPen(QPen(INK, 3))
        p.setBrush(QColor("#f6f1ff"))
        p.drawRoundedRect(shaft, 14, 14)
        p.setPen(QPen(QColor(42, 11, 138, 60), 2, Qt.DashLine))
        cx = shaft.center().x()
        p.drawLine(QPointF(cx, shaft.top() + 10), QPointF(cx, shaft.bottom() - 10))   # 와이어

        inner_top, inner_h = shaft.top() + 10, shaft.height() - 20
        floor_font = QFont(DISPLAY_FONT, 15)
        for floor, mm in self.FLOORS.items():
            y = self._y(mm, inner_top, inner_h)
            is_target = self.direction != 0 and not self.emergency and floor == self.target
            color = CYAN if is_target else QColor(42, 11, 138, 110)
            p.setPen(QPen(color, 4 if is_target else 2))
            p.drawLine(QPointF(shaft.left() + 4, y), QPointF(shaft.right() - 4, y))
            p.setFont(floor_font)
            p.setPen(INK if floor == self.current or is_target else QColor("#8f80c9"))
            p.drawText(QRectF(0, y - 14, label_w, 28), Qt.AlignRight | Qt.AlignVCenter, f"{floor}F")

        # 카
        mm = self.pos_mm if self.pos_mm is not None else self.FLOORS.get(self.current, 95)
        cy = self._y(mm, inner_top, inner_h)
        car_w, car_h = shaft.width() * 0.62, max(42.0, inner_h * 0.17)
        car = QRectF(cx - car_w / 2, cy - car_h, car_w, car_h)       # 카 바닥이 층 선에 닿게
        fill = LED_ON if (self.emergency and self._blink) else (QColor("#ff8aa1") if self.emergency else ROYAL)
        pen = QPen(INK, 3, Qt.DashLine if self.pos_mm is None else Qt.SolidLine)
        p.setPen(pen)
        p.setBrush(fill)
        p.drawRoundedRect(car, 10, 10)
        # 문
        p.setPen(QPen(WHITE, 2))
        p.drawLine(QPointF(cx, car.top() + 8), QPointF(cx, car.bottom() - 6))
        # 방향 화살표
        if self.direction:
            a = QPainterPath()
            s = min(car_w, car_h) * 0.22
            tip = car.top() + 8 if self.direction > 0 else car.bottom() - 8
            base = tip + s * 1.2 if self.direction > 0 else tip - s * 1.2
            ax = car.right() - s - 8
            a.moveTo(ax, tip)
            a.lineTo(ax - s, base)
            a.lineTo(ax + s, base)
            a.closeSubpath()
            p.setPen(Qt.NoPen)
            p.fillPath(a, WHITE)


class ModeBadge(QWidget):
    """NORMAL / EMERGENCY 배지. EMERGENCY일 때는 빨갛게 깜빡인다."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.text = "연결 전"
        self.kind = "idle"            # idle / normal / emergency
        self._blink = True
        self.font = QFont(DISPLAY_FONT, 15)
        self.setFixedHeight(40)
        self.setMinimumWidth(150)
        self._timer = QTimer(self, interval=500, timeout=self._toggle)
        self._timer.start()

    def _toggle(self):
        self._blink = not self._blink
        if self.kind == "emergency":
            self.update()

    def set_mode(self, kind, text):
        if (kind, text) != (self.kind, self.text):
            self.kind, self.text = kind, text
            self.update()

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(1.5, 1.5, self.width() - 3, self.height() - 3)
        if self.kind == "emergency":
            bg, fg = (LED_ON, WHITE) if self._blink else (WHITE, LED_ON)
        elif self.kind == "normal":
            bg, fg = CYAN_SOFT, INK
        else:
            bg, fg = MUTED, QColor("#8f80c9")
        p.setPen(QPen(INK, 3))
        p.setBrush(bg)
        p.drawRoundedRect(r, r.height() / 2, r.height() / 2)
        p.setFont(self.font)
        p.setPen(fg)
        p.drawText(r, Qt.AlignCenter, self.text)
