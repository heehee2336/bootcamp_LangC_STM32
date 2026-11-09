"""엘리베이터 컨트롤 패널 실행 파일

    python main.py          실제 보드(ST-Link)와 연결
    python main.py --sim    보드 없이 가상 엘리베이터로 실행
"""
import sys
from pathlib import Path

from PySide6.QtGui import QFont, QFontDatabase
from PySide6.QtWidgets import QApplication

from main_window import MainWindow

HERE = Path(__file__).resolve().parent


def load_fonts():
    """fonts 폴더의 글꼴을 이 프로그램 안에서만 쓰도록 등록 (PC에 설치할 필요 없음)."""
    for ttf in sorted((HERE / "fonts").glob("*.ttf")):
        if QFontDatabase.addApplicationFont(str(ttf)) < 0:
            print(f"글꼴을 불러오지 못했어요: {ttf.name}")


def main():
    app = QApplication(sys.argv)
    load_fonts()
    app.setFont(QFont("IBM Plex Sans KR", 10))
    app.setStyleSheet((HERE / "style.qss").read_text(encoding="utf-8"))
    window = MainWindow(simulate="--sim" in sys.argv)
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
