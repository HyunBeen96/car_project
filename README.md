# 🚗 Autonomous Car Project

STM32F411과 Raspberry Pi 4를 이용한 자율주행 RC카 프로젝트

## 📋 프로젝트 개요

- **목적**: 초음파 센서를 이용한 장애물 회피 자율주행 구현
- **모드**: 수동 조작 모드 / 자율 주행 모드 전환 가능
- **통신**: Bluetooth (HM-10) 통한 무선 제어

## 🛠 하드웨어 구성

### MCU 및 보드
- **메인 컨트롤러**: STM32F411 Discovery Board
- **서브 컨트롤러**: Raspberry Pi 4

### 센서 및 모듈
- **초음파 센서**: HC-SR04 × 3개 (전방, 좌측, 우측)
- **모터 드라이버**: L298N
- **블루투스 모듈**: HM-10
- **조이스틱**: 아날로그 2축 + 스위치
- **ADC**: PCF8591 (I2C 8-bit ADC)

## 📁 프로젝트 구조

```
autonomous_car_project/
├── stm32_411/          # STM32 펌웨어
│   ├── main.c/h        # 메인 프로그램
│   ├── motorset.c/h    # 모터 제어
│   ├── ultrasonic.c/h  # 초음파 센서
│   ├── freertos.c      # RTOS 태스크
│   ├── usart.c/h       # UART 통신
│   ├── tim.c/h         # 타이머 설정
│   ├── gpio.c/h        # GPIO 설정
│   ├── delay.c/h       # 딜레이 함수
│   └── raspi.c/h       # 라즈베리파이 통신
│
└── raspi/              # Raspberry Pi 코드
    ├── car.py          # 메인 제어 프로그램
    ├── BT.py           # 블루투스 테스트
    └── button.py       # 버튼 테스트
```

## 🔧 주요 기능

### 1. 자율주행 모드
- 3개의 초음파 센서로 전방 및 좌우 장애물 감지
- 장애물 회피 알고리즘 구현
- 벽 추종 주행 가능

### 2. 수동 조작 모드
- 조이스틱을 통한 전진/후진/좌회전/우회전
- 속도 제어 (PWM 방식)
- 부드러운 가감속 구현

### 3. 모드 전환
- 조이스틱 버튼으로 수동/자율 모드 전환
- 실시간 모드 상태 표시

## 📡 통신 프로토콜

### Bluetooth (UART)
- **Baud Rate**: 9600 bps (HM-10), 115200 bps (Debug)
- **데이터 형식**: 
  - 조이스틱: `"Y값,X값\n"` (예: "2048,2048\n")
  - 모드 전환: '1' (수동), '2' (자율)

### I2C
- **PCF8591 ADC**: 주소 0x48
- **채널**: CH0 (Y축), CH1 (X축)

## ⚙️ 핀 연결

### STM32F411 핀맵

#### 모터 드라이버 (L298N)
- PB13: Motor IN1
- PB14: Motor IN2
- PB15: Motor IN3
- PB1: Motor IN4
- PC6: PWM Left (TIM3_CH1)
- PC7: PWM Right (TIM3_CH2)

#### 초음파 센서
- PA5: Trig Left
- PA6: Trig Front
- PA7: Trig Right
- PB6: Echo Left (TIM4_CH1)
- PB7: Echo Right (TIM4_CH2)
- PB8: Echo Front (TIM4_CH3)

#### UART
- PA9/PA10: USART1 (Bluetooth)
- PA2/PA3: USART2 (Debug)

### Raspberry Pi 4 핀맵
- GPIO 17: 조이스틱 SW
- I2C1 (GPIO 2/3): PCF8591 연결
- UART (GPIO 14/15): HM-10 연결

## 🚀 설치 및 실행

### STM32 펌웨어
1. STM32CubeIDE에서 프로젝트 열기
2. 빌드 후 보드에 업로드

### Raspberry Pi
```bash
# 필요 패키지 설치
pip install bleak asyncio smbus2 opencv-python RPi.GPIO

# 실행
python3 raspi/car.py
```

## 📊 FreeRTOS 태스크

| 태스크 | 우선순위 | 주기 | 기능 |
|-------|---------|------|------|
| JoystickRecv | Low | 20ms | 블루투스 데이터 수신 |
| ManualDrv | Low | 20ms | 수동 모드 모터 제어 |
| AutoDrv_Sensor | AboveNormal | 90ms | 초음파 센서 측정 |
| AutoDrv_Control | AboveNormal | 100ms | 자율주행 제어 |

## 🔍 주요 파라미터

### 모터 제어
- PWM 주파수: 109 kHz
- PWM 범위: 0-1000 (0-100%)
- 최소 속도: 650
- 최대 속도: 1000

### 자율주행
- 최소 안전거리: 20cm
- 최대 감지거리: 80cm
- 센서 측정 주기: 90ms

## 📝 라이센스

MIT License

## 👥 기여자

- [Your Name]

## 📅 개발 기간

2025.01 - 2025.01

---

*Think about it step-by-step.*
