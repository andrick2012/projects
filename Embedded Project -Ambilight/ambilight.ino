// This #include statement was automatically added by the Particle IDE.
#include <LiquidCrystal_I2C_Spark.h>

// This #include statement was automatically added by the Particle IDE.
#include <IRremote.h>

// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>
#include <math.h>
#include <IRremote.h>
#include <LiquidCrystal_I2C_Spark.h>

// Fast Fourier Transforms used in audio mode
#define FFT_FORWARD 1
#define FFT_REVERSE -1

// The 4 modes available 
#define OFF 0
#define VIDEO 1
#define AUDIO 2
#define AMBIENCE 3

// Pins used for input/output
const int irReceiverPin = A0; 
const int audioInput = A1;
const int ledPin = D6;
int test = 0;

// Set-up of TCP server to communicate with Photon
TCPServer server = TCPServer(23);
TCPClient client;

//Ir reciever for remote control
IRrecv irrecv(irReceiverPin); 
decode_results results;

// Modes possible 
int Mode;

// LCD display
LiquidCrystal_I2C *lcd;
String localIPString;

// LED strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, ledPin, WS2812B2);
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(12, ledPin, WS2812B2);



// FFT INITILIZATION =====
// set the sizes for the FFT
const int m = 8;
const int FFT_SIZE = pow(2, m);

// initialize the buffers for the FFT
float samples[FFT_SIZE * 2];
float imagine[FFT_SIZE * 2];
int sampleCounter = 0;

// Variables for audio mode
const int NUM_ZONES = 12;
const int STEP = int(FFT_SIZE/(2*NUM_ZONES));
float zoneVals[NUM_ZONES];


// flag that lets us know when we should or shouldn't fill the buffer
bool fillBuffer = false;


void setup() {
  Mode = OFF;   //initially switched off  
  pinMode(audioInput, INPUT); //input pin for audio through sound sensor

  //wifi for video mode data
  waitUntil(WiFi.ready);
  server.begin();

  Serial.begin(9600); //Serial for debugging

  //Initialize lcd display with Ip for tcp connection
  localIPString = WiFi.localIP();
  lcd = new LiquidCrystal_I2C(0x27, 16, 2);
  //Turns on the lcd screen
  lcd->init();
  //Turns on the backlight
  lcd->backlight();
  lcdDisplay();

  //Irreciver initialization
  irrecv.enableIRIn(); 

  //starting led strip
  strip.begin();
  strip.show();

  //for audio sampling
  startSampling();
}

//clear Strip switches off all the leds in the strip
void clearStrip()
{
    uint32_t c = strip.Color(0, 0, 0);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    strip.show();
    
}


// Change modes based on received IR code
bool irModeChanger()
{
    // Decode IR signal
    if (irrecv.decode(&results)) 
    {
        Serial.println(results.value, HEX);
        // Button pressed : Off
        if(results.value == 0xFFA25D){clearStrip(); Mode= OFF;}
        // Button pressed : 1
        else if(results.value == 0xFF30CF){ server.begin(); Mode = VIDEO;}
        // Button pressed : 2
        else if(results.value == 0xFF18E7) Mode = AUDIO;
        // Button pressed : 3
        else if(results.value == 0xFF7A85) Mode = AMBIENCE;
        // Button pressed : Any other button
        else if (Mode != VIDEO){
            clearStrip();
            Mode= OFF;
        }
        //display modes on lcd display
        lcdDisplay(); 
        irrecv.resume();
        return true;
        
    }
    return false;
    
}

//lcd Display displays modes and also tells the user if the python api is connected or not
//if not connected, it will display the ip for the user
void lcdDisplay()
{
    // Clear LCD display, place cursor at top-left corner, display IP address
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print(localIPString);
    // Set cursor at bottom left corner, display the corresponding mode
    lcd->setCursor(0,1);
    if(Mode == OFF)
    lcd->print("OFF");
    else if(Mode == AUDIO)
    lcd->print("Audio Mode");
    else if(Mode == VIDEO)
    lcd->print("Video Mode");
    else
    lcd->print("Ambience Mode");
}
//this method takes rgb values from the python api along with the index of led.
void getImageData()
{
    //variables for the message and RGB
    static String val = "";
    static int red = 0;
    static int green = 0;
    static int blue = 0;


    if (client.connected()){
       localIPString = "Connected";
        //Serial.println("Laptop Connected");
        while (client.available()) 
        {
            //segregate the message into characters
            char letter = (char)client.read();
            if(isalnum(letter))
            {
                if(isdigit(letter))
                {
                    //store digits as string first
                    if(val.length() == 3) val = "";
                    val+=letter;
                }
                else
                {
                    //change val to integer then assign it to either r, g or b depending on the next character
                    if(letter == 'r')
                    red = val.toInt();
                    else if(letter == 'g')
                    green = val.toInt();
                    else if(letter == 'b')
                    blue = val.toInt();
                    else if(letter == 'i')
                    {
                        //if letter i is recieved , val is used as an index for the led
                        //using the r,g and b values the led at location i is lit.
                        int i = val.toInt();
                        if(red>=0 && red<256 && blue >=0 && blue<256
                        && green>=0 && green<256 && i>= 0  && i< strip.numPixels())
                        {
                          strip.setPixelColor(i, strip.Color(red, green, blue)); 
                        }
                        red = 0;
                        green = 0;
                        blue = 0;
                        
                    }
                    //marks the end of a cycle, thats when all the written led values are displayed
                    else if(letter == 'x')
                    strip.show();
                    val = "";
                }
            }
        }
        lcdDisplay();
    } 
    else
       client = server.available();
}

//ambience mode is a simple method gradually increasing blue and decreasing red
// then cycle is reversed and it is repeated
void ambienceMode()
{
    static uint8_t r = 100;
    static uint8_t g = 0;
    static uint8_t b = 0;
    static int incr = -1;
    uint32_t c = strip.Color(r, g, b);
    for(uint16_t i=0; i<strip.numPixels(); i+=1) {
      strip.setPixelColor(i, c);      
      if(irModeChanger()) break;      
    }
    strip.show();
    if(r <= 0) incr = 1;
    if(b <= 0) incr = -1;
    r+=incr;
    b-=incr;
    delay(30);
}
//Audio mode samples sound sensor, does FFT on the sample
//map the frequency buckets to zones on led strip symmetric around the centre.

void audioMode() {
   
	// if we're currently filling the buffer
	if (fillBuffer == true) {
		// read from the input pin and start filling the buffer
		samples[sampleCounter] = analogRead(audioInput);
		// the buffer full of imaginary numbers for the FFT doesn't matter for our use, so fill it with 0s
		imagine[sampleCounter] = 0.0;
		// increment the counter so we can tell when the buffer is full
		sampleCounter++;
	}

	// if the buffer is full
	if (samplingIsDone()) {
	    Serial.print(test);
        Serial.print(" ");
        test += 1;
		// stop sampling
		stopSampling();
		// do the FFT
 		FFT(FFT_FORWARD, m, samples, imagine);
		// if the client is connected to the server
		int ind = 0;
        float sum = 0.0;
        float maximum = -1;
        
		for (int i = 1; i < FFT_SIZE/2; i ++) {
		    sum += abs(samples[i]*samples[i]) + abs(imagine[i]*imagine[i]);
		    if (i % STEP == 0){
		        float value = float(sum/STEP);
		        // Only use value that are above a given threshold 
		        // (avoids lighting LED with ambiance noise)
		        if (value >= 5000) zoneVals[ind] = value;
		        else zoneVals[ind] = 0;
		        
		        //Update the maximum to normalize LED brightness
		        if(zoneVals[ind] > maximum) maximum = zoneVals[ind];
		        ind += 1;
		        sum = 0.0;
		    }
		}
		
		for(uint16_t i=0; i<strip.numPixels(); i+=1) {
            strip.setPixelColor(i, generateLedSignalFromFrequency(i, maximum));
            if(irModeChanger()) break;
        }
        strip.show();
 
		// start sampling again
		startSampling();
	}
}


// EXTRA FUNCTIONS =====

// Color zones on the LED strip differently based on frequency detected
uint32_t generateLedSignalFromFrequency(int ledIndex, int maximum)
{
    if(ledIndex >= 74) ledIndex = 149-ledIndex;
    int zone = ledIndex/6;
    int r,g,b;
    if(zone == 0)
    {
        r = 255;
        g = 0;
        b = 0;
    }
    else if(zone == 1)
    {
        r = 255;
        g = 0;
        b = 0;
    }
    else if(zone == 2)
    {
        r = 128;
        g = 128;
        b = 0;
    }
    else if(zone == 3)
    {
        r = 0;
        g = 255;
        b = 0;
    }
    else if(zone == 4)
    {
        r = 0;
        g = 0;
        b = 255;   
    }
    else if(zone == 5)
    {
        r = 0;
        g = 127;
        b = 127;
    }
    else if(zone == 6)
    {
        r = 0;
        g = 0;
        b = 255;
    }
    else if(zone == 7)
    {
        r = 0;
        g = 127;
        b = 127;
    }
    else if(zone == 8)
    {
        r = 0;
        g = 255;
        b = 0;
    }
    else if(zone == 9)
    {
        r = 128;
        g = 128;
        b = 0;
    }
    else if(zone == 10)
    {
        r = 255;
        g = 64;
        b = 0;
    }
    else if(zone == 11)
    {
          r = 255;
        g = 0;
        b = 0;
    }
    else
    return strip.Color(0, 0, 0);
    
    r = (int) (zoneVals[zone]*r/maximum);
    g = (int) (zoneVals[zone]*g/maximum);
    b = (int) (zoneVals[zone]*b/maximum);
    //Serial.print("Color at :" + ledIndex);
   // Serial.println(" r "+ String(r) + " g " + String(g) + " b " +String(b));
    
    return strip.Color(r,g,b);
}

// Start sampling data from music
void startSampling() {
	sampleCounter = 0;
	fillBuffer = true;
}

// Stop sampling music data
void stopSampling() {
	fillBuffer = false;
}

// Once the FFT buffer
bool samplingIsDone() {
	return sampleCounter >= FFT_SIZE; // *2
}

short FFT(short int dir, int m, float *rx, float *iy) {
	
	/*
	FFT() from Paul Bourke: http://paulbourke.net/miscellaneous/dft/
	as referenced by @phec on the Particle forums: https://community.particle.io/t/fast-fourier-transform-library/3784/4
	This computes an in-place complex-to-complex FFT 
	rx and iy are the real and imaginary arrays of 2^m points.
	
	dir gives FFT_FORWARD or FFT_REVERSE transform
	rx is the array of real numbers on input, and the x coordinates on output
	iy is the array of imaginary numbers on input, and the y coordinates on output
	*/
	
	// \/ \/ \/ DO NOT EDIT THIS CODE UNLESS YOU KNOW WHAT YOU'RE DOING \/ \/ \/
	
	int n, i, i1, j, k, i2, l, l1, l2;
	float c1, c2, tx, ty, t1, t2, u1, u2, z;

	/* Calculate the number of points */
	n = 1;
	for (i=0;i<m;i++) 
		n *= 2;

	/* Do the bit reversal */
	i2 = n >> 1;
	j = 0;
	for (i=0;i<n-1;i++) {
		if (i < j) {
			tx = rx[i];
			ty = iy[i];
			rx[i] = rx[j];
			iy[i] = iy[j];
			rx[j] = tx;
			iy[j] = ty;
		}
		k = i2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Compute the FFT */
	c1 = -1.0; 
	c2 = 0.0;
	l2 = 1;
	for (l=0;l<m;l++) {
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0; 
		u2 = 0.0;
		for (j=0;j<l1;j++) {
			for (i=j;i<n;i+=l2) {
				i1 = i + l1;
				t1 = u1 * rx[i1] - u2 * iy[i1];
				t2 = u1 * iy[i1] + u2 * rx[i1];
				rx[i1] = rx[i] - t1; 
				iy[i1] = iy[i] - t2;
				rx[i] += t1;
				iy[i] += t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0 - c1) / 2.0);
		if (dir == 1) 
			c2 = -c2;
		c1 = sqrt((1.0 + c1) / 2.0);
	}

	/* Scaling for forward transform */
	if (dir == 1) {
		for (i=0;i<n;i++) {
			rx[i] /= n;
			iy[i] /= n;
		}
	}

	return(0);
}

//loop method calls the appropriate method based on the mode.
void loop() {
    irModeChanger();
    if(Mode == AMBIENCE)
     ambienceMode();
    if(Mode == VIDEO)
     getImageData();
    if(Mode == AUDIO)
     audioMode();
}