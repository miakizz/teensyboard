//Nathan Kiesman, MIT License
//Super helpful documentation at the bottom of this page, I reccomend reading this first: https://github.com/tmk/tmk_keyboard/blob/master/tmk_core/protocol/adb.c

#define adbPort 2

bool sentChar = false; //Have we already "pressed" a key?
//ASCII value is the index, and the result is the ADB scan code for the corresponding charactor
const word conversionTable[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x33ff, 0x30ff, 0x24ff, 0xffff, 0xffff, 0x24ff, 0xffff, 0xffff, 0xffff, 0x7f7f, 0x36ff, 0x367f, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x31ff, 0x3812, 0x3827, 0x3814, 0x3815, 0x3817, 0x381a, 0x27ff, 0x3819, 0x381d, 0x381c, 0x3818, 0x2bff, 0x1bff, 0x2fff, 0x2cff, 0x1dff, 0x12ff, 0x13ff, 0x14ff, 0x15ff, 0x17ff, 0x16ff, 0x1aff, 0x1cff, 0x19ff, 0x3829, 0x29ff, 0x382b, 0x18ff, 0x382f, 0x382c, 0x3813, 0x3800, 0x380b, 0x3808, 0x3802, 0x380e, 0x3803, 0x3805, 0x3804, 0x3822, 0x3826, 0x3828, 0x3825, 0x382e, 0x382d, 0x381f, 0x3823, 0x380c, 0x380f, 0x3801, 0x3811, 0x3820, 0x3809, 0x380d, 0x3807, 0x3810, 0x3806, 0x21ff, 0x2aff, 0x1eff, 0x3816, 0x381b, 0x32ff, 0xff, 0xbff, 0x8ff, 0x2ff, 0xeff, 0x3ff, 0x5ff, 0x4ff, 0x22ff, 0x26ff, 0x28ff, 0x25ff, 0x2eff, 0x2dff, 0x1fff, 0x23ff, 0xcff, 0xfff, 0x1ff, 0x11ff, 0x20ff, 0x9ff, 0xdff, 0x7ff, 0x10ff, 0x6ff, 0x3821, 0x382a, 0x381e, 0x3832, 0x33ff};
byte lastReceived;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(300); //A screaming fast 300 baud (wow)
  pinMode(adbPort, INPUT); //It starts as an input as we're listening for the attention
}

// the loop routine runs over and over again forever:
void loop() {
  while(digitalRead(adbPort) == HIGH){;} //Wait while it's high (i.e. idle)
  int startTime = micros(); 
  while(digitalRead(adbPort) == LOW){;} //We're either reciving an attention signal or the middle of data
  if(micros() - startTime > 560 && (Serial.available() > 0 || sentChar)){ //If it's long enough to be an attention signal, and we have something to send
    Serial.println("attention"); //This helps with troubleshooting wiring
    while(digitalRead(adbPort) == HIGH){;} //Wait during start bit
    byte command = 0;
    for(int i = 7; i>=0 ; i--){ //Recive 8 bits of data
      startTime = micros();
      while(digitalRead(adbPort) == LOW){;}
      command |= (micros() - startTime < 55) << i; //If it's low for less then 55us, then the bit is 1, and that gets shifted into the current bit we're reading
      while(digitalRead(adbPort) == HIGH){;} //Wait during rest of bit
    }
    while(digitalRead(adbPort) == LOW){;} //Wait during stop bit
    if(command == 0x2C){ //If it's reading from the keyboard
      startTime = micros();
      pinMode(adbPort, OUTPUT); //Now we're writing data
      while(micros() - startTime < 200){;} //Wait 200 us
      putBit(1); //Start bit
      if(sentChar){ //If there's already a key pressed
        putWord(conversionTable[lastReceived] | 0x8080); //Send the last scan code with bit 7 and 15 set, to tell the controller the key had been released
        sentChar = false;
      } else{
        lastReceived = Serial.read();
        putWord(conversionTable[lastReceived]); //Convert recivied ASCII to a Scan Code and send it
          sentChar = true;
      }
      putBit(0); //Stop bit
    }
  }
  pinMode(adbPort, INPUT); //Back to input to listen for another attention
}

void putBit(byte data){
  digitalWrite(adbPort, LOW); //A send starts low
  if(data){ //If it's a one
    delayMicroseconds(35); //Low for 35us, high for 65us
    digitalWrite(adbPort,HIGH);
    delayMicroseconds(65);
  }
  else{ //Zero
    delayMicroseconds(65); //Low for 65us, high for 35 us
    digitalWrite(adbPort,HIGH);
    delayMicroseconds(35);
  }
}

void putWord(word data){
  for(int i = 15;i>=0;i--){ //For each bit in word, MSB first
    putBit((data >> i) & 1); //Shift the data over, and it so only the one bit gets read, and send it
  }
}

