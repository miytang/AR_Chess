#include <Braccio.h>
#include <Servo.h>
#define MAX_CH 4

Servo base;
Servo shoulder;
Servo elbow;
Servo wrist_rot;
Servo wrist_ver;
Servo gripper;

int event = 0;
int m1 = 90;
int m2 = 90;
int m3 = 90;
int m4 = 90;
int m5 = 90;
int m6 = 100;

char a1[MAX_CH + 1];
int newAngle = 10;
int angle = 90;
int i = 0;
int count = 0;
char temp[MAX_CH];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("Hello World!\n");
  Braccio.begin();
}

void serialEvent()
{ 
  if(Serial.available()){
    char ch = Serial.read();

    if(i < MAX_CH && isDigit(ch)){
      a1[i++] = ch;
    }
    else if (ch == 'b'){
      //Serial.write("Base\n");
      a1[i] = '\0';
      m1 = atoi(a1);
      //Serial.print(m1);
      i = 0;
    }
    else if(ch == 's'){
      //Serial.write("Shoulder\n");
      a1[i] = '\0';
      m2 = atoi(a1);
      //Serial.print(m2);    
      i = 0;
    }
    else if(ch == 'e'){
      //Serial.write("Elbow\n");
      a1[i] = '\0';
      m3 = atoi(a1);
      //Serial.print(m3);
      i = 0;
    }
    else if (ch == 'w'){
      //Serial.write("Wrist ver\n");
      a1[i] = '\0';
      m4 = atoi(a1);
      //Serial.print(m4);
      i = 0;
    }
    else if (ch == 'v'){
      //Serial.write("Wrist rot\n");
      a1[i] = '\0';
      m5 = atoi(a1);
      //Serial.print(m5);
      i = 0;
    }
    else if (ch == 'g'){
      //Serial.write("Gripper\n");
      a1[i] = '\0';
      m6 = atoi(a1);
      //Serial.print(m6);
      i = 0;
    }
    else if (ch == 'z')
    {
      //Serial.write("Boosted\n");
      count = 1;
    }
    else if (ch == 'r')
    {
      m1 = 90; // Base
      m2 = 90; // Shoulder
      m3 = 135; // Elbow
      m4 = 180; // Wrist
      m5 = 90; // Wrist Rotation
      m6 = 0; // Grip
      count = 10;
    }
    //count ++;
  }
  
  /*Serial.print("base: "); 
  Serial.println(m1);
  Serial.print("shoulder: ");
  Serial.println(m2);
  Serial.print("elbow: ");
  Serial.println(m3);
  Serial.print("wrist: ");
  Serial.println(m4);
  Serial.print("wrist rot: ");
  Serial.println(m5);
  Serial.print("grip: ");
  Serial.println(m6);
  Serial.println("==========");*/
}

void loop() {
  if (count){
    count = 0;
    Braccio.ServoMovement(20, m1, m2, m3, m4, m5, m6);
  }
}
