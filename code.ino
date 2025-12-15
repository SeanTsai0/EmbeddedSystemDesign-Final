// Human Following Robot - GRAFCET Structure
// Converted based on provided diagrams and example format
// Libraries required: AFMotor, NewPing, Servo

#include <AFMotor.h>
#include <NewPing.h>
#include <Servo.h>

// === 硬體腳位與常數定義 ===
#define RIGHT A2          // Right IR sensor
#define LEFT A3           // Left IR sensor
#define TRIGGER_PIN A1    // Ultrasonic Trigger
#define ECHO_PIN A0       // Ultrasonic Echo
#define MAX_DISTANCE 200  // Maximum ping distance

// === 模組宣告 (Function Prototypes) ===
void grafcet0(); void datapath0();
void grafcet1(); void datapath1();
void grafcet2(); void datapath2();
void grafcet3(); void datapath3();

// === 全域物件與變數 ===
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
AF_DCMotor Motor1(1, MOTOR12_1KHZ);
AF_DCMotor Motor2(2, MOTOR12_1KHZ);
AF_DCMotor Motor3(3, MOTOR34_1KHZ);
AF_DCMotor Motor4(4, MOTOR34_1KHZ);
Servo myservo;

// === 系統狀態資料結構 ===
typedef enum {
  ACTION_FORWARD,
  ACTION_LEFT,
  ACTION_RIGHT,
  ACTION_STOP
} RobotAction;

typedef struct {
  unsigned int distance_cm;
  bool ranging_completed;
} SensorData_US;

typedef struct {
  unsigned int right_val;
  unsigned int left_val;
  bool positioning_completed;
} SensorData_IR;

typedef struct {
  RobotAction currentAction;
  bool move_completed;
} SystemStatus;

// 實例化狀態變數
SensorData_US usData;
SensorData_IR irData;
SystemStatus sysStatus;

// === GRAFCET 狀態步進變數 ===
// Level 0: Main Sequencer
int X0 = 1, X1 = 0, X2 = 0, X3 = 0;
// Level 1: Ultrasonic Module
int X10 = 1, X11 = 0, X12 = 0;
// Level 2: IR Module
int X20 = 1, X21 = 0, X22 = 0;
// Level 3: Decision & Motion Module
int X30 = 1, X31 = 0, X32 = 0;

// === 初始化 ===
void setup() {
  Serial.begin(9600);
  
  // Servo 初始化 (原始程式中的掃描動作)
  myservo.attach(10);
  for(int pos = 90; pos <= 180; pos += 1) { myservo.write(pos); delay(15); }
  for(int pos = 180; pos >= 0; pos -= 1) { myservo.write(pos); delay(15); }
  for(int pos = 0; pos <= 90; pos += 1) { myservo.write(pos); delay(15); }

  pinMode(RIGHT, INPUT);
  pinMode(LEFT, INPUT);

  Serial.println("System Initialized");
}

// === 主控流程 ===
void loop() {
  datapath0();
  grafcet0();
}

// ==========================================
// === GRAFCET 0: 主流程控制 (對應 0.png) ===
// ==========================================
void grafcet0() {
  // Step 0: Init -> Step 1
  if (X0 == 1) {
    X0 = 0;
    X1 = 1;
    return;
  }
  
  // Step 1: Ultrasonic Module (呼叫 Grafcet 1)
  if (X1 == 1) {
    grafcet1(); 
    return;
  }
  
  // Step 2: IR Module (呼叫 Grafcet 2)
  if (X2 == 1) {
    grafcet2();
    return;
  }

  // Step 3: Direction Decision & Motion (呼叫 Grafcet 3)
  if (X3 == 1) {
    grafcet3();
    return;
  }
}

void datapath0() {
  if (X0 == 1) {
    // 可以在此重置所有子 Grafcet 的初始狀態
    X10 = 1; X11 = 0; X12 = 0;
    X20 = 1; X21 = 0; X22 = 0;
    X30 = 1; X31 = 0; X32 = 0;
  }
  if (X1 == 1) datapath1();
  if (X2 == 1) datapath2();
  if (X3 == 1) datapath3();
}

// =======================================================
// === GRAFCET 1: 超音波感測模組 (對應 1.png) ===
// =======================================================
void grafcet1() {
  // Step 10: Init -> 11
  if (X10 == 1) {
    X10 = 0;
    X11 = 1;
    return;
  }
  
  // Step 11: Sensing -> 12 (模擬 sensing_done)
  if (X11 == 1) {
    // 在 datapath1 中執行了 ping，這裡直接轉換
    X11 = 0;
    X12 = 1;
    return;
  }

  // Step 12: Calculate/Store -> 回到主流程
  if (X12 == 1) {
    X12 = 0;
    X10 = 1; // 重置子步進
    
    // 主流程轉換: ranging_done
    X1 = 0;
    X2 = 1; 
    return;
  }
}

void datapath1() {
  if (X11 == 1) {
    // HC-SR04_sensing_module()
    delay(50); // Wait 50ms between pings
    usData.distance_cm = sonar.ping_cm();
  }
  
  if (X12 == 1) {
    // calculate_distance() / Output
    Serial.print("distance: ");
    Serial.println(usData.distance_cm);
  }
}

// ===============================================
// === GRAFCET 2: 紅外線感測模組 (對應 2.png) ===
// ===============================================
void grafcet2() {
  // Step 20: Init -> 21
  if (X20 == 1) {
    X20 = 0;
    X21 = 1;
    return;
  }

  // Step 21: IR Sensing -> 22
  if (X21 == 1) {
    // 讀取完畢後轉換
    X21 = 0;
    X22 = 1;
    return;
  }

  // Step 22: Positioning/Store -> 回到主流程
  if (X22 == 1) {
    X22 = 0;
    X20 = 1; // 重置子步進
    
    // 主流程轉換: positioning_done
    X2 = 0;
    X3 = 1; 
    return;
  }
}

void datapath2() {
  if (X21 == 1) {
    // IR_sensing_module()
    irData.right_val = digitalRead(RIGHT);
    irData.left_val = digitalRead(LEFT);
  }

  if (X22 == 1) {
    // barrier_positioning() / Output
    Serial.print("RIGHT: "); Serial.println(irData.right_val);
    Serial.print("LEFT: "); Serial.println(irData.left_val);
  }
}

// ===============================================================
// === GRAFCET 3: 決策與動作模組 (對應 3.png) ===
// ===============================================================
void grafcet3() {
  // Step 30: Decision (判斷邏輯) -> 31
  if (X30 == 1) {
    X30 = 0;
    X31 = 1;
    return;
  }

  // Step 31: Action Execution (執行動作) -> 32
  if (X31 == 1) {
    // 假設動作執行一次循環即可視為完成 (action_done)
    X31 = 0;
    X32 = 1;
    return;
  }

  // Step 32: Moving Done -> 回到主流程開始 (Loop)
  if (X32 == 1) {
    X32 = 0;
    X30 = 1; // 重置子步進

    // 主流程轉換: moving_done -> 回到 Step 0 (實際上是回到 Step 1 的入口)
    X3 = 0;
    X0 = 1; 
    return;
  }
}

void datapath3() {
  // Step 30: 進行邏輯判斷 (對應圖 3 的分支條件)
  if (X30 == 1) {
    unsigned int d = usData.distance_cm;
    unsigned int r = irData.right_val;
    unsigned int l = irData.left_val;

    // 邏輯參照原始程式碼
    if (d > 1 && d < 15) {
      sysStatus.currentAction = ACTION_FORWARD;
    } 
    else if (r == 0 && l == 1) {
      sysStatus.currentAction = ACTION_LEFT;
    }
    else if (r == 1 && l == 0) {
      sysStatus.currentAction = ACTION_RIGHT;
    }
    else if (d > 15) {
      sysStatus.currentAction = ACTION_STOP;
    }
    else {
      // 預設狀況 (如果都不符合，例如 d=0 或 d=15 邊界狀況，依原始邏輯通常會停或維持)
      sysStatus.currentAction = ACTION_STOP; 
    }
  }

  // Step 31: 執行馬達控制 (對應圖 3 的 forward(), turn_left() 等)
  if (X31 == 1) {
    switch (sysStatus.currentAction) {
      case ACTION_FORWARD:
        moveForward();
        break;
      case ACTION_LEFT:
        turnLeft();
        break;
      case ACTION_RIGHT:
        turnRight();
        break;
      case ACTION_STOP:
        stopRobot();
        break;
    }
  }
}

// === 動作子功能 (Action Primitives) ===

void moveForward() {
  Motor1.setSpeed(130); Motor1.run(FORWARD);
  Motor2.setSpeed(130); Motor2.run(FORWARD);
  Motor3.setSpeed(130); Motor3.run(FORWARD);
  Motor4.setSpeed(130); Motor4.run(FORWARD);
  Serial.println("Action: Forward");
}

void turnLeft() {
  Motor1.setSpeed(150); Motor1.run(FORWARD);
  Motor2.setSpeed(150); Motor2.run(FORWARD);
  Motor3.setSpeed(150); Motor3.run(BACKWARD);
  Motor4.setSpeed(150); Motor4.run(BACKWARD);
  delay(150); // 原始程式中的 delay 放在動作裡
  Serial.println("Action: Turn Left");
}

void turnRight() {
  Motor1.setSpeed(150); Motor1.run(BACKWARD);
  Motor2.setSpeed(150); Motor2.run(BACKWARD);
  Motor3.setSpeed(150); Motor3.run(FORWARD);
  Motor4.setSpeed(150); Motor4.run(FORWARD);
  delay(150); // 原始程式中的 delay
  Serial.println("Action: Turn Right");
}

void stopRobot() {
  Motor1.setSpeed(0); Motor1.run(RELEASE);
  Motor2.setSpeed(0); Motor2.run(RELEASE);
  Motor3.setSpeed(0); Motor3.run(RELEASE);
  Motor4.setSpeed(0); Motor4.run(RELEASE);
  Serial.println("Action: Stop");
}
