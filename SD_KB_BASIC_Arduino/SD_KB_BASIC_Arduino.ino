//Copyright 2012 Usmar Padow usm@padow@gmail.com and Greg Cox
//SD memory manager and BASIC interpreter
/*
Wireing
KB2 3 (other arduino TV7 9) SD10 11 12 13
Datalines 4 5 6 8 A0 A1 A2 A3 A4 A5
free but not very good 0 1
*/
/* to use  PS2Keyboard library
  
  PS2Keyboard now requries both pins specified for begin()

  keyboard.begin(data_pin, irq_pin);
  
  Valid irq pins:
     Arduino:      2, 3
     Arduino Mega: 2, 3, 18, 19, 20, 21
     Teensy 1.0:   0, 1, 2, 3, 4, 6, 7, 16
     Teensy 2.0:   5, 6, 7, 8
     Teensy++ 1.0: 0, 1, 2, 3, 18, 19, 36, 37
     Teensy++ 2.0: 0, 1, 2, 3, 18, 19, 36, 37
     Sanguino:     2, 10, 11
  
  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
  
  Like the Original library and example this is under LGPL license.
  
  Modified by Cuninganreset@gmail.com on 2010-03-22
  Modified by Paul Stoffregen <paul@pjrc.com> June 2010
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
*/
//Copyright 2011 Usmar A Padow usmpadow@gmail.com
//Parallel communication between 2 arduinos
#ifdef __linux__
	#include "ArduinoSimulator.cpp"
	#define byte_type char

	char* freeMemory(){
		  return "This is just a dummy,so the ammount of memory wont be displayed";
	}
		
	//SDCard library simulator
	char SDmem[1000000];
	void sd_raw_write(int offset, char byteArray[],int length){
  		for(int counter=0;counter<length;counter++){
    		SDmem[counter+offset]=byteArray[counter];
  		}
	}

	void sd_raw_read(int offset, char byteArray[],int length){
  		for(int counter=0;counter<length;counter++){
    		byteArray[counter]=SDmem[counter+offset];
  		}
	}
	void sd_raw_sync(){
	}
	int sd_raw_init(){//may want to make a system to make sure that this function is called before you can read or write to the SD card, or show an error
		return 1;
	}
#else
	#include <MemoryFree.h>
	//int var[100];//this works
	#include <sd-reader_config.h>
	#include <sd_raw.h>
	#include <sd_raw_config.h>
        #include <Arduino.h>
    #define byte_type byte
#endif

#include <stdio.h> // I think sprintf might be contained in here
//#include <stdlib.h>  /* exit */
//#include <errno.h>   /* errno */

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

//these libraries for the binary converter functions
//#include <fstream>
//#include <cmath>//dont need this on the arduino cause pow used by the binary converstion functions seems to be already included by the arduino software
#include <string.h>

//binary conversion function Binary to Ascii by GeDaMo
void BtoA(unsigned char ch, char *b) {
  int mask = 0x80;
  int i;
 
  for (i = 0; i < 8; ++i) {
    b[i] = ch & mask ? '1' : '0';
    mask >>= 1;
  }
 
  b[i] = '\0';
}
 


void set_datapins_to_output_mode() {
        pinMode(dataPin0,OUTPUT);
        pinMode(dataPin1,OUTPUT);
        pinMode(dataPin2,OUTPUT);
        pinMode(dataPin3,OUTPUT);
        pinMode(dataPin4,OUTPUT);
        pinMode(dataPin5,OUTPUT);
        pinMode(dataPin6,OUTPUT);
        pinMode(dataPin7,OUTPUT);
}        
#define WAIT_FOR_RECIEVER_FREE 0
#define WAIT_FOR_RECIEVER_READ_BYTES 1
int state=WAIT_FOR_RECIEVER_FREE;

/*  PS2Keyboard library example
  
  PS2Keyboard now requries both pins specified for begin()

  keyboard.begin(data_pin, irq_pin);
  
  Valid irq pins:
     Arduino:      2, 3
     Arduino Mega: 2, 3, 18, 19, 20, 21
     Teensy 1.0:   0, 1, 2, 3, 4, 6, 7, 16
     Teensy 2.0:   5, 6, 7, 8
     Teensy++ 1.0: 0, 1, 2, 3, 18, 19, 36, 37
     Teensy++ 2.0: 0, 1, 2, 3, 18, 19, 36, 37
     Sanguino:     2, 10, 11
  
  for more information you can read the original wiki in arduino.cc
  at http://www.arduino.cc/playground/Main/PS2Keyboard
  or http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
  
  Like the Original library and example this is under LGPL license.
  
  Modified by Cuninganreset@gmail.com on 2010-03-22
  Modified by Paul Stoffregen <paul@pjrc.com> June 2010
*/
   
#include <PS2Keyboard.h>

const int DataPin = 2;
const int IRQpin =  3;

PS2Keyboard keyboard;


#include<string.h>
char outputbuffer[100]="";
void output_update() {
      //-sender waits until -<-(rcvack) is LOW (receiver is free to accept data)
      if(state==WAIT_FOR_RECIEVER_FREE) {
            if(digitalRead(rcvack)==LOW && outputbuffer[0]!='\0') {
                        //-sender sets the 8 bits for transferring 1 byte.
                        char bitbuffer[9];
                        BtoA(outputbuffer[0], bitbuffer);
                        digitalWrite(dataPin0, bitbuffer[0]=='1'?HIGH:LOW);
                        digitalWrite(dataPin1, bitbuffer[1]=='1'?HIGH:LOW);
                        digitalWrite(dataPin2, bitbuffer[2]=='1'?HIGH:LOW);
                        digitalWrite(dataPin3, bitbuffer[3]=='1'?HIGH:LOW);
                        digitalWrite(dataPin4, bitbuffer[4]=='1'?HIGH:LOW);
                        digitalWrite(dataPin5, bitbuffer[5]=='1'?HIGH:LOW);
                        digitalWrite(dataPin6, bitbuffer[6]=='1'?HIGH:LOW);
                        digitalWrite(dataPin7, bitbuffer[7]=='1'?HIGH:LOW); 
                        //-sender sets line ->-(sndack) HIGH    (signify that the sender has a full byte on the data lines, ready for reading)
                        digitalWrite(sndack, HIGH);
                        strcpy(outputbuffer,outputbuffer+1);
                        //Serial.print(outputbuffer);
                        state=WAIT_FOR_RECIEVER_READ_BYTES;
            }
      }            
      //-sender waits until line -<-(rcvack) goes HIGH (signifying that the receiver has read the data bits)
      if(state==WAIT_FOR_RECIEVER_READ_BYTES) {
            if(digitalRead(rcvack)==HIGH) {
                  //-sender sets ->-(sndack) to LOW                (nothing to be send at this moment)
                  digitalWrite(sndack, LOW);
                 state=WAIT_FOR_RECIEVER_FREE; 
            }
      }
}
void print2(char str[]) {
      strcpy(outputbuffer,str);
      while(outputbuffer[0]!='\0') output_update();
}
void newline() {
	print2("\r\n");
}

void print2nl(char str[]) {
	print2(str);
	newline();
}
void cls() {
      //implement later as a command
        //lcd.clear();
        //strcpy(buffer,"                                                                                ");//20*4 spaces
        //pos=0;
}




//Arduino SD BASIC interpretera Copyright 2011 Usmar A Padow

#include<stdio.h>
#include<stdint.h>
//we want this only to compule for the simulator, for the real arduino, we want to use the SD card

//#include <PS2uartKeyboard.h>
//PS2uartKeyboard keyboard;
#define rows 20
#define cols 4
void printWelcome();

void setup() {
        delay(1000);
        keyboard.begin(DataPin, IRQpin);
	Serial.begin(9600);
        pinMode(rcvack, INPUT);
        pinMode(sndack,OUTPUT);
        set_datapins_to_output_mode();
  
        printWelcome();
        if(!sd_raw_init())  {
           print2("MMC/SD initialization failed");      
        }
        randomSeed(millis());//randomize the seed
}


int ProgNum=0;
unsigned char NumberOfLines=0;
#define LineLength 30//I made this small because o the limit in arudino memory
//warning this is because of the limit of the size of char which we will use to store ln, we may change it later
#define LineCount 250
#define NumberOfPrograms 10
#define ProgramSize LineLength*LineCount
#define ln0offset ProgramSize*NumberOfPrograms
//nol=number of lines
#define noloffset ln0offset+(LineCount*NumberOfPrograms)
//greg, is this the same thing? #define noloffset ln0offset+(ProgramSize*NumberOfPrograms)
//#include <arduino.h>
//byte_type tempbytes[LineLength];
byte_type tempbytes[LineCount];//this is nessesary to have a program delete command,  but it might be too much memory when I mix it with the TVout library, so I may need to change it to the commented line above

char OneLine[LineLength];//used many times, global var
int i;
int ReturnStack[5];//increase this if you think there will be more than 5 levels of return from GOSUBs
int StackCount=0;

void delete_lns() {
   for(i=1;i<LineCount;i++){
	      tempbytes[i]=0;
    }
	unsigned long offset;
	offset=(unsigned long) ln0offset+(LineCount*ProgNum);
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
	sd_raw_write(offset,tempbytes,LineCount+1);
        sd_raw_sync();
        keyboard.begin(DataPin, IRQpin);
}

void delete_nol() {
	tempbytes[0]=0;//set teh number of lines back to 0
	unsigned long offset;
	offset=(unsigned long) noloffset+ProgNum;
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
	sd_raw_write(offset,tempbytes,1);
        sd_raw_sync();
        keyboard.begin(DataPin, IRQpin);
}
void writeString (int offset, char str[], int len) {
  for(int counter=0;counter<len;counter++){
    tempbytes[counter]=str[counter];
    //char output[100];
    //sprintf(output,"%c",str[counter]);
    //print2(output);
  }
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
        sd_raw_write(offset,tempbytes,len);
        sd_raw_sync();
        keyboard.begin(DataPin, IRQpin);

}


void set_ln(unsigned char RealLineNumber, unsigned char SDPointer) {
	tempbytes[0]=SDPointer;
	unsigned long offset;
	offset=(unsigned long) ln0offset+(LineCount*ProgNum)+RealLineNumber;
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
	sd_raw_write(offset,tempbytes,1);
        sd_raw_sync();
        keyboard.begin(DataPin, IRQpin);
}

char get_ln(unsigned char RealLineNumber) {
        unsigned long offset;
	offset=(unsigned long) ln0offset+(LineCount*ProgNum)+RealLineNumber;
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
        sd_raw_read(offset,tempbytes,1);//it seems this read fails
        keyboard.begin(DataPin, IRQpin);
	return tempbytes[0];
}
void set_nol() {
	tempbytes[0]=NumberOfLines;
	unsigned long offset;
	offset=(unsigned long) noloffset+ProgNum;
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
	sd_raw_write(offset,tempbytes,1);
        sd_raw_sync();
        keyboard.begin(DataPin, IRQpin);
}
int get_nol() {
	unsigned long offset;
	offset=(unsigned long) noloffset+ProgNum;
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
  	sd_raw_read(offset,tempbytes,1);
        keyboard.begin(DataPin, IRQpin);
	return tempbytes[0];
}

//void bytecpy(byte_type info[]){
 void pr(unsigned long offset,int len){
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
        sd_raw_read(offset,tempbytes,len);
        keyboard.begin(DataPin, IRQpin);
  for(int counter=0;counter<len;counter++) {
    //print2((char)tempbytes[counter]);
  }
}

 void pr2(unsigned long offset,int len){
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
        //print2("a");//fails at this brake point
        //char tmp[70];
        //this is a mistakesprintf(tmp,"offset ul:%ul",offset);
        //print2(tmp);
        /*
        print offset
        sprintf(tmp,"offset lu:%lu",offset);
        print2(tmp);
        */
        //nope this doesnt work either :(delay(50);//dont know, try giving it a rest between reads to teh SD card???
        //byte info[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//initialize to 30 zeros
        sd_raw_read(offset,tempbytes,len);//it seems this sometimes fails and crashes the arduino, the offset is pointing to the correct position (found out by debugging), len is 30
        //sd_raw_read(offset,info,len);//it seems this sometimes fails anc crashes the arduino, the offset is pointing to the correct position (found out by debugging), len is 30

        //print2("b");
        keyboard.begin(DataPin, IRQpin);

        for(int counter=0;counter<len;counter++) {
              OneLine[counter]=(char)tempbytes[counter];
              //OneLine[counter]=(char)info[counter];
              //print2((char)tempbytes[counter]);
        }
        //OneLine[30]='\0';
        //print2(OneLine);

  //print2("c");
}
/*
void bytecpy(unsigned long offset,int len){
      int counter;
      print2("\r\n");
      for(counter=0;counter<LineLength;counter++) {
           OneLine[counter]=(char)tempbytes[counter];
           char tmp[3];
           sprintf(tmp,"*%c",(char)tempbytes[counter]);
           print2(tmp);
      }
      //p[counter]='\0';
 
       sd_raw_read(offset,tempbytes,len);
       for(int counter=0;counter<len;counter++) {
            print2((char)tempbytes[counter]);
       }
}
*/
/* I want this but it uses RAM
char * string_repeat( int n, const char * s ) {
  size_t slen = strlen(s);
  char * dest = malloc(n*slen+1);
 
  int i; char * p;
  for ( i=0, p = dest; i < n; ++i, p += slen ) {
    memcpy(p, s, slen);
  }
  *p = '\0';
  return dest;
}
*/

void AddLine(unsigned char RealLineNumber, char str[]) {
        unsigned char SDPointer=get_ln(RealLineNumber);//this seems to conflict with the var[0]=0; inside loop()
        //char SDPointer=0;//this works, so it is probably something inside get_ln();
        //print2("here7");
         
        //delay(1000);
	unsigned long offset;
	if(SDPointer) {//editing an existing line
		offset=ProgNum*ProgramSize+SDPointer*LineLength;
	} else {//creating a new line
		if(NumberOfLines+1>LineCount) { 
			print2("Error, the maximum number of lines possible is 250");
		}
		NumberOfLines++;
                set_nol();
		set_ln(RealLineNumber,NumberOfLines);
		offset=(unsigned long) ProgNum*ProgramSize+NumberOfLines*LineLength;
	}
        writeString(offset,str,strlen(str)+1);//the +1 is so that it will also add the null character of teh string        
/*
        //make a string that takes up the whole line legnth
        strcpy(OneLine,"                              ");//30 spaces
        for(i=0;i<strlen(str)-1;i++) {//copy the string, except the null char
              OneLine[i]=str[i];
        }
        print2("-->");
        print2(OneLine);
        print2("<--");
	writeString(offset,OneLine,LineLength);
 */      
}
void  get_Line(char *p, unsigned char RealLineNumber) {
	unsigned char SDPointer=get_ln(RealLineNumber);
	unsigned long offset;		
	if(SDPointer==0) {
		//*p='\0';
		strcpy(p,"EMPTYLINE");
		return;
	}	
	offset=(unsigned long)ProgNum*ProgramSize+SDPointer*LineLength;
        detachInterrupt(IRQpin);
        detachInterrupt(DataPin);
	sd_raw_read(offset,tempbytes,LineLength);
        keyboard.begin(DataPin, IRQpin);

        
	//bytecpy(p,tempbytes);
        //print2(p);
}

void  get_Line2(unsigned char RealLineNumber) {
	unsigned char SDPointer=get_ln(RealLineNumber);
        //OKprint2("\r\nSDPointer=get_ln(RealLineNumber);\r\n");
	unsigned long offset;		
	if(SDPointer==0) {
		//*p='\0';
		strcpy(OneLine,"EMPTYLINE");
                return;
	}	
	offset=(unsigned long)ProgNum*ProgramSize+SDPointer*LineLength;
	//sd_raw_read(offset,tempbytes,LineLength);
        //delay(1000);
        //print2("\r\npr(offset,LineLength)=");
        //pr(offset,LineLength);
        //print2("\r\nend");

        
        //char tmp[30];
        //sprintf(tmp,"\r\nRLN=%i,offset=%lu\r\n",RealLineNumber,offset);
        //print2(tmp);

        //print2("\r\npr(offset,LineLength)=");
        //print2("\r\nbefore pr2(offset,LineLength);\r\n");
        pr2(offset, LineLength );//bug in this function
        //print2("\r\nafter pr2(offset,LineLength);\r\n");
        //print2("\r\nend");

}

//Spinelli's Basic interpreter, ported to arduino bu Usmar A Padow usmpadow@gmail.com
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int *callp,callstk[10];
#define NumberOfNestedLoops 4
int forln[NumberOfNestedLoops],forend[NumberOfNestedLoops],for_itteration[NumberOfNestedLoops];
int var[100],CurrentLineNumber,quoted;
char buf[100],F[2];//size of buf might be reducible
//char *ln[100],*p,*q,*x,*y,*z,*s,*d,*f;
char *p,*q,*x,*y,*z,*s,*d,*f;


char *findstr(char *s,char *o) {
        for(x=s;*x;x++){
                for(y=x,z=o; *z&&*y==*z; y++)
                        z++;
                if(z>o&&!*z)
                        return x;
        }
        return 0;
}
void editln(){
        CurrentLineNumber=atoi(buf);
        //ln[l] && free(ln[l]);
        p=findstr(buf," ");
        if(findstr(buf," ")) {
              AddLine(CurrentLineNumber, p+1);
        }
}

int eqexp();
int atomexp();
int mulexp();
int addexp();
int cmpeexp();
int cmpexp();

int eqexp(){
        int o=cmpexp();
        switch(*p++){
                ;
                break;
        case '=':
                return o == eqexp();
                ;
                break;
        case '#':
                return o != eqexp();
        default:
                p--;
                return o;
        }
}
int debug(){
        char tmp[6];
        sprintf(tmp,"*p is %s,var[*p] is %d\r\n",*p,var[*p]);
        print2(tmp);
        return 0;
}
    
int atomexp(){
        int o;
        return *p=='-'
                ? p++,-atomexp()
                : *p>='0' && *p<='9'
                        ? strtol(p,&p,0)
                        : *p=='('
                                ? p++,o=eqexp(),p++,o
                                : var[*p++];
}

int mulexp(){
        int o=atomexp();
        switch(*p++){
                ;
                break;
        case '*':
                return o * mulexp();
                ;
                break;
        case '/':
                return o / mulexp();
        default:
                p--;
                return o;
        }
}

int addexp(){
        int o=mulexp();
        switch(*p++){
                ;
                break;
        case '+':
                return o + addexp();
                ;
                break;
        case '-':
                return o - addexp();
        default:
                p--;
                return o;
        }
}

int cmpeexp(){
        int o=addexp();
        switch(*p++){
                ;
                break;
        case '$':
                return o <= cmpeexp();
                ;
                break;
        case '!':
                return o >= cmpeexp();
        default:
                p--;
                return o;
        }
}
int cmpexp(){
        int o=cmpeexp();
        switch(*p++){
                ;
                break;
        case '<':
                return o < cmpexp();
                ;
                break;
        case '>':
                return o > cmpexp();
        default:
                p--;
                return o;
        }
}

void puts2(char *s) {
       //print2("puts2");
       print2(s);
       //return 0;
}
void gets2(char *str) {
      int inbyte=0;
      char tmp[2];
      //strcpy(s,"");//clear string
      str[0]='\0';//initialize string
      while(inbyte!=13){//13 is enter
            //print2("x");
            
            if(keyboard.available()) {
                inbyte=keyboard.read();
                //convert upper case to lower case and lower case to upper case
                if(inbyte>64&& inbyte<91) {//the character is uppercase
                      inbyte+=32;//convert to lower case
                } else{
                      if(inbyte>60&& inbyte<123) {//the character is lowercase
                            inbyte-=32;//convert to upper case
                      }
                }
                //echo character to LCD if it is not ENTER
                sprintf(tmp,"%c",inbyte);
                //later add check for input lenght;
/*
                if(strlen(str)+1>4) {
                      print2("INPUT TOO LONG");
                      return;
                }
*/
                strcat(str,tmp);//add this byte_type
                if(inbyte!=13) print2(tmp);//echo inbyte to terminal
            }

      }
      //how to flush serials //<Ragnorok> Read bytes and toss them out until no more are available          while(inbyte_type!=13){//13 is enter
      while(keyboard.available()) {
                inbyte=keyboard.read();
       }
 
}



int print_disk_info();
int sample();
int readDisk();

byte_type incomingbyte_type;
long int address;


/*
void setup()
{
  
  Serial.begin(9600);
  delay(1000);

}

char buf[100];
*/
void loop() {
        NumberOfLines=get_nol();//get the LineCount for program 0
        while(1) {
                //puts2("\n\rOk\n\");
                newline();
                strcpy(buf,"OK");
                print2nl(buf);
               //needs to be gets on PC and gets2 on arduino
               #ifdef __linux__
                     gets(buf);
               #else
                     gets2(buf);
               #endif
                if(buf[0]=='M') {//case 'M': // MEM  shows how much free memory is left
                        print2("in loop(),freeMemory()=");
                        sprintf(buf,"%i",freeMemory());
                        print2(buf);
                }
                switch(buf[0]) {
                case 'R': // RUN
                        callp=callstk;
                        for(i=0;i<100;i++) var[i]=0;//reinitializes all variables to 0
                        for(CurrentLineNumber=0;CurrentLineNumber<LineCount && CurrentLineNumber!=-1;CurrentLineNumber++) {
                                        //print2("\r\bbefore get_Line2\r\n");
                                        //char tmp[50];
                                        //sprintf(tmp,"CurrentLineNumber:%i",CurrentLineNumber);
                                        //print2(tmp);
			      		get_Line2(CurrentLineNumber);//freezes sometimes in this function
                                        //print2("\r\nafter get_Line2\r\n");
                        if(strcmp(OneLine,"EMPTYLINE")!=0) {//EXECUTE this line
                                      if(OneLine[0]=='P') {//if it is PRINT
                                            if(OneLine[6]=='"') {//if there is a quote mark it is a normal print statement
                                                  //*d=0; //puts2(buf+6);
                                                  int counter=7;//after the quotes
                                                  char tmp[2];
                                                  while(1){
                                                  if(OneLine[counter]=='"') break;//look for the ending quotes
                                                        counter++;
                                                  }
                                                  //print2("\n\r");
                                                  newline();
                                                  int counter2;
                                                  tmp[1]='\0';
                                                  for(counter2=7;counter2<=counter-1;counter2++) {
                                                        //sprintf(tmp,"%c",s[counter2]);
                                                        tmp[0]=OneLine[counter2];
                                                        print2(tmp);
                                                  }      
                                            } else {// if there is not quote mark, it will print the content of the variable
                                                  char tmp[20];
                                                  //p=buf+5;
                                                  //p[1]='\0';
                                                  sprintf(tmp,"%i",var[OneLine[6]]);
                                                  //newline();
                                                  print2nl(tmp);
                                            }
                                      }
                                      if(OneLine[0]=='I' && OneLine[1]=='N') {//if it is INPUT
                                            newline();
                                            //print2nl("INPUT NUMBER:");
										    #ifdef __linux__
											 	 gets(buf);
										    #else
												 gets2(buf);
										    #endif
                                            p=buf;
                                            var[OneLine[6]]=eqexp();
                                            //sscanf(buf,"%i",&i);
                                            //var[OneLine[6]]=i;
                                      }
                                      if(OneLine[1]=='=') {//assignment
                                            if(OneLine[2]=='R' && OneLine[3]=='N'){//var=RND, you must set X to the minimum number first then Y to the maximum number: example 10 X=10 20 Y=20 30 N=RND  now N will contain a value between 10 and 20
                                                  var[OneLine[0]]=random(var['X'], var['Y']);
                                                  continue;
                                            } else {//just assign
                                                  p=OneLine+2, var[*OneLine]=eqexp();p=OneLine+2, var[*OneLine]=eqexp();
                                                  continue;
                                            }
                                      }
                                      switch(OneLine[0]) {
                                            case 'E':        // END 
                                                    CurrentLineNumber=-2;//must be -2 cause the for loop will increase it to -1
                                                    break;
                                            case 'I':        //IF exp THEN line-number 
                                                    if(OneLine[1]=='F') {
															//char OldStr[LineLength];
															strcpy(buf,OneLine);
                                                            (*(q=findstr(OneLine,"TH"))=0,
                                                                p=OneLine+2,
                                                                eqexp() && (p=q+4, CurrentLineNumber=eqexp()-1));
                                                                //printf("%i",CurrentLineNumber);
                                                            //p=OneLine+2,
                                                            //eqexp() && (p=q+4, CurrentLineNumber=eqexp()-1));
                                                            //printf("%i",eqexp());
                                                            
                                                            //ok, eqexp() is 1 if the expression is true and 0 if the expression is false, now I just haveto get the line number into CurrentLineNumber//printf("eqexp()=%i",eqexp());
															//p=OneLine+2;
															if(eqexp()) {
																q=findstr(buf,"THEN ");
																p=q+5;
																//printf("p=%i", eqexp());//this shows that eqexp returns an integer with the line number to goto
																CurrentLineNumber=eqexp()-1;
																continue;
															}
                                                    }
                                                    break;
                                            case 'G':        // GOTO|GOSUB
                                            /*
                                             * In the C and C++ programming languages, the comma operator (represented by the token ,) is a binary operator that evaluates its first operand and discards the result, and then evaluates the second operand and returns this value (and type). The comma operator has the lowest precedence of any C operator, and acts as a sequence point.
    The use of the comma token as an operator is distinct from its use in function calls and definitions, variable declarations, enum declarations, and similar constructs, where it acts as a separator.
    In this example, the differing behavior between the second and third lines is due to the comma operator having lower precedence than assignment.
    * http://en.wikipedia.org/wiki/Comma_operator */
    												/*
                                                    p=buf+4,//this ends up putting the value agter GOTO into p      what do single commas mean in C greg?
                                                    buf[2]=='S' && (*callp++=CurrentLineNumber, p++),//if GOSUB
                                                    CurrentLineNumber=eqexp()-1;
                                                    break;
                                                    */
                                                    /*
                                                    p=OneLine+4,//this ends up putting the value after GOTO into p,I think this is pointer arithmatic      what do single commas mean in C greg?
                                                    OneLine[2]=='S' && (*callp++=CurrentLineNumber, p++),//if GOSUB
                                                    CurrentLineNumber=eqexp()-1;
                                                    break;
                                                    */
                                                    if(OneLine[2]=='S') {//this is true is it is GoSub
                                                    	StackCount++;
                                                    	ReturnStack[StackCount]=CurrentLineNumber+1;
                                                    }
                                                    sscanf(OneLine,"%*s %i",&i);//puts line number into i
						    CurrentLineNumber=i-1;//must be -1 cause ther for loop will increase it to teh right number
                                                    break;
                                            case 'R':        // REM|RETURN
                                                    //OneLine[2]!='M' && (CurrentLineNumber=*--callp);
                                                    if(OneLine[2]!='M') {//if it it not ReM it is RETURN
                                                    	CurrentLineNumber=ReturnStack[StackCount]-1;//must be -1 cause the for loop will increase this number
                                                    	StackCount--;
                                                   	}
                                                    break;
                                            case 'F':        // FOR X=10 TO 15
                                                        char varname; int start; int end;
                                                        sscanf(OneLine,"FOR %c=%i TO %i",&varname,&start,&end);
                                                        //printf("FOR %c=%i TO %i\r\n",varname,start,end);
                                                        for_itteration[varname]=start;
                                                        forend[varname]=end;
                                                        forln[varname]=CurrentLineNumber;
                                                        var[varname]=start;
                                                        break;
                                            case 'N':        // NEXT
                                                        sscanf(OneLine,"NEXT %c",&varname);
                                                        if(for_itteration[varname]<forend[varname]) {//for loop has not ended
                                                                for_itteration[varname]++;//increment itteration number;
                                                                var[varname]=for_itteration[varname];
                                                                CurrentLineNumber=forln[varname];//goto beginning of for loop
                                                        }
                                                        break;
                                      }
                                      //CurrentLineNumber++;
                              } 
                     };
                      break;

                case 'D':        // DELETE current program
                	print2("DELET PROGRAM Y/N?");
					#ifdef __linux__
						 gets(buf);
					#else
						 gets2(buf);
					#endif
					if(buf[0]=='Y') {
	                	print2("DELETING...");                	
						delete_lns();
						delete_nol();
						print2("DONE.");
						NumberOfLines=0;
					}
					break;
			    case 'O':        // OPEN (program number)
					ProgNum=buf[5]-'0';
			        NumberOfLines=get_nol();//get the number of lines for this program
			        break;
                      case 'L':        // LIST
                      newline();
                      sprintf(buf,"#OF LINES:%i",NumberOfLines);
                      print2nl(buf);
                      print2nl("[ENTER] TO LIST...");
                      gets2(buf);//just wait for enter
                      for(CurrentLineNumber=0;CurrentLineNumber<LineCount;CurrentLineNumber++) {
			    get_Line2(CurrentLineNumber);
                            //strcpy(OneLine,"TEST");//this works, so the rpoblem of the eternal loop must have something to do with get_Line
                            if(strcmp(OneLine,"EMPTYLINE")!=0) {
                                  //char tmp[100];
                                  cls();
                                  sprintf(buf,"%i ",CurrentLineNumber);
                                  print2(buf);
                                  OneLine[strlen(OneLine)-1]='\0';//get rid of the last 2 characters which seems to be a new line or carrige return
                                  sprintf(buf,"%s",OneLine);
                                  print2nl(buf);
                                  if(CurrentLineNumber%7==0) {//the screen is full(show 7 lines per screen)
                                        print2nl("[ENTER] NEXT LINE");
                                        gets2(buf);//just wait for enter
                                  }
                            }
			}
                        cls();
                      break;
                      
                      case 'C':        // CREDITS
                      newline();
                      print2nl  ("-Usmar A Padow");
                      print2nl  ("BASIC,Project Manager");
                      print2nl  ("-Greg Cox");
                      print2nl  ("SD memory allocation");
                      print2nl  ("-Yotson, design of");
                      print2nl  ("Parallel protocol");
                      print2    ("-and others...");
                      //sorry doesnt fit in memory//print2nl  ("-GeDaMo");
                      //print2    ("Binary conversions");
                      //sorry doesnt fit in memory //print2nl  ("-Belxjander");
                      //print2    ("idea, using 2 arduinos");
                      break;

                      
                case 0:
                default:// labeled line 
                        //print2("edit line");
                        editln();
                }
        }
}



void printWelcome()
{
    print2nl("ARDUINO TvKbSd BASIC");
    print2nl("by Usmar Padow      ");
    print2  ("Type CREDITS");
    //print2  ("Yotson & GeDaMo");
}


//needed for the arudino simulator
#ifdef __linux__
	int main(void) {
			initArray();
			connect();
			setup();
			while(1){
					loop();
			}
	}
#endif
