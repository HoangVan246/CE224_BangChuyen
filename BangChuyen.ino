  #include <LiquidCrystal_I2C.h>
  #include <Wire.h>
  #include <Servo.h>
  LiquidCrystal_I2C lcd(0x27,16,2); //LCD 16x02, địa chỉ I2C là 0X27
  
  int cambien = 7; //Chân cảm biến nối chân số 7 Arduino
  int reset = 6;        //A
  int switch_mode = 8;  //B
  int increase = 11;     //C
  int decrease = 10;     //C
  int servo_pin = 9;     //servo
  
  int gtmacdinh = 1;
  int giatri;
  int count = 0;
  int greset;
  int gmode;
  int gdec;
  int ginc;
  
  char mode = 'A';
  int start = 0;

  int speed_mode = 0;
  
  int SQ = 1; //specified quantity
  int time_modeE = 0;
  
  int return_set_mode = 0;
  int list[4];
  int t = 0;
  int ST; //specified time
  int i = 0;
  int NL = 0; //number of loops

  Servo servo; //assigns PWM pin to the servo object  
  int speed_servo = 80;
   
  void setup()
  {
    Serial.begin(9600);
  
    pinMode(reset, INPUT_PULLUP);
    pinMode(switch_mode, INPUT_PULLUP);
    pinMode(decrease, INPUT_PULLUP);
    pinMode(increase, INPUT_PULLUP);
  
    pinMode(cambien, INPUT);
    servo.attach(servo_pin);
  
    lcd.init(); //Khởi động LCD
    lcd.backlight(); //Bật đèn nền
  }
  
  void loop()
  {    
    ReceiveFullData(&greset, &gmode, &gdec, &ginc);
    //============================Set mode==============================
LoopMode:
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODE: ");
    servo.write(90);
    
    switch(mode){
      case 'A':
        lcd.print("A");
        while (mode != 'B' && greset){
          greset = digitalRead(reset);
          gmode = digitalRead(switch_mode);
          
          lcd.setCursor(0,1); 
          lcd.print("QUANTITY: ");
          lcd.print(count);
          
          if (!gmode) mode = 'B';
          
          if (!greset) start = 1;
        }
        break;
      case 'B':
        lcd.print("B");
        while (mode != 'C' && greset){
          ReceiveFullData(&greset, &gmode, &gdec, &ginc);

          lcd.setCursor(0,1); //Cột 0, hàng 1
          lcd.print("QUANTITY: ");
          lcd.setCursor(10,1);
          lcd.print("      ");
          lcd.setCursor(10,1);
          lcd.print(count);
          
          if (!gmode) mode = 'C';

          if (!greset) start = 1;

          if (!gdec){if (count > 0) count--;}

          if (!ginc) count++;
          delay(200);
        }
        break;
      case 'C':
        lcd.print("C");
        while (mode != 'D' && greset){
          ReceiveFullData(&greset, &gmode, &gdec, &ginc);
          
          lcd.setCursor(8,0);
          lcd.print("SPEED: "); // 5 25 45 85
          lcd.print(speed_mode);
          lcd.setCursor(0,1); //Cột 0, hàng 1
          lcd.print("QUANTITY: ");
          lcd.print(count);
          
          if (!gmode) mode = 'D';

          if (!greset) {start = 1;}

          if (!gdec){if (speed_mode > 0) speed_mode--;}

          if (!ginc){if (speed_mode < 3) speed_mode++;}

          if (speed_mode == 0)      speed_servo = 80;
          else if (speed_mode == 1) speed_servo = 55;
          else if (speed_mode == 2) speed_servo = 30;
          else                      speed_servo = 5;
          
          delay(200);
        }
        break;
      default:
        lcd.print("D");
        while (mode != 'A' && greset){
          ReceiveFullData(&greset, &gmode, &gdec, &ginc);
          
          lcd.setCursor(8,0);
          lcd.print("SQ: ");
          lcd.print(SQ);
          lcd.setCursor(0,1); //Cột 0, hàng 1
          lcd.print("QUANTITY: ");
          lcd.print(count);
          
          if (!gmode) mode = 'A';

          if (!greset) {start = 1; count = 0;}
          
          if (!gdec){if (SQ > 1) SQ--;}

          if (!ginc) SQ++;
          delay(200);
        }


        
    }

    //============================Run==============================
    if (start){
      Start_display();
      lcd.setCursor(10,1);
      lcd.print("      ");
      servo.write(speed_servo);
LoopRS:      
      while(1){
        ReceiveFullData(&greset, &gmode, &gdec, &ginc);
        giatri = digitalRead(cambien);
        
        if (!ginc){
          servo.write(90);
          while (gdec){
            delay(200);
            gdec = digitalRead(decrease);
            ginc = digitalRead(increase);
            
            if (!ginc){
              start = 0;
              Load_display();
              goto LoopMode;
              break;
            }
          }
          servo.write(speed_servo);
        }

        if (!greset){
          count = 0;
          Reset_display();
          lcd.setCursor(0,0);
          lcd.print("MODE: ");
          lcd.print(mode);
          lcd.setCursor(8,0);
          if (mode == 'C'){
            lcd.print("SPEED: ");
            lcd.print(speed_mode);
            }
          if (mode == 'D'){
            lcd.print("SQ: ");
            lcd.print(SQ);
          }
          goto LoopRS;  
        }
        
        //--------------mode A, B, C---------------
        if (giatri != gtmacdinh){ //gia trị đọc từ cảm biến khác giá trị mặc định
              if (giatri == 0){ //Nếu giá trị = 0
                count++; //Biến đếm cộng 1
              }
              gtmacdinh = giatri; //giá trị mặc định = giá trị
            }
        lcd.setCursor(0,1); //Cột 0, hàng 1
        lcd.print("QUANTITY: ");
        lcd.print(count);
              
        //--------------mode D---------------
        if (mode == 'D'){
          if (count == SQ){
            NL++;
            servo.write(90);
            while (gdec){
              gdec = digitalRead(decrease);
              ginc = digitalRead(increase);
  
              if (NL > 9){
                lcd.setCursor(0,1);
                lcd.print("Can't add loop  ");  
                while(ginc) {ginc = digitalRead(increase);}
                start = 0;
                NL = 0;
                start = 0;
                Load_display();
                goto LoopMode;  
              }
              else{
                lcd.setCursor(14,1);
                lcd.print(NL);
                lcd.print("L");
              }
              
              if (!ginc){
                start = 0;
                NL = 0;
                start = 0;
                Load_display();
                goto LoopMode;
              }
            }
            servo.write(speed_servo);
            count = 0;
          }
        }
        
        delay(100);
      }
    }
    delay(200);
  }

  void ReceiveFullData(int *greset, int *gmode, int *gdec, int *ginc){
    *greset = digitalRead(reset);
    *gmode = digitalRead(switch_mode);
    *gdec = digitalRead(decrease);
    *ginc = digitalRead(increase);
  }
  
  void Reset_display(){
    lcd.setCursor(0,1);
    lcd.print("              ");
    lcd.setCursor(2,1); //Cột 2, hàng 0
    lcd.print("RESET....");
    delay (1000);    
  }

  void Start_display(){
    lcd.setCursor(0,1);
    lcd.print("              ");
    lcd.setCursor(2,1); //Cột 2, hàng 0
    lcd.print("START....");
    delay (1000);    
  }

  void Load_display(){
    lcd.setCursor(0,1);
    lcd.print("              ");
    lcd.setCursor(2,1); //Cột 2, hàng 0
    lcd.print("LOADING....");
    delay (1000);    
  }
