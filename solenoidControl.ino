#include <LiquidCrystal.h>
#include <CytronMotorDriver.h>

CytronMD motor(PWM_DIR, 3, 4);  // PWM = Pin 3, DIR = Pin 4.

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  
int x = analogRead (A0); 

int mode=0;
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;

 // For V1.0 comment the other threshold and use the one below:
/*
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 195)  return btnUP;
 if (adc_key_in < 380)  return btnDOWN;
 if (adc_key_in < 555)  return btnLEFT;
 if (adc_key_in < 790)  return btnSELECT;
*/


 return btnNONE;  // when all others fail, return this...
}

void setup()
{     
 pinMode(D1,OUTPUT);
 pinMode(D11,OUTPUT);
 pinMode(D12,OUTPUT);
 pinMode(D13,OUTPUT);   
 CytronMD motor(PWM_DIR, A0, A1); 
 lcd.begin(16, 2);           
 lcd.setCursor(0,0);
 lcd.print("Push the buttons");
}

void loop(){ 
 lcd.setCursor(0,0);
 lcd_key = read_LCD_buttons();

 switch (lcd_key){
   case btnRIGHT:
     {
     lcd.print("RIGHT ");
     if(mode==0){
      lcd.print ("Solenoid ON");
      motor.setSpeed(-255);
      delay(1000);
     }
     if(mode==1){
      lcd.print ("Mositure Sensors ON");
      digitalWrite(D1, HIGH);
      digitalWrite(D11, HIGH);
      digitalWrite(D12, HIGH);
      digitalWrite(D13, HIGH);
     }
     break;
     }
   case btnLEFT:
     {
     lcd.print("LEFT   ");
     if(mode==0){
      lcd.print ("Solenoid OFF");
      motor.setSpeed(255);
      delay(1000);
     }
     if(mode==1){
      lcd.print ("Mositure Sensors OFF");
      digitalWrite(D1, LOW);
      digitalWrite(D11, LOW);
      digitalWrite(D12, LOW);
      digitalWrite(D13, LOW);
     }
     break;
     }
   case btnUP:
     {
     lcd.print("Solenoid");
     mode=0;
     break;
     }
   case btnDOWN:
     {
     lcd.print("Mositure Sensors");
     mode=1;
     break;
     }
   case btnSELECT:
     {
     lcd.print("SELECT");
     break;
     }
     case btnNONE:
     {
     lcd.print("NONE  ");
     break;
     }
 }

}
