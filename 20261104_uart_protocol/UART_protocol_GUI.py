import sys
import serial
import serial.tools.list_ports
import time
from PyQt5.QtWidgets import (QApplication, QWidget, QVBoxLayout, QHBoxLayout, 
                             QPushButton, QLabel, QGroupBox, QGridLayout, QMessageBox)
from PyQt5.QtCore import QThread, pyqtSignal, Qt
from PyQt5.QtGui import QFont

# --- 통신 수신용 백그라운드 스레드 ---
class SerialReceiverThread(QThread):
    packet_received = pyqtSignal(int, list) # 수신 완료 시 메인 창으로 (CMD, DATA리스트) 전달
    error_occurred = pyqtSignal(str)

    def __init__(self, serial_port):
        super().__init__()
        self.serial = serial_port
        self.running = True

    def run(self):
        state = 'WAIT_SOF'
        cmd = 0
        data_len = 0
        data = []

        while self.running and self.serial.is_open:
            try:
                if self.serial.in_waiting > 0:
                    byte_read = self.serial.read(1)[0]

                    if state == 'WAIT_SOF':
                        if byte_read == 0xAA:
                            state = 'READ_CMD'
                    
                    elif state == 'READ_CMD':
                        cmd = byte_read
                        state = 'READ_LEN'
                    
                    elif state == 'READ_LEN':
                        data_len = byte_read
                        data = []
                        if data_len > 0:
                            state = 'READ_DATA'
                        else:
                            state = 'READ_CHECKSUM'
                    
                    elif state == 'READ_DATA':
                        data.append(byte_read)
                        if len(data) == data_len:
                            state = 'READ_CHECKSUM'
                    
                    elif state == 'READ_CHECKSUM':
                        checksum = byte_read

                        print(f"CMD: {hex(cmd)}, DATA: {[hex(x) for x in data]}, Calc_CS: {hex(calc_checksum)}, Rx_CS: {hex(rx_checksum)}")
                        # Checksum 검증: (CMD + LEN + sum(DATA)) & 0xFF
                        calc_checksum = (cmd + data_len + sum(data)) & 0xFF
                        
                        if checksum == calc_checksum:
                            self.packet_received.emit(cmd, data)
                        else:
                            print(f"[RX Error] Checksum mismatch. Calc:{calc_checksum}, Rx:{checksum}")
                            print("Checksum Error: 계산값과 수신값이 다릅니다!")
                        state = 'WAIT_SOF' # 다음 패킷 대기
                        
            except Exception as e:
                self.error_occurred.emit(str(e))
                break

    def stop(self):
        self.running = False


# --- 메인 GUI 클래스 ---
class STM32App(QWidget):
    def __init__(self):
        super().__init__()
        self.serial = serial.Serial()
        self.serial.dtr = False  # 추가: DTR 신호 비활성화
        self.serial.rts = False  # 추가: RTS 신호 비활성화
        self.serial.baudrate = 115200
        self.serial.timeout = 0.5
        self.rx_thread = None
        self.led_status = [0, 0, 0, 0] # 4개 LED의 현재 상태 (0: OFF, 1: ON)

        self.initUI()

    def initUI(self):
        self.setWindowTitle('STM32 Nucleo-F411RE Controller')
        self.resize(400, 350)
        self.setStyleSheet("""
            QWidget { background-color: #f4f5f7; font-family: 'Segoe UI', sans-serif; }
            QGroupBox { font-weight: bold; border: 1px solid #ccc; border-radius: 8px; margin-top: 10px; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #333; }
            QPushButton { background-color: #0078D7; color: white; border: none; border-radius: 5px; padding: 8px 12px; font-weight: bold; }
            QPushButton:hover { background-color: #005A9E; }
            QPushButton:disabled { background-color: #a0c4e3; }
            QPushButton.led-on { background-color: #28a745; }
            QPushButton.led-off { background-color: #dc3545; }
            QLabel { color: #333; font-size: 14px; }
        """)

        main_layout = QVBoxLayout()

        # 1. 연결 설정 섹션
        conn_group = QGroupBox("Connection")
        conn_layout = QHBoxLayout()
        self.lbl_status = QLabel("Status: Disconnected")
        self.btn_connect = QPushButton("Auto Detect & Connect")
        self.btn_connect.clicked.connect(self.auto_connect)
        conn_layout.addWidget(self.lbl_status)
        conn_layout.addStretch()
        conn_layout.addWidget(self.btn_connect)
        conn_group.setLayout(conn_layout)
        main_layout.addWidget(conn_group)

        # 2. LED 제어 섹션
        led_group = QGroupBox("LED Control")
        led_layout = QGridLayout()
        self.btn_leds = []
        for i in range(4):
            btn = QPushButton(f"LED {i} OFF")
            btn.setProperty('class', 'led-off')
            btn.setEnabled(False) # 연결 전에는 비활성화
            btn.clicked.connect(lambda checked, idx=i: self.toggle_led(idx))
            self.btn_leds.append(btn)
            led_layout.addWidget(btn, 0, i)
        led_group.setLayout(led_layout)
        main_layout.addWidget(led_group)

        # 3. 센서 데이터 섹션
        sensor_group = QGroupBox("Sensors")
        sensor_layout = QGridLayout()
        
        self.btn_temp_hum = QPushButton("Request Temp/Humid")
        self.btn_temp_hum.setEnabled(False)
        self.btn_temp_hum.clicked.connect(lambda: self.send_packet(0x20, []))
        self.lbl_temp_hum = QLabel("Temp: -- °C | Humid: -- %")
        self.lbl_temp_hum.setAlignment(Qt.AlignCenter)

        self.btn_distance = QPushButton("Request Distance")
        self.btn_distance.setEnabled(False)
        self.btn_distance.clicked.connect(lambda: self.send_packet(0x30, []))
        self.lbl_distance = QLabel("Distance: -- mm")
        self.lbl_distance.setAlignment(Qt.AlignCenter)

        sensor_layout.addWidget(self.btn_temp_hum, 0, 0)
        sensor_layout.addWidget(self.lbl_temp_hum, 0, 1)
        sensor_layout.addWidget(self.btn_distance, 1, 0)
        sensor_layout.addWidget(self.lbl_distance, 1, 1)
        sensor_group.setLayout(sensor_layout)
        main_layout.addWidget(sensor_group)

        self.setLayout(main_layout)

    def auto_connect(self):
        if self.serial.is_open:
            self.disconnect_serial()
            return

        ports = serial.tools.list_ports.comports()
        target_port = None
        
        # 포트 자동 감지 로직 (STLink 또는 Nucleo라는 키워드 검색)
        for port in ports:
            if "STLink" in port.description or "STM32" in port.description or "Serial" in port.description:
                target_port = port.device
                break
        
        # 이름으로 못 찾았다면 첫 번째 사용 가능한 포트 시도
        if not target_port and ports:
            target_port = ports[0].device

        if target_port:
            try:
                self.serial.port = target_port
                self.serial.open()
                self.lbl_status.setText(f"Status: Connected ({target_port})")
                self.btn_connect.setText("Disconnect")
                self.btn_connect.setStyleSheet("background-color: #dc3545;")
                self.enable_controls(True)
                
                # 수신 스레드 시작
                self.rx_thread = SerialReceiverThread(self.serial)
                self.rx_thread.packet_received.connect(self.handle_packet)
                self.rx_thread.error_occurred.connect(self.handle_rx_error)
                self.rx_thread.start()

                # 연결 직후 LED 전체 상태 한번 요청
                self.send_packet(0x14, [])

            except Exception as e:
                QMessageBox.critical(self, "Connection Error", f"Failed to open {target_port}\n{str(e)}")
        else:
            QMessageBox.warning(self, "Not Found", "Could not detect any STM32 board.")

    def disconnect_serial(self):
        if self.rx_thread:
            self.rx_thread.stop()
            self.rx_thread.wait()
        if self.serial.is_open:
            self.serial.close()
        
        self.lbl_status.setText("Status: Disconnected")
        self.btn_connect.setText("Auto Detect & Connect")
        self.btn_connect.setStyleSheet("") # 스타일 초기화
        self.enable_controls(False)

    def enable_controls(self, enable):
        for btn in self.btn_leds:
            btn.setEnabled(enable)
        self.btn_temp_hum.setEnabled(enable)
        self.btn_distance.setEnabled(enable)

    def toggle_led(self, led_index):
        # 현재 상태를 뒤집어서 전송 (0->1, 1->0)
        target_state = 1 if self.led_status[led_index] == 0 else 0
        cmd = 0x10 + led_index # 0x10, 0x11, 0x12, 0x13
        self.send_packet(cmd, [target_state])
        
        # 낙관적 UI 업데이트 (응답을 받기 전에 일단 변경, 실제로는 응답을 받고 변경하는 것이 더 안전함)
        # 여기서는 패킷 전송만 하고 실제 버튼 색상은 0x90(또는 0x94) 응답을 받았을 때 업데이트하도록 설계

    def send_packet(self, cmd, data_list):
        if not self.serial.is_open:
            return
        
        sof = 0xAA
        data_len = 1 + len(data_list)
        checksum = (cmd + data_len + sum(data_list)) & 0xFF
        
        packet = bytearray([sof, data_len, cmd] + data_list + [checksum])
        # 디버깅용 출력
        print(f"TX Packet: {[hex(x) for x in packet]}")
        self.serial.write(packet)
        self.serial.flush()
        time.sleep(0.005)   

    def update_led_ui(self):
        """self.led_status 배열 상태에 따라 GUI 버튼 색상과 텍스트를 갱신하는 함수"""
        for i in range(4):
            btn = self.btn_leds[i]
            if self.led_status[i] == 1:
                btn.setText(f"LED {i} ON")
                btn.setStyleSheet("background-color: #28a745;")  # 초록색 (ON)
            else:
                btn.setText(f"LED {i} OFF")
                btn.setStyleSheet("background-color: #dc3545;")  # 빨간색 (OFF)

    def handle_packet(self, cmd, data):
        # STM32로부터 정상적인 패킷을 수신했을 때 실행되는 로직
        
        # 1. 개별 LED 제어 응답 (CMD: 0x90 ~ 0x93)
        if 0x90 <= cmd <= 0x93:
            if len(data) >= 1 and data[0] == 0x00:  # RSP_OK (성공 응답)
                led_idx = cmd - 0x90
                # 성공 응답을 받았으므로 해당 LED 상태를 반전 (0 -> 1, 1 -> 0)
                self.led_status[led_idx] = 1 if self.led_status[led_idx] == 0 else 0
                self.update_led_ui()

        # 2. 전체 LED 상태 응답 (CMD: 0x94 - 포트 최초 연결 시)
        elif cmd == 0x94 and len(data) == 4:
            self.led_status = list(data)
            self.update_led_ui()

        # 3. 온습도 응답 (CMD: 0xA0)
        elif cmd == 0xA0 and len(data) == 4:
            temp = data[0] + (data[1] * 0.1)
            humid = data[2] + (data[3] * 0.1)
            self.lbl_temp_hum.setText(f"Temp: {temp:.1f} °C | Humid: {humid:.1f} %")

        # 4. 거리 응답 (CMD: 0xB0)
        elif cmd == 0xB0 and len(data) == 2:
            distance = (data[0] << 8) | data[1]
            self.lbl_distance.setText(f"Distance: {distance} mm")

    def handle_rx_error(self, error_msg):
        print(f"RX Error: {error_msg}")
        self.disconnect_serial()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = STM32App()
    ex.show()
    sys.exit(app.exec_())