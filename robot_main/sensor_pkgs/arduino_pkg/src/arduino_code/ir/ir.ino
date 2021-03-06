#include <TimerFive.h>
#include <ros.h>
#include <life_msgs/IR.h>
#include <life_msgs/Status.h>
#include <life_msgs/LED_SET.h>

#define IR_NUM 5
#define FILTER_SIZE 10

#define LED_R_1 53
#define LED_G_1 51
#define LED_B_1 49

#define LED_B_2 52
#define LED_G_2 50
#define LED_R_2 48

float IR_data[2][IR_NUM] {};
float IR[IR_NUM][FILTER_SIZE];
float REMOVE[IR_NUM];
int IR_index = 0;
int IR_pre_index = 0;
int step = 0;
bool flag = false;
ros::NodeHandle  nh;
life_msgs::IR IR_msg;
ros::Publisher IRpub("/life/IR", &IR_msg);

void LEDCb( const life_msgs::LED_SET& msg){
  digitalWrite(LED_R_1,msg.Step.red);
  digitalWrite(LED_G_1,msg.Step.green);
  digitalWrite(LED_B_1,msg.Step.blue);
  
  digitalWrite(LED_R_2,msg.Status.red);
  digitalWrite(LED_G_2,msg.Status.green);
  digitalWrite(LED_B_2,msg.Status.blue);
}

ros::Subscriber<life_msgs::LED_SET> sub("/life/LED", &LEDCb );
void setup() {
  Timer5.initialize(5000);
  Timer5.attachInterrupt(timer_hander);
  Timer5.start();
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  pinMode(LED_R_1,OUTPUT);
  pinMode(LED_G_1,OUTPUT);
  pinMode(LED_B_1,OUTPUT);

  pinMode(LED_R_2,OUTPUT);
  pinMode(LED_G_2,OUTPUT);
  pinMode(LED_B_2,OUTPUT);
  digitalWrite(LED_R_1,LOW);
  digitalWrite(LED_G_1,HIGH);
  digitalWrite(LED_B_1,HIGH);
  
  digitalWrite(LED_R_2,LOW);
  digitalWrite(LED_G_2,HIGH);
  digitalWrite(LED_B_2,HIGH);
  
  nh.initNode();
  nh.subscribe(sub);
  nh.advertise(IRpub);
}

void loop() {
  if(flag){
    flag = false;
    switch(step){
        case 0 :
           ReadIR();
           step++;
           break;
        case 1:
           filter();
           step++;
           break;
        case 2:
           IRpub.publish(&IR_msg);
           step ++;
           break;
        case 3:
           nh.spinOnce();
           step = 0;
           break;
        default:
           step = 0;
           break;  
     }
  }
}


void ReadIR(){
    IR_pre_index = IR_index;
    IR_index = (IR_index+1)%FILTER_SIZE;
    IR[0][IR_index] = getDistanceSharp(A0);
    IR[1][IR_index] = getDistanceSharp(A1);
    IR[2][IR_index] = getDistanceSharp(A2);
    IR[3][IR_index] = getDistanceSharp(A3);
    IR[4][IR_index] = getDistanceSharp(A4);
}
float getDistanceSharp(int PIN){
  float sensorValue = analogRead(PIN);
  float volts = sensorValue*0.0048828125;
  float distance = 138773.464825 * pow(sensorValue,-1.0233470);
  if(volts < 0.2 || distance >1200)
    distance = 1200;
  return distance;
}

void timer_hander(){
  flag = true;
}

void filter(){
  //HIGH PASS FILTER
    
    IR_data[1][0] = 0.88*(IR_data[1][0] + IR[0][IR_index] - IR[0][IR_pre_index]);
    IR_data[1][1] = 0.88*(IR_data[1][1] + IR[1][IR_index] - IR[1][IR_pre_index]);
    IR_data[1][2] = 0.88*(IR_data[1][2] + IR[2][IR_index] - IR[2][IR_pre_index]);
    IR_data[1][3] = 0.88*(IR_data[1][3] + IR[3][IR_index] - IR[3][IR_pre_index]);
    IR_data[1][4] = 0.88*(IR_data[1][4] + IR[4][IR_index] - IR[4][IR_pre_index]);
    
    REMOVE[0] = (IR[0][IR_index] - IR_data[1][0]);
    REMOVE[1] = (IR[1][IR_index] - IR_data[1][1]);
    REMOVE[2] = (IR[2][IR_index] - IR_data[1][2]);
    REMOVE[3] = (IR[3][IR_index] - IR_data[1][3]);
    REMOVE[4] = (IR[4][IR_index] - IR_data[1][4]);
     
    //LOW PASS FILETER
    
    IR_data[0][0] = 0.9*IR_data[0][0] + 0.1*REMOVE[0];
    IR_data[0][1] = 0.9*IR_data[0][1] + 0.1*REMOVE[1];
    IR_data[0][2] = 0.9*IR_data[0][2] + 0.1*REMOVE[2];
    IR_data[0][3] = 0.9*IR_data[0][3] + 0.1*REMOVE[3];
    IR_data[0][4] = 0.9*IR_data[0][4] + 0.1*REMOVE[4];
  for(int i = 0 ; i< IR_NUM;i++) {
    IR_msg.ir[i] = IR_data[0][i];
  }
}
