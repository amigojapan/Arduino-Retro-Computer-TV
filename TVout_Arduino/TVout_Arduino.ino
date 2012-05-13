//Copyright 2012 Usmar A Padow usmpadow@gmail.com
/*
Wireing
TV7 9(other arduino KB2 3  SD10 11 12 13)
Datalines 4 5 6 8 A0 A1 A2 A3 A4 A5
free but not very good 0 1
*/
/*Yotson parallel protocol. implemented by Usmar A Padow usmpadow@gmail.com copyright 2012
sender: ------8 data lines------ : receiver
      : --------->-------------- :(sndack)
      : ---------<-------------- :(rcvack)

-sender waits until -<-(rcvack) is LOW (receiver is free to accept data)
-sender sets the 8 bits for transferring 1 byte.
-sender sets line ->-(sndack) HIGH    (signify that the sender has a full byte on the data lines, ready for reading)
-sender waits until line -<-(rcvack) goes HIGH (signifying that the receiver has read the data bits)
-sender sets ->-(sndack) to LOW                (nothing to be send at this moment)

-receiver waits until ->-(sndack) goes HIGH (sender has put 8 valid bits on the lines)
-receiver reads the 8 data bits
-receiver if finished sets -<-(rcvack) HIGH (telling the sender that the receiver got the 8 bits)
-receiver waits until ->-(sndack) goes LOW
-receiver sets -<-(rcvack) LOW (telling the sender to send some more)

Arduino uno pin     | TVout  
 -------------------------------------------
 pin 9              |  to 1kohm resistor  ____to center of RCA video
 pin 7              |  to 470 ohm resistor__/
 ground             |  Ground (to outside of RCA video) 
*/
//Copyright 2011 Usmar A Padow usmpadow@gmail.com
//Parallel communication between 2 arduinos

#include <stdio.h> // I think sprintf might be contained in here
#include <TVout.h>
#include <fontALL.h>

TVout TV;

int dataPin0=4;
int dataPin1=5;
int dataPin2=6;
int dataPin3=8;
int dataPin4=A5;
int dataPin5=A4;
int dataPin6=A3;
int dataPin7=A2;
int sndack=A0;//     ***Yotron should this number should be opposite on the other arduino?
int rcvack=A1;//     ***Yotron should this number should be opposite on the other arduino?

//binary conversion function Ascii to Binary by GeDaMo
int AtoB(char *b){
  int value = 0;
  int i;
 
  for (i = 0; i < 8; ++i) {
    value <<= 1;
    value |= (b[i] == '1') ? 1 : 0;
  }
  return value;
}
  
#include <string.h>
void set_datapins_to_input_mode() {
        pinMode(dataPin0,INPUT);
        pinMode(dataPin1,INPUT);
        pinMode(dataPin2,INPUT);
        pinMode(dataPin3,INPUT);
        pinMode(dataPin4,INPUT);
        pinMode(dataPin5,INPUT);
        pinMode(dataPin6,INPUT);
        pinMode(dataPin7,INPUT);
}
#define WAIT_FOR_SENDER_HIGH 0
#define WAIT_FOR_SENDER_LOW 1
int state=WAIT_FOR_SENDER_HIGH;
char buffer='\0';
/*
void poll_parallel() {
}
*/
void setup(){  
	Serial.begin(9600);
        pinMode(rcvack, OUTPUT);
        pinMode(sndack,INPUT);
        set_datapins_to_input_mode();
        TV.begin(_NTSC);
        //TV.set_hbi_hook(poll_parallel); 
        TV.select_font(font6x8);
        TV.println("TVout driver chip boot sucessfull");
}
void loop() {
  //-receiver waits until ->-(sndack) goes HIGH (sender has put 8 valid bits on the lines)
  if(state==WAIT_FOR_SENDER_HIGH) {
            if(digitalRead(sndack)==HIGH) {
                        //-receiver reads the 8 data bits
                        char bitbuffer[9];
                        sprintf(bitbuffer,"%i%i%i%i%i%i%i%i"
                        ,digitalRead(dataPin0)
                        ,digitalRead(dataPin1)
                        ,digitalRead(dataPin2)
                        ,digitalRead(dataPin3)
                        ,digitalRead(dataPin4)
                        ,digitalRead(dataPin5)
                        ,digitalRead(dataPin6)
                        ,digitalRead(dataPin7)
                        );
                        //-receiver if finished sets -<-(rcvack) HIGH (telling the sender that the receiver got the 8 bits)
                        digitalWrite(rcvack, HIGH); 
                        int read_character;
                        read_character=AtoB(bitbuffer);
                        //output the character
                        char output[2];
                        sprintf(output,"%c",read_character);
                        //Serial.print("got this character:");
                        //Serial.print(output);
                        TV.print(output);
                        state=WAIT_FOR_SENDER_LOW;
            }
      }            
      //-receiver waits until ->-(sndack) goes LOW
      if(state==WAIT_FOR_SENDER_LOW) {
            if(digitalRead(sndack)==LOW) {
                  //-receiver sets -<-(rcvack) LOW (telling the sender to send some more)
                  digitalWrite(rcvack, LOW);
                  state=WAIT_FOR_SENDER_HIGH; 
            }
      }
}
