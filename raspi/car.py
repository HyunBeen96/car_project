import asyncio
from bleak import BleakClient
import smbus2 as smbus
import time
import cv2
import threading
import RPi.GPIO as GPIO

# GPIO 설정
SW_PIN = 17  # 조이스틱 SW 핀
GPIO.setmode(GPIO.BCM)
GPIO.setup(SW_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

# HM-10 설정
hm10_mac = "18:93:D7:47:B2:38"
char_uuid = "0000ffe1-0000-1000-8000-00805f9b34fb"

# I2C 설정
bus = smbus.SMBus(1)
pcf_addr = 0x48

def is_button_pressed():
    return GPIO.input(SW_PIN) == GPIO.LOW  # 눌리면 LOW

def read_adc_stable(channel):
    """PCF8591 ADC 읽기 (0-255 범위)"""
    bus.write_byte(pcf_addr, 0x40 | channel)
    time.sleep(0.003)
    bus.read_byte(pcf_addr)  # 더미 읽기
    time.sleep(0.003)
    return bus.read_byte(pcf_addr)

def scale_adc_to_stm32(value):
    """8비트 ADC 값을 STM32의 12비트 범위로 스케일링"""
    # 0-255 -> 0-4095
    return int(value * 4095 / 255)

def show_stream():
    """RTMP 스트림 표시 (별도 스레드)"""
    cap = cv2.VideoCapture("rtmp://192.168.0.94:1935/live/stream")
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)  # 버퍼 최소화로 지연 감소
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("⚠️ 스트림 연결 끊김, 재연결 시도...")
            time.sleep(1)
            continue
            
        # 화면 크기 조정 (옵션)
        frame = cv2.resize(frame, (640, 480))
        cv2.imshow("🚗 Live Stream", frame)
        
        if cv2.waitKey(1) == ord('q'):
            break
            
    cap.release()
    cv2.destroyAllWindows()

async def send_mode_change(client, mode):
    """모드 변경 명령 전송"""
    mode_cmd = b'2' if mode == 1 else b'1'  # '2'=자동, '1'=수동
    await client.write_gatt_char(char_uuid, mode_cmd)
    await asyncio.sleep(0.1)  # 명령 처리 대기

async def send_joystick_data(client, y, x):
    """조이스틱 데이터 전송"""
    data = f"{y},{x}\n"
    await client.write_gatt_char(char_uuid, data.encode())

# 메인 BLE 통신 루프
async def main():
    print("🔄 HM-10 연결 시도 중...")
    
    try:
        async with BleakClient(hm10_mac) as client:
            print("✅ HM-10 연결 성공!")
            
            mode = 1  # 0: 수동, 1: 자동 (기본값 자동)
            last_pressed = False
            last_mode_change = 0
            
            # 초기 모드 설정
            await send_mode_change(client, mode)
            print(f"📡 초기 모드: {'자동' if mode else '수동'}")
            
            while True:
                # 조이스틱 ADC 읽기 및 스케일링
                y_raw = read_adc_stable(0)
                x_raw = read_adc_stable(1)
                
                # STM32 12비트 범위로 변환
                y = scale_adc_to_stm32(y_raw)
                x = scale_adc_to_stm32(x_raw)
                
                # 수동 모드일 때만 조이스틱 데이터 전송
                if mode == 0:
                    await send_joystick_data(client, y, x)
                    print(f"🕹️ 조이스틱: Y={y:4d}, X={x:4d} (raw: {y_raw:3d}, {x_raw:3d})")
                
                # SW 버튼으로 모드 전환 (디바운싱 포함)
                pressed = is_button_pressed()
                current_time = time.time()
                
                if pressed and not last_pressed and (current_time - last_mode_change > 0.5):
                    mode = 1 - mode  # 토글
                    await send_mode_change(client, mode)
                    print(f"🔄 모드 전환: {'자동' if mode else '수동'}")
                    last_mode_change = current_time
                
                last_pressed = pressed
                
                # 주기 조정 (수동: 빠르게, 자동: 느리게)
                if mode == 0:
                    await asyncio.sleep(0.05)  # 20Hz (수동 모드)
                else:
                    await asyncio.sleep(0.2)   # 5Hz (자동 모드 - 대기만)
                    
    except Exception as e:
        print(f"❌ 에러 발생: {e}")
        print("재연결을 시도하려면 프로그램을 다시 실행하세요.")
    
    finally:
        GPIO.cleanup()

if __name__ == "__main__":
    # 영상 스트리밍을 별도 스레드로 실행
    stream_thread = threading.Thread(target=show_stream, daemon=True)
    stream_thread.start()
    
    # BLE 통신 메인 루프 실행
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n👋 프로그램 종료")
        GPIO.cleanup()