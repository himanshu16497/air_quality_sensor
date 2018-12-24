#define AQ 0; //AQ means acquision,it denoted data acquision
#define GPS 1; //this defines the block where data from the GPS device is collected.
#define SER 2; //this is the block where data is sent over to the serial terminal
#define IND 3; // this is the indicator block, that indicates that the data is being collected
#include <TinyGPS++.h>    //this library has been used to encode the GPS data received in intelligible form.
#include <SoftwareSerial.h>   //this library has been used to initiate serial ports at pins(4,3) and pins(10,11)
SoftwareSerial myserial(10,11);   //rx=10,tx=11,blue wire is the transmitter on the sensor part,yellow wire is the receiver
SoftwareSerial mygps(4,3);    //rx=4,tx=3
TinyGPSPlus gps;
  //different variable that have been used in the code are being initialised below.
bool flag=0;
int var=0;
float pm2=0,pm10=0;
double longi,lati;    //the GPS co-ordinates are required to be very precise.hence they are initialised as doubles.
void setup() {
flag=0;
Serial.begin(9600);    //baud rate for a serial port has been initialised here, device connected at the serial terminal must have the same baud rate.
pinMode(6,OUTPUT);     //the pin to which the buzzer is connected has been initialised at output here.
mygps.begin(38400);   //3DR gps used in the device works at 38400 baud rate.
myserial.begin(9600);  //this serial port is for the SDS021, which works at baud rate of 9600.
}

void loop() {
    switch(var)
    {
      //first of all we acquire the data from SDSO21, we store the data in the variabels PM10 and PM2.5,these varibales are modified by the function read_data.
      case 0:
        pm2=-1;
        pm10=-1; 
        myserial.listen();  //as software serial can only listen to one functional port at a time.(see the documentation for more detail)
        delay(100);
        data_read();
        if(pm2>0 & pm10>0)  //this is to make sure that the values returned by data_read() are not bogus(it happens some time)
        {
          var=GPS;    //if the values are legitimate then move to the next case
          }
        else
        {
          var=AQ;   //if values are bogus then call this function again
          }
        break; 
    case 1:
      mygps.listen();   //as software serial can only listen to one functional port at a time.
      while(mygps.available()>0 && flag==0)  //this flag is used to see if the gps.encode() function works the correct way.
             {
              if(gps.encode(mygps.read()))
              {  
                displayInfo();
              }
             }
              if(flag==1)   // if the flag is equal to one that means the function gps.encode() worked correctly, else we need to run it again
              var=2;
              else
              var=1;
              break;
    case 2:
      flag=0;
      Serial.println();   // all the data(pm2,pm10,longi,latti) that has been acquired is now being sent to serial port of the nano
      Serial.print(pm2);   //the data is being sent in a format that makes it easier to convert it into .csv file.
      Serial.print(",");    
      Serial.print(pm10);
      Serial.print(",");
      Serial.print(longi,7);
      Serial.print(",");
      Serial.print(lati,7);
      var=IND;
      break;
    case 3:
      digitalWrite(6,HIGH); // after the data has been sent, we beep the buzzer to let the person collecting data that know that the data is being collected 
      delay(10);    //if the buzzer does not beep then the person should try to move under clear sky and wait for 3-4 minutes, as the GPS some times gets a cold start.
      digitalWrite(6,LOW);    
      var=AQ;
      break;
  }
}

void data_read()
{
  byte data,arr[4];
  while(myserial.available()==0){}    //this to see off the phase when data transmitted is zero.
  if(myserial.available())    //now we check that the data is available for reading.
  {
    data=myserial.read();
    if(data==0xAA) // the first packet sent by the SDS021 is AA.
    {
      data=myserial.read();
      if(data==0xC0)  //second packet sent by the sensor is C0
      {
        data=myserial.read(); // then the lower byte of PM2.5 is sent, we store this in the 0 index of array arr
        arr[0]=data;
        data=myserial.read(); //then the higher byte of PM2.5 is sent, we store this in the 1 index of array arr
        arr[1]=data;
        data=myserial.read(); //then the lower byte of PM10 is sent, we store this in the 2 index of array arr
        arr[2]=data;
        data=myserial.read(); //then the higher byte of PM10 is sent, we store this in the 3 index of a array arr
        arr[3]=data;
        while(myserial.read()!=0xAB){} //the last byte sent by the sensor,this marks the end of the message
        int x=sizeof(arr);
        for(int i=0;i<x;i++) 
        {
        pm2=(arr[1]*256+arr[0])/10.0; //this conversion formula.
        pm10=(arr[3]*256+arr[2])/10.0;
        }
      }
    }
  }
}    
void displayInfo()
{ 
  if (gps.location.isValid())  //this checks if the location that has been encoded by gps.encode above is correct or not.
  {
    lati=gps.location.lat(); //if the location is found to be correct, then the lattitude is stored in the variable 'lati'
    longi=gps.location.lng();   //longitude is stored in the variable 'longi'
    flag=1;   //flag is set to 1 so that the program can move to the next block
  }
  else
  {
    flag=0; //if gps location is found to be invalid, the flag is set to 0 and the functon displayInfo() is called again.
  }
   
}
