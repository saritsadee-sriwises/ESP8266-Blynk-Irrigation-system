#define sensor A0 //soil moisture sensor
#define pump D5 //mini pump

//จอ LCD
#define SDA D2
#define SCL D1

#include <Wire.h> //library สำหรับสื่อสารกับ I2C ที่สงสัญญาณให้ LCD 
#include <LiquidCrystal_I2C.h> //library สำหรับคุมจอ LCD
#include <BlynkSimpleEsp8266_SSL.h> //library สำหรับให้บอร์ด ต่อกับ blynk cloud

#define BLYNK_TEMPLATE_ID "-" //blynk template id
#define BLYNK_TEMPLATE_NAME "-" //blynk template name
#define BLYNK_AUTH_TOKEN "-" //blynk auth token

#define ssid "-" //wifi name
#define pass "-" //wifi password

LiquidCrystal_I2C lcd(0x27, 16, 2); //จอ lcd ขนาด 16x2 
BlynkTimer timer; //ตัวจับเวลา

bool manual = false; //manual
bool auto_mode = false; //โหมดอัตโนมัติ
bool motor = false; //ปั๊ม

int  threshold = 30; //ตั้งค่าความชื้นที่ต้องการเป็น %
const int hysteresis = 3; //กันแกว่ง +-3

unsigned long time = 0; //เวลา pump เริ่มเปิด
const unsigned long max_time = 600000UL; //เวลาเปิดปั๊มสูงสุดเพื่อ safety ตั้งไว้ 10 นาที

void setup(){
  pinMode(pump,OUTPUT); //ตั้งให้ pump(สาย D5) เป็น output
  digitalWrite(pump,HIGH); //ปิดปั๊ม

  Wire.begin(SDA,SCL); //เรื่มส่งสัญญาณ
  lcd.init(); //เปิด LCD
  lcd.backlight(); //เปิดไฟ backlight จอ LCD
  lcd.clear(); //clear จอ

  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass,"blynk.cloud",9443); //ต่อ BLYNK
  timer.setInterval(1000L,main_loop); //ตั้งให้ run function main_loop ทุกๆ 1 วิ
}

void setup_pump(bool on){ //function เพื่อเปิดปิดปั๊ม
  if(motor == on) return; //ถ้าปั๊มเปิดอยู่ให้หยุดการทำงาน(ป้องกันการสั่งซ้ำ)
  motor = on; //เปิดปั๊ม

  if(on){ //ถ้า function setup_pump เป็น true 
    time = millis(); //เริ่มจับเวลา
    digitalWrite(pump,LOW); //เปิดปั๊ม
    lcd.setCursor(0,1);
    lcd.print("Pump is ON");
  }else{
    digitalWrite(pump,HIGH); //ปิดปั๊ม
    lcd.setCursor(0,1);
    lcd.print("Pump is OFF");
  }
}

BLYNK_WRITE(V1){
  manual = (param.asInt() == 1); //รับค่า manual จากBLYNK

  if(manual){
    auto_mode = false; //ปิดโหมด auto
    Blynk.virtualWrite(V2,0); //เปลี่ยนปุ่ม auto ใน app ให้เป็น OFF
    setup_pump(true); //เปิดปั๊ม
  }else{
    setup_pump(false); //ปิดปั๊ม
  }
}

BLYNK_WRITE(V2){
  auto_mode = (param.asInt() == 1); //รับค่าโหมด auto จากBLYNK

  if(auto_mode){
    manual = false; //ปิดโหมด manual
    Blynk.virtualWrite(V1,0); //เปลี่ยนปุ่ม manual ใน app ให้เป็น OFF
  }else{
    if(manual == false) setup_pump(false); //ถ้า manual ปิดอยู่ ให้ปิดปั๊ม
  }
}

BLYNK_WRITE(V3){
  threshold = param.asInt(); //รับค่า threshold ที่ปรับมาจาก BLYNK
}

void main_loop(){
  int moisture = analogRead(sensor); //ให้ตัวแปล moisture อ่านค่าความชื่นจาก sensor(A0)
  moisture = map(moisture,0,1023,0,100); //เปลี่ยนค่าที่อ่านได้จาก 0-1023 เป็น 0-100(%)
  moisture = 100 - moisture; //กลับค่าจากค่าสูง ดินแห้ง และค่าต่ำดินเปียก เป็นค่าต่ำ ดินแห้ง และค่าสูงดินเปียก

  Blynk.virtualWrite(V0,moisture); //ส่งค่าให้ V0 ในBLYNK แสดงค่าความชื้น
  lcd.setCursor(0,0);
  lcd.print(moisture); //print ค่าความชื้น (%)
  lcd.print("%");

  if(manual == false && auto_mode == true){ //ถ้าโหมด manual ปิดอยู่ แล้วโหมด auto เปิดอยู่
    if(moisture < threshold - hysteresis){ //ค่าความชื้นต่ำกว่าค่าที่ตั้งไว้ -3 (%)
      setup_pump(true); //เปิดปั๊ม
    }else if(moisture > threshold + hysteresis){ //ค่าความชื้นมากกว่าค่าที่ตั้งไว้ +3 (%)
      setup_pump(false); //ปิดปั๊ม
    }
  }
  else if(manual == false && auto_mode == false){ //ถ้าโหมด manual ปิดอยู่ แล้วโหมด auto ปิดอยู่
    setup_pump(false); //ปิดปั๊ม
  }

  if(motor == true && (millis() - time >= max_time)){ //ถ้า motor เปิดอยู่แล้ว เวลาเปิดปั๊มเกิน 10 นาที
    setup_pump(false); //ปิดปั๊ม
    manual    = false; //ปิด manual
    auto_mode = false; //ปิด โหมด auto

    Blynk.virtualWrite(V1,0); //เปลี่ยนเป็น OFF
    Blynk.virtualWrite(V2,0); //เปลี่ยนเป็น OFF

    lcd.setCursor(0,1);
    lcd.print("SAFETY OFF");
  }
}

void loop(){
  Blynk.run(); //ต่อกับ blynk
  timer.run(); //run blynk timer (ตั้้งชื่อเป็น timer)
}