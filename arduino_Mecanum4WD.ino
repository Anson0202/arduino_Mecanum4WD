#include <Wire.h>
#include "PS2X_lib.h"
#include "QGPMaker_MotorShield.h"
#include "QGPMaker_Encoder.h"

QGPMaker_MotorShield AFMS = QGPMaker_MotorShield();
PS2X ps2x;

QGPMaker_DCMotor *DCMotor_2 = AFMS.getMotor(2);
QGPMaker_DCMotor *DCMotor_4 = AFMS.getMotor(4);
QGPMaker_DCMotor *DCMotor_1 = AFMS.getMotor(1);
QGPMaker_DCMotor *DCMotor_3 = AFMS.getMotor(3);

QGPMaker_Encoder Encoder1(1);
QGPMaker_Encoder Encoder2(2);
QGPMaker_Encoder Encoder3(3);
QGPMaker_Encoder Encoder4(4);

void forward() {
  DCMotor_1->setSpeed(200);
  DCMotor_1->run(FORWARD);
  DCMotor_2->setSpeed(200);
  DCMotor_2->run(FORWARD);
  DCMotor_3->setSpeed(200);
  DCMotor_3->run(FORWARD);
  DCMotor_4->setSpeed(200);
  DCMotor_4->run(FORWARD);
}

void turnLeft() {
  DCMotor_1->setSpeed(200);
  DCMotor_1->run(BACKWARD);
  DCMotor_2->setSpeed(200);
  DCMotor_2->run(BACKWARD);
  DCMotor_3->setSpeed(200);
  DCMotor_3->run(FORWARD);
  DCMotor_4->setSpeed(200);
  DCMotor_4->run(FORWARD);
}

void turnRight() {
  DCMotor_1->setSpeed(200);
  DCMotor_1->run(FORWARD);
  DCMotor_2->setSpeed(200);
  DCMotor_2->run(FORWARD);
  DCMotor_3->setSpeed(200);
  DCMotor_3->run(BACKWARD);
  DCMotor_4->setSpeed(200);
  DCMotor_4->run(BACKWARD);
}

void moveLeft() {
  DCMotor_1->setSpeed(200);
  DCMotor_1->run(BACKWARD);
  DCMotor_2->setSpeed(200);
  DCMotor_2->run(FORWARD);
  DCMotor_3->setSpeed(200);
  DCMotor_3->run(BACKWARD);
  DCMotor_4->setSpeed(200);
  DCMotor_4->run(FORWARD);
}

void moveRight() {
  DCMotor_1->setSpeed(200);
  DCMotor_1->run(FORWARD);
  DCMotor_2->setSpeed(200);
  DCMotor_2->run(BACKWARD);
  DCMotor_3->setSpeed(200);
  DCMotor_3->run(FORWARD);
  DCMotor_4->setSpeed(200);
  DCMotor_4->run(BACKWARD);
}

void backward() {
  DCMotor_1->setSpeed(200);
  DCMotor_1->run(BACKWARD);
  DCMotor_2->setSpeed(200);
  DCMotor_2->run(BACKWARD);
  DCMotor_3->setSpeed(200);
  DCMotor_3->run(BACKWARD);
  DCMotor_4->setSpeed(200);
  DCMotor_4->run(BACKWARD);
}

void stopMoving() {
  DCMotor_1->setSpeed(0);
  DCMotor_1->run(RELEASE);
  DCMotor_2->setSpeed(0);
  DCMotor_2->run(RELEASE);
  DCMotor_3->setSpeed(0);
  DCMotor_3->run(RELEASE);
  DCMotor_4->setSpeed(0);
  DCMotor_4->run(RELEASE);
}

#define FRAME_HEADER      0X7B       // Frame header
#define FRAME_TAIL        0X7D       // Frame tail
#define RECV_DATA_SIZE    9          // The length of data sent by ROS to the lower machine 


uint8_t recv_data[RECV_DATA_SIZE];
uint8_t send_data[11];
int recv_count;
bool start_frame;
float x,y,z;

float motor_1_target, motor_2_target, motor_3_target, motor_4_target;
int16_t motor_1_pwm, motor_2_pwm, motor_3_pwm, motor_4_pwm;
unsigned long last_time_send_data, last_time_control;

// wheel radius
#define wheel_radius (3.0/100) // unit: meter
// half of the distance between front wheels
#define wheel_spacing (19.5/2.0/100) // unit: meter
// half of the distance between front wheel and rear wheel
#define axle_spacing (15.1/2.0/100) // unit: meter

bool ps2_control;

void setup(){
  AFMS.begin(50);

  int error = 0;
  int c=100;
  do{
    error = ps2x.config_gamepad(13,11,10,12, true, true);
    if(error == 0){
      ps2_control=true;
      break;
    }else{
      ps2_control=false;
      delay(100);
    }
  }while(c--);
  Serial.begin(38400);
  recv_count = 0;
  start_frame = false;
  last_time_send_data = millis();
  last_time_control = millis();
}

void loop(){
  if(ps2_control){ //從手柄接收模式
    ps2x.read_gamepad(false, 0);
    delay(30);
    if (ps2x.Button(PSB_PAD_UP)) {
      if (ps2x.Button(PSB_L2)) {
        DCMotor_2->setSpeed(200);
        DCMotor_2->run(FORWARD);
        DCMotor_4->setSpeed(200);
        DCMotor_4->run(FORWARD);
      } else if (ps2x.Button(PSB_R2)) {
        DCMotor_1->setSpeed(200);
        DCMotor_1->run(FORWARD);
        DCMotor_3->setSpeed(200);
        DCMotor_3->run(FORWARD);
      } else {
        forward();
      }
    } else if (ps2x.Button(PSB_PAD_DOWN)) {
      if (ps2x.Button(PSB_L2)) {
        DCMotor_2->setSpeed(200);
        DCMotor_2->run(BACKWARD);
        DCMotor_4->setSpeed(200);
        DCMotor_4->run(BACKWARD);
      } else if (ps2x.Button(PSB_R2)) {
        DCMotor_1->setSpeed(200);
        DCMotor_1->run(BACKWARD);
        DCMotor_3->setSpeed(200);
        DCMotor_3->run(BACKWARD);
      } else {
        backward();
      }
    } else if (ps2x.Button(PSB_PAD_LEFT)) {
      turnLeft();
    } else if (ps2x.Button(PSB_PAD_RIGHT)) {
      turnRight();
    } else if (ps2x.Button(PSB_L1)) {
      moveLeft();
    } else if (ps2x.Button(PSB_R1)) {
      moveRight();
    } else {
      stopMoving();
    }
    // 按下手柄X按钮，手柄震动一下
    if (ps2x.Button(PSB_CROSS)) {
      ps2x.read_gamepad(true, 200);
      delay(300);
      ps2x.read_gamepad(false, 0);
    }
    delay(2);
  }
  else{ //ROS模式
    if(Serial.available()){
      uint8_t t = Serial.read();
      if(start_frame){
        recv_data[recv_count++] = t;
      }
      else if(t==FRAME_HEADER){
        start_frame = true; // start receiving a frame
        recv_data[recv_count++] = t;
      }
      
      if(recv_count==RECV_DATA_SIZE){
        recv_count = 0;
        start_frame = false;
        if(t==FRAME_TAIL){
          // finish receiving a frame
          if(recv_data[7]==(recv_data[0]^recv_data[1]^recv_data[2]^recv_data[3]^recv_data[4]^recv_data[5]^recv_data[6])){ //檢查資料
            // received frame data correct
            x = (int16_t)((recv_data[1]<<8)|recv_data[2])/1000.0;
            y = (int16_t)((recv_data[3]<<8)|recv_data[4])/1000.0;
            z = (int16_t)((recv_data[5]<<8)|recv_data[6])/1000.0;
          }
        }
      }
    }
    // this runs at 100Hz
    if(millis()-last_time_control>10){
      motor_1_target = x-y-z*(wheel_spacing+axle_spacing);
      motor_2_target = x+y-z*(wheel_spacing+axle_spacing);
      motor_3_target = x-y+z*(wheel_spacing+axle_spacing);
      motor_4_target = x+y+z*(wheel_spacing+axle_spacing);
    
      motor_1_pwm=Incremental_PI_A(Encoder1.getspeed()*wheel_radius,motor_1_target);
      motor_2_pwm=Incremental_PI_B(Encoder2.getspeed()*wheel_radius,motor_2_target);
      motor_3_pwm=Incremental_PI_C(Encoder3.getspeed()*wheel_radius,motor_3_target);
      motor_4_pwm=Incremental_PI_D(Encoder4.getspeed()*wheel_radius,motor_4_target);
    
      DCMotor_1->setPwm(motor_1_pwm);
      DCMotor_2->setPwm(motor_2_pwm);
      DCMotor_3->setPwm(motor_3_pwm);
      DCMotor_4->setPwm(motor_4_pwm);
    
      last_time_control = millis();
    }
  }
  // send data every 50ms
  if(millis()-last_time_send_data>50){
    int32_t pos1 = Encoder1.read();
    int32_t pos2 = Encoder2.read();
    int32_t pos3 = Encoder3.read();
    int32_t pos4 = Encoder4.read();
    float rpm1 = 600*(pos1/4320.0);  // 單位:round/minute
    float rpm2 = 600*(pos2/4320.0);
    float rpm3 = 600*(pos3/4320.0);
    float rpm4 = 600*(pos4/4320.0);
    float rpm1_final = (rpm1*2*3.14)/60; // 單位:radius/second
    float rpm2_final = (rpm2*2*3.14)/60;
    float rpm3_final = (rpm3*2*3.14)/60;
    float rpm4_final = (rpm4*2*3.14)/60;
    Encoder1.write(0); // 編碼器設置為0
    Encoder2.write(0);
    Encoder3.write(0);
    Encoder4.write(0);
    
    int16_t M1, M2, M3, M4; // 把轉速變成16bits
    M1 = rpm1_final*1000;
    send_data[1] = M1;     // 分成2個8bits輸出
    send_data[0] = M1>>8;
    
    M2 = rpm2_final*1000;
    send_data[3] = M2;
    send_data[2] = M2>>8;
    
    M3 = rpm3_final*1000;
    send_data[5] = M3;
    send_data[4] = M3>>8;
    
    M4 = rpm4_final*1000;
    send_data[7] = M4;
    send_data[6] = M4>>8;
    
    Serial.write(FRAME_HEADER);
    Serial.write(send_data[0]); // 輸出4個輪子的速度
    Serial.write(send_data[1]);
    Serial.write(send_data[2]);
    Serial.write(send_data[3]);
    Serial.write(send_data[4]);
    Serial.write(send_data[5]);
    Serial.write(send_data[6]);
    Serial.write(send_data[7]);
    Serial.write(FRAME_HEADER^send_data[0]^send_data[1]^send_data[2]^send_data[3]^send_data[4]^send_data[5]^send_data[6]^send_data[7]);
    Serial.write(FRAME_TAIL);
    
    last_time_send_data = millis();
  }
}



/*incremental pi controllers*/
float Velocity_KP=300,Velocity_KI=300; 
int Incremental_PI_A (float Encoder,float Target)
{ 	
  static float Bias,Pwm,Last_bias;
  if(Target==0.0){
    Pwm=0.0;
    Bias=0.0;
    Last_bias=0.0;
    return Pwm;
  }
  Bias=Target-Encoder; //Calculate the deviation //计算偏差
  Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias; 
  if(Pwm>4096)Pwm=4096;
  if(Pwm<-4096)Pwm=-4096;
  Last_bias=Bias; //Save the last deviation //保存上一次偏差 
  return Pwm;    
}
int Incremental_PI_B (float Encoder,float Target)
{  
  static float Bias,Pwm,Last_bias;
  if(Target==0.0){
    Pwm=0.0;
    Bias=0.0;
    Last_bias=0.0;
    return Pwm;
  }
  Bias=Target-Encoder; //Calculate the deviation //计算偏差
  Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;  
  if(Pwm>4096)Pwm=4096;
  if(Pwm<-4096)Pwm=-4096;
  Last_bias=Bias; //Save the last deviation //保存上一次偏差 
  return Pwm;
}
int Incremental_PI_C (float Encoder,float Target)
{  
  static float Bias,Pwm,Last_bias;
  if(Target==0.0){
    Pwm=0.0;
    Bias=0.0;
    Last_bias=0.0;
    return Pwm;
  }
  Bias=Target-Encoder; //Calculate the deviation //计算偏差
  Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias; 
  if(Pwm>4096)Pwm=4096;
  if(Pwm<-4096)Pwm=-4096;
  Last_bias=Bias; //Save the last deviation //保存上一次偏差 
  return Pwm; 
}
int Incremental_PI_D (float Encoder,float Target)
{  
  static float Bias,Pwm,Last_bias;
  if(Target==0.0){
    Pwm=0.0;
    Bias=0.0;
    Last_bias=0.0;
    return Pwm;
  }
  Bias=Target-Encoder; //Calculate the deviation //计算偏差
  Pwm+=Velocity_KP*(Bias-Last_bias)+Velocity_KI*Bias;  
  if(Pwm>4096)Pwm=4096;
  if(Pwm<-4096)Pwm=-4096;
  Last_bias=Bias; //Save the last deviation //保存上一次偏差 
  return Pwm; 
}
