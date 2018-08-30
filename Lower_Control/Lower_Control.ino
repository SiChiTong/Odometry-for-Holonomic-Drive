#include <EasyTransfer.h>
#include <DueTimer.h>
#include <DuePWM.h>

#define TimerEncoder Timer1
#define maxWheelRPM 300
#define maxMotRPM 468
#define maxPWM 255
#define EncoderTime 0.1
#define onebyroot2 0.7071
#define DegreeToRadian(x) x*0.0174532
#define RadianToDegree(x) x*57.295779
#define GearRatio 40.0/35.0
#define GearRatio_dash  40.0/27.0 
#define pi 3.14159263

EasyTransfer ET;
struct DATASTRUCT{
  int16_t rpm[4];
}; 
DATASTRUCT mydata;
#define UsartSerial Serial3

class Encoder
{
  public:
  int channel1;
  int channel2;
  int ppr;
  volatile long long int Count;
  volatile long long int prevCount;
  int rpm;
  float gearRatio;
  
  void initEncoder(){
    pinMode(channel1,INPUT);
    pinMode(channel2,INPUT);
   }
};

  Encoder encoder1 = {41,43,135,0,0,0,GearRatio} ,       *pEncoder1=&encoder1;
  Encoder encoder2 = {44,47,135,0,0,0,GearRatio} ,       *pEncoder2=&encoder2;
  Encoder encoder3 = {49,51,1000,0,0,0,GearRatio_dash} , *pEncoder3=&encoder3;
  Encoder encoder4 = {5,14,135,0,0,0,GearRatio} ,        *pEncoder4=&encoder4;

  Encoder *pEncoder[4]={&encoder1,&encoder2,&encoder3,&encoder4};
    
  void returnCount1();
  void returnCount2();
  void returnCount3();
  void returnCount4();

class Motor{
  public:
  int direction1;
  int direction2;
  int pwmPin;
  
  void initMotor(){
    pinMode(direction1,OUTPUT);
    pinMode(pwmPin,OUTPUT);
    pinMode(direction2,OUTPUT);
  }
  void driveMotor(float op,float maxvalue);
};


  Motor motor1={23,29,8};
  Motor *pMotor1=&motor1;

  Motor motor2={27,25,7};
  Motor *pMotor2=&motor2;

  Motor motor3={39,37,9};
  Motor *pMotor3=&motor3;
  
  Motor motor4={31, 15, 18};
  Motor *pMotor4=&motor4;
  
  Motor *pMotor[4]={&motor1,&motor2,&motor3,&motor4};

 /*********************************************************************************************************************************************/
 /******************************************************       PID          ***************************************************************************************/
class PID{
   float Kp;
   float Kd;
   float Ki;
   float maxControl;
   float minControl;
  public:
   float required;
   float prevRequired;
   float error;
   float prevError;
   float derivativeError;
   float integralError;
   void initPID(float kp,float kd,float ki,float req,float minV,float maxV);
   float pidControl(float actual);
  
};
 
  PID PIDMotor1;
  PID *pPIDMotor1=&PIDMotor1;

  PID PIDMotor2;
  PID *pPIDMotor2=&PIDMotor2;

  PID PIDMotor3;
  PID *pPIDMotor3=&PIDMotor3;

  PID PIDMotor4;
  PID *pPIDMotor4=&PIDMotor4;
  
  PID *pPIDMotor[4]={&PIDMotor1,&PIDMotor2,&PIDMotor3,&PIDMotor4};


class Auto_Bot{
  public:
  float X_pos;
  float Y_pos;
  float Angle;
  float X_vel;
  float Y_vel;
  float vel;
  Auto_Bot(){
             X_pos = 0;
             Y_pos = 0;
             Angle = 0;
             X_vel = 0;
             Y_vel = 0;
             vel = 0;
            }
};

  Auto_Bot FourWheelDrive; 
  Auto_Bot *pBot=&FourWheelDrive;
  
float output[4];

void setup() {  
  
  Serial.begin(9600);
  UsartSerial.begin(9600);
  ET.begin(details(mydata), &UsartSerial);
  for(int i=0;i<4;++i)
    pMotor[i]->initMotor();
  
  for(int i = 0;i<4;++i)
    pEncoder[i]->initEncoder();
  
  attachInterrupt(pEncoder1->channel1,returnCount1,RISING);   
  attachInterrupt(pEncoder2->channel1,returnCount2,RISING);
  attachInterrupt(pEncoder3->channel1,returnCount3,RISING);
  attachInterrupt(pEncoder4->channel1,returnCount4,RISING);

  pPIDMotor1->initPID(0.1,0,0,0,-maxMotRPM,maxMotRPM);
  pPIDMotor2->initPID(0.1,0,0,0,-maxMotRPM,maxMotRPM);    
  pPIDMotor3->initPID(0.1,0,0,0,-maxMotRPM,maxMotRPM);
  pPIDMotor4->initPID(0.1,0,0,0,-maxMotRPM,maxMotRPM);
  
  TimerEncoder.attachInterrupt(timerHandler);
  TimerEncoder.start( 1000000 * EncoderTime );
}

void loop() {
  for(int i=0;i<4;++i)
{  pPIDMotor[i]->required = 100;
  pPIDMotor[i]->prevRequired = pPIDMotor[i]->required;
}  
  //getUpperData();
 // Serial.println((int)pEncoder[0]->Count);
  //Serial.println((int)pEncoder[1]->Count);
 // Serial.println(pEncoder[2]->rpm);
  //Serial.println((int)pEncoder[3]->Count);
}

void timerHandler()
{
  int n=4;
  for(int i=0;i<4;++i)
  pEncoder[i]->rpm=((pEncoder[i]->Count - pEncoder[i]->prevCount) * 60.0)/(EncoderTime * pEncoder[i]->gearRatio * pEncoder[i]->ppr);
  
  for(int i=0;i<n;++i)
  {
    if(pPIDMotor[i]->required * pPIDMotor[i]->prevRequired>0)
    {
      output[i]=pPIDMotor[i]->pidControl(pEncoder[i]->rpm);
    }
    else
    {
     pPIDMotor[i]->prevRequired=pPIDMotor[i]->required;
     output[i]=0;
    }
  }

  for(int i=0;i<n;++i)
  if(pPIDMotor[i]->required==0) 
  output[i]=0;

  for(int i=0;i<n;++i)
  pMotor[i]->driveMotor(output[i],maxMotRPM);

  for(int i=0;i<n;++i)
  pEncoder[i]->prevCount=pEncoder[i]->Count;

  //Serial.println((int)pEncoder[1]->Count);
}
