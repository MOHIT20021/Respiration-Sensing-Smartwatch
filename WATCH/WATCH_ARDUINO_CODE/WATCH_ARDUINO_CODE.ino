
// please extract the library in the library folder for your arduino IDE as we have done certain changes in setup files of ST7789_t3 .so please use file provided by us
// other libraries can be downloaded from internet but are provided in the folder (except Wire.h and SPI.h)


#include "Adafruit_GFX.h"

#include <ST7789_t3.h>
#include <Wire.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "SPI.h"

// scl 19 
//sda  18

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air
float mlx90640To[768];
paramsMLX90640 mlx90640;







// / for display     //////////////////////////////////////////////////////////////////////////////////////////////
#define TFT_MISO  12
#define TFT_MOSI  11  //a12
#define TFT_SCK   13  //a13
#define TFT_DC   9 
#define TFT_CS   10  
#define TFT_RST  8



ST7789_t3 tft = ST7789_t3(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);
float T_max, T_min; 


boolean display1 = true;
double m1=0;       
double dx=75;     //// interval of x1 after which the graphs is redrawn  
double x1=m1;     /// x initialization for graphs

double ox , oy ;  // variable for graphing functions
int YLOW=-30;   // defining y lower limit
int YHIGH=30;   // defining y higher limit
int XLOW=0;     // defining x lower limit
int XHIGH=dx;    // defining x higher limit

int size=7;
int maxcol=32*size;
float Temps[32*8][24];      // defining temp array
float Low,High,Delta,TempC;  // some parameter


double avg=0;
int WINDOW=10;   // window_length
int MEAN_ARR_SIZE=0;   /// initilze with 0
double MEAN_ARR[10]={0}; // window size array



#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49

// /////////////////////////////////////////////     FUNCTIONS    /////////////////////
double Convolve(double* arr,int* size,int window,double *avg,double val){
    if((*size)<window){
        arr[*size]=val;
        (*avg)=( (*avg) * (*size)+ val)/((*size)+1);
        (*size)++;
        Serial.print("  if   AVG COLVOLNE ");
        Serial.print((*avg));
        Serial.print(" SIZE ");
        Serial.print((*size));
    }
    else {
        double temp=arr[(*size)%window];
        arr[(*size)%window]=val;
        (*avg)=(*avg)-(temp/window)+(val/window);
        (*size)++;
        Serial.print("  else AVG COLVOLNE ");
        Serial.print((*avg));
        Serial.print(" SIZE ");
        Serial.print((*size));
    }

    return *avg;
}

// Convert a grayscale value to a 16-bit RGB565 color
uint16_t grayscaleToRGB565(int grayscale) {
    // Ensure grayscale value is within the range 0-255
    grayscale = constrain(grayscale, 0, 255);
    // Convert to 5-bit red, 6-bit green, and 5-bit blue
    uint16_t red = (grayscale >> 3) & 0x1F;
    uint16_t green = (grayscale >> 2) & 0x3F;
    uint16_t blue = (grayscale >> 3) & 0x1F;
    // Combine into a 16-bit RGB565 color
    return (red << 11) | (green << 5) | blue;
}

boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}



void setup() {


    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(400000); // Increase I2C clock speed to 400kHz
    if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

///// Setting refresh rate
  MLX90640_SetRefreshRate(MLX90640_address, 0x06); //Set rate to 16Hz  
  Wire.setClock(1000000);




    /// SETUP FOR DISPLAY
    tft.init(240,320);
    tft.setRotation(2);
    tft.fillScreen(0x0000); // Black background   



    /// smothing  

} 

void loop() {

  // Collecting the array from camera in mlx90640To
    long startTime = millis();
    for (byte x = 0 ; x < 2 ; x++)
    {
      uint16_t mlx90640Frame[834];
      int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);

      float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
      float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

      float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
      float emissivity = 0.95;

      MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
    }



////////////////// Display ////////////////////////////////////

    T_min = 30;     // manually defining the min temperature 
    T_max = 40;      // manually degining yhe max temperature

    int k=0;         
    double mean=0;
    int Blength=50;    /// length pf box ( in x axis direction)
    int Bwidth=40;     // length of box (int -y axis direction)
    // float alpha=-1.3; 
    // float beta=95;
    int x, y;


int yoffset=0;
int xoffset=0;
int IMG_WIDTH=32*size; // x axis
int IMG_HEIGHT=24*size; // y axis


for (x = 0; x < IMG_WIDTH; x++) {
    for (y = 0; y < IMG_HEIGHT; y++) {
        int x0 = (x) / 10;
        int y0 = (y) / 10;
        int x1 = constrain(x0 + 1, 0, maxcol - 1);
        int y1 = constrain(y0 + 1, 0, 23);

        // Directly fetch Low and High temperatures for x direction
        float Low_x0 = mlx90640To[(x0) + (32 * y0)];
        float High_x0 = mlx90640To[(x1) + (32 * y0)];
        float Low_x1 = mlx90640To[(x0) + (32 * y1)];
        float High_x1 = mlx90640To[(x1) + (32 * y1)];

        float Delta_x0 = (High_x0 - Low_x0) / 10.0;
        float Delta_x1 = (High_x1 - Low_x1) / 10.0;
        float TempC_x0 = Low_x0 + (Delta_x0 * (x % 10));
        float TempC_x1 = Low_x1 + (Delta_x1 * (x % 10));

        // Interpolate in y direction
        float Delta_y = (TempC_x1 - TempC_x0) / 10.0;
        TempC = TempC_x0 + (Delta_y * (y % 10));

        float grayscale =255-map(TempC, T_min, T_max, 150, 350);

        if (x>((IMG_WIDTH)/2-Blength/2) && y>((IMG_HEIGHT)/2-Bwidth/2)  &&  x<((IMG_WIDTH)/2+Blength/2) && y<((IMG_HEIGHT)/2+Bwidth/2)  ){
              mean=mean+(grayscale/(Blength*Bwidth));              
            }
        uint16_t color = grayscaleToRGB565(grayscale); // Convert to RGB565

        // Draw pixel
        // tft.drawPixel(IMG_WIDTH-x+5,IMG_HEIGHT-y+5, color);
        tft.drawPixel(x+5,y+5, color);

        // delay();
    }
     
                  // x        , y
     tft.drawRect((IMG_WIDTH)/2-Blength/2+5,(IMG_HEIGHT)/2-Bwidth/2+5, Blength, Bwidth,RED );

}






//////////       graph plotting  ///////////////////////////////////////////
    Serial.print(" Mean ");
    Serial.print(mean);
    avg=Convolve(MEAN_ARR,&MEAN_ARR_SIZE,WINDOW,&avg,mean);
    Serial.print(" avg ");
    Serial.print(avg);
    Serial.println( " ");

    mean=avg;
    if (mean>YHIGH){
      mean=YHIGH;

    }
    if (mean<YLOW){
      mean=YLOW;
      
    }

    // this is just printing random number as a placeholder for respiration rate.
    if (mean>YLOW && mean<YHIGH){
      tft.setTextSize(5);
      tft.setTextColor(GREEN,GREEN);
      tft.setCursor((IMG_WIDTH)/2-Blength/2+10, 5);
      tft.println(5);

    }
    
    Graph(tft, x1, mean, 5, 300, 230, 120, XLOW, XHIGH, 1, YLOW, YHIGH, 0.5, "", "", "", DKBLUE, RED, YELLOW, WHITE, BLACK, display1);


    // Serial.print(x);
    x1=x1+1;

if (x1>m1+dx){

m1=m1+dx;
x1=m1;
tft.fillRect(0,0,tft.width(),yoffset/2,BLACK);
tft.fillRect(0,0,xoffset/2,IMG_HEIGHT,BLACK);
tft.fillRect(IMG_WIDTH-4,0,xoffset,IMG_HEIGHT,BLACK);
tft.fillRect(0,IMG_HEIGHT,tft.width(),tft.height()-IMG_HEIGHT,BLACK);


}

  // delay(10);

}




void Graph(ST7789_t3 &d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor, boolean &redraw) {

  double ydiv, xdiv;

  double i;
  double temp;
  int rot, newrot;

  if (redraw == true) {

    redraw = false;
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      if (i == 0) {
        d.drawLine(gx, temp, gx + w, temp, acolor);  // y axis line 
      }
      else {
        d.drawLine(gx, temp, gx + w, temp, gcolor);
      }

      d.setTextSize(1);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(gx - 40, temp);
      // precision is default Arduino--this could really use some format control
      // d.println(i);  // print yticks
    }
    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {

      // compute the transform

      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        d.drawLine(temp, gy, temp, gy - h, acolor);  // x axis line 
      }
      else {
        d.drawLine(temp, gy, temp, gy - h, gcolor);
      }

      // d.setTextSize(1);
      // d.setTextColor(tcolor, bcolor);
      // d.setCursor(temp, gy + 10);
      // precision is default Arduino--this could really use some format control
      // d.println(i);  // print xticks 
    }

    //now draw the labels
    d.setTextSize(2);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(gx , gy - h - 30);
    d.println(title);

    d.setTextSize(1);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx , gy + 20);
    d.println(xlabel);

    d.setTextSize(1);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx - 30, gy - h - 10);
    d.println(ylabel);


  }

  //graph drawn now plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized as static above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  d.drawLine(ox, oy, x, y, pcolor);
  d.drawLine(ox, oy + 1, x, y + 1, pcolor);
  d.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;

  if (x>=w+gx){
    int delta=xhi-xlo;
    XLOW=xhi;   // re initilize the lower limit and upper limit of x axis
    XHIGH=xhi+delta;
    redraw=true;
    // d.fillScreen(BLACK);
    
  }

}

/*
  End of graphing functioin
*/

