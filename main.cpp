// WMG aka Wearable Medical Gaurdian by Milan Kkharel. 
// Cortex-M3 Processor - ARM is main heart of the device 
// Perefirals used: BMP_180 Temperature and Pressure sensor
//                  Adafruit EZ-link BT module 
//                  Amped Pulse Sensor
//                  4-Digit 7 segment display 
//                  Buzzer , Pull-Up push button                      

#include "mbed.h"
#include "BMP180.h"
#include "PulseSensor.h"
#include "DigitDisplay.h"
#include "beep.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#define PIN_SDA p9  // BPM_180 DataAvailable
#define PIN_SCL p10 // clock for BPM180

// fadeout effect constants macros
#define PWM_STEPS   35
#define PWM_SPEED   50
#define LED_ON      1
#define LED_OFF     0

// pins
Beep buzzer (p21);
DigitDisplay display(p27, p28);
Serial pc(USBTX, USBRX);
Serial bt(p13,p14);
InterruptIn pb(p8);
DigitalOut led1 (LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);


//global variables and pointers
PulseSensor* pSensor;
int bpm = 0;   // current reading for bpm
bool received = false;
bool set = false;
float temperature;
int minimum,maximum,counter;
string temp,pres,puls,sent,bpm_Min, bpm_Max, city;
string SOS;  // push button
ostringstream stream_t,stream_bpm, stream_p; // float to string vars
int heat_freq=30, bpm_freq=130;  // converted values from the string
const unsigned char msg_set[] = { 0x00, 0x00, 0x00, 0x06d, 0x79, 0x78,0x3e, 0x73, 0x00, 0x00,0x00};
const unsigned char msg_act[] = { 0x77, 0x39, 0x78, 0x30, 0x1e, 0x79, 0x00, 0x00, 0x00 };
const unsigned char msg_wait[] = { 0x00, 0x01,0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x3f, 0x40, 0xff, 0x00 };
int active = 0, flag_heat=0, flag_bpm = 0;  // triggers
int city_index =8;  // North bay by default
int temps[] = { 24,25,26,26,27,28,32,34,30,26,27,28,28 };


// ===================FUNCTIONs======================
void heat_alert (void)
{
    wait(0.5);
        if(temperature > temps[city_index]*0.1) {
            buzzer.beep (heat_freq, 0.25);
            flag_heat = 1;
            }
    wait(0.5);
    }
    
void blink_led(int from, int to)
{
    for (int i = 0; i < PWM_STEPS; i++) {
        led1 = from;
        led2=from;
        led3=from;
        led4=from;
        wait_us(i * PWM_SPEED);

        led1 = to;
        led2 = to;
        led3 = to;
        led4 = to;
        wait_us((PWM_STEPS - i) * PWM_SPEED);
    }
}
void sos(void)
{
    if ((flag_heat == 1) and (heat_freq>0)){
        heat_freq=0;
        }
    else if((flag_bpm == 1)and(bpm_freq>0)){
        bpm_freq=0;
        }
    else
       SOS="1"; 
}
void sendDataToProcessing(char symbol, int data)
{
    //regestiring and collecting data in real-time
    if(symbol == 'B') {
        bpm = data;
        //pc.printf("%c %d\r\n", symbol, data);
        //pc.printf("bpm = %d \r\n S= %6.3f \r\n Q= %6.3f \r\n", bpm , S ,Q );

        //  4 leds on the beep
        //blink_led(LED_OFF, LED_ON);
        //blink_led(LED_ON, LED_OFF);
    }
// BT <=
    while(bt.readable()) {
        received = true;
        sent += bt.getc();
    }
     if(sent.length() == 2 ) {
            heat_freq = atoi(sent.c_str());
            if(heat_freq == 10)
                heat_freq=0;
            buzzer.beep(heat_freq,0.5);
            sent.assign("");
            pc.printf("heat");
            
        }
        if(sent.length() == 3) {
            bpm_freq = atoi(sent.c_str());
            buzzer.beep(bpm_freq,0.5);
            sent.assign("");
            pc.printf("bpm");
        }
}

int main()
{
    pb.mode(PullUp);
    pb.rise(&sos);
    pSensor = new PulseSensor(p20, sendDataToProcessing , 20);
    pSensor->start();  //global pointer is used

    BMP180 bmp180(PIN_SDA, PIN_SCL);
    float pressure;
    // float temperature;
    bmp180.Initialize(32, BMP180_OSS_ULTRA_LOW_POWER); // 64m altitude compensation and low power oversampling
    display.setBrightness(2);

    while(1) {
        if (bmp180.ReadData(&temperature, &pressure))
            //if(temperature>30 && temperature <35) {
            //pc.printf("Temp %6.2f \r\n", temperature);
            //  }
            //pc.printf("temp : %6.2f",temperature);
            //pc.printf("printin  %d",bt.getc());
            //bt.printf("Pressure(hPa): %6.2f \t   Temperature(C): %6.2f \t Pulse(BPM): %d \r\n", pressure, temperature, bpm);//,S,Q);
            //bt.printf("Pressure(hPa): %6.2f \t   Temperature(C): %6.2f \t Pulse(BPM): %d \r\n", pressure, temperature, bpm);//,S,Q);
            //pc.printf("bpm = %d \r\n S= %6.3f \r\n Q= %6.3f \r\n", bpm , S ,Q );

            //if(bpm>100)
            //    bt.printf("xx");

            // if(bpm<=100)
            //    bt.printf("ok");

            // if ( !checked && received  ) {
            //        checked = true;
            //        }
            // pc.printf("length of temp %d value %s / is at %d \n\r", temp.length(), temp.c_str(),comma);

            // pc.printf("min %s  and max is %s \r\n",restMin.c_str(),restMax.c_str());

// Check recieved data
        if(sent.length() == 1) {
            printf("mode %s \r\n",sent.c_str());
            active = atoi(sent.c_str());
            sent.assign("");

        }

        if(sent.length() > 6) {
            size_t comma = sent.find(",");
            size_t dot = sent.find(".");
            city.assign(sent.begin(),comma);
            bpm_Min.assign(sent.begin()+comma+1,sent.begin()+dot);
            bpm_Max.assign(sent.begin()+dot+1,sent.end());
            minimum = atoi(bpm_Min.c_str());
            maximum = atoi(bpm_Max.c_str());
            city_index = atoi(city.c_str());
            sent.assign("");
            pc.printf("recieved: min %s and max is %s city index = %s\r\n",bpm_Min.c_str(),bpm_Max.c_str(),city.c_str());
        }
    
// IF SET !    
    if(minimum> 0)
    {
//when resting
        if(active == 0) {
            counter++;
            if (counter < 12) {
                display.writeRaw(0,msg_wait[counter]);
                display.writeRaw(1,msg_wait[counter]);
                display.writeRaw(2,msg_wait[counter]);
                display.writeRaw(3,msg_wait[counter]);
                heat_alert();
                }
    // reached the count, monitoring mode         
            else 
            { 
                // displaing on 4digit 7 seg
                if (bpm<100) {
                    display.write(0,0xff);
                    display.write(1,0xc0);
                    display.write(2,bpm / 10);
                    display.write(3,bpm % 10);
                }
                if(bpm>=100) {
                    display.write(0,0xc0);
                    display.write(1,bpm/100);
                    display.write(2,(bpm/10)%10);
                    display.write(3,bpm % 10);
                }
                
                if( ((0.85*minimum) > bpm) || ((1.15*maximum) < bpm)) {
                    buzzer.beep(bpm_freq,0.25);
                        flag_bpm = 1;
                }
                heat_alert();
            }
     //     
 }
 // Is active
        else if ( active == 1) 
            {
            counter =0;
            set=true; 
                display.writeRaw(0,msg_act[0]);
                display.writeRaw(1,msg_act[1]);
                display.writeRaw(2,msg_act[2]);
                display.writeRaw(3,msg_act[3]);
                
                heat_alert();
            }
//HEAT ALERT
            
   }/*end of received*/
   
// Please set-up message if BPM is not received 
        else{
                for (int at=0; at < 8; at++) 
                    {
                    display.writeRaw(0,msg_set[at]);
                    display.writeRaw(1,msg_set[at+1]);
                    display.writeRaw(2,msg_set[at+2]);
                    display.writeRaw(3,msg_set[at+3]);
                    wait_ms(140);
                    }   
                    //total wait time is 1040 ms
            }
                stream_t << fixed << setprecision(1) << temperature;
                temp = stream_t.str();

                stream_bpm << fixed << setprecision(2) << bpm;
                puls = stream_bpm.str();
                if (puls.length() == 2 )
                    puls.insert(0,"0");
                if(puls.length() == 1)
                    puls.insert(0,"00");

                stream_p << fixed << setprecision(1) << pressure;
                pres = stream_p.str();
            //BT =>
                if( pres.length() == 6)
                    bt.printf("%s,%s,%s,%s",puls,temp,pres,SOS);
                else
                    bt.printf("%s,%s,%s0,%s",puls,temp,pres,SOS);
                    
                pc.printf("%s,%s,%s,%s,sent: %d \n\r",puls,temp,pres,SOS,pres.length());
            
                stream_t.str("");
                stream_bpm.str("");
                stream_p.str("");
                SOS="0";   
    }
}
