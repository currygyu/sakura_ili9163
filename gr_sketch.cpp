/*GR-SAKURA Sketch Template Version: V1.06*/
#include <rxduino.h>
#include <sdmmc.h>

#include <../font5x8.h>

extern const unsigned char font5x8[][6];

// SD card
SDMMC mySD;


// Translates a 3 byte RGB value into a 2 byte value for the LCD (values should be 0-31)
unsigned short decodeRgbValue(unsigned char r, unsigned char g, unsigned char b)
{
    return (b << 11) | (g << 6) | (r);
}   

// This routine takes a row number from 0 to 20 and
// returns the x coordinate on the screen (0-127) to make
// it easy to place text
unsigned char lcdTextX(unsigned char x) { return x*6; }

// This routine takes a column number from 0 to 20 and
// returns the y coordinate on the screen (0-127) to make
// it easy to place text
unsigned char lcdTextY(unsigned char y) { return y*8; }

#define INTERVAL 100

#define LCD_RESET  8
#define LCD_RS     9
#define LCD_WR     10
#define LCD_RD     11
#define LCD_CS     12

// Screen orientation defines:
// 0 = Ribbon at top
// 1 = Ribbon at left
// 2 = Ribbon at right
// 3 = Ribbon at bottom
#define LCD_ORIENTATION0    0
#define LCD_ORIENTATION1    96
#define LCD_ORIENTATION2    160
#define LCD_ORIENTATION3    192

// ILI9163 LCD Controller Commands
#define NOP                     0x00
#define SOFT_RESET              0x01
#define GET_RED_CHANNEL         0x06
#define GET_GREEN_CHANNEL       0x07
#define GET_BLUE_CHANNEL        0x08
#define GET_PIXEL_FORMAT        0x0C
#define GET_POWER_MODE          0x0A
#define GET_ADDRESS_MODE        0x0B
#define GET_DISPLAY_MODE        0x0D
#define GET_SIGNAL_MODE         0x0E
#define GET_DIAGNOSTIC_RESULT   0x0F
#define ENTER_SLEEP_MODE        0x10
#define EXIT_SLEEP_MODE         0x11
#define ENTER_PARTIAL_MODE      0x12
#define ENTER_NORMAL_MODE       0x13
#define EXIT_INVERT_MODE        0x20
#define ENTER_INVERT_MODE       0x21
#define SET_GAMMA_CURVE         0x26
#define SET_DISPLAY_OFF         0x28
#define SET_DISPLAY_ON          0x29
#define SET_COLUMN_ADDRESS      0x2A
#define SET_PAGE_ADDRESS        0x2B
#define WRITE_MEMORY_START      0x2C
#define WRITE_LUT               0x2D
#define READ_MEMORY_START       0x2E
#define SET_PARTIAL_AREA        0x30
#define SET_SCROLL_AREA         0x33
#define SET_TEAR_OFF            0x34
#define SET_TEAR_ON             0x35
#define SET_ADDRESS_MODE        0x36
#define SET_SCROLL_START        0X37
#define EXIT_IDLE_MODE          0x38
#define ENTER_IDLE_MODE         0x39
#define SET_PIXEL_FORMAT        0x3A
#define WRITE_MEMORY_CONTINUE   0x3C
#define READ_MEMORY_CONTINUE    0x3E
#define SET_TEAR_SCANLINE       0x44
#define GET_SCANLINE            0x45
#define READ_ID1                0xDA
#define READ_ID2                0xDB
#define READ_ID3                0xDC
#define FRAME_RATE_CONTROL1     0xB1
#define FRAME_RATE_CONTROL2     0xB2
#define FRAME_RATE_CONTROL3     0xB3
#define DISPLAY_INVERSION       0xB4
#define SOURCE_DRIVER_DIRECTION 0xB7
#define GATE_DRIVER_DIRECTION   0xB8
#define POWER_CONTROL1          0xC0
#define POWER_CONTROL2          0xC1
#define POWER_CONTROL3          0xC2
#define POWER_CONTROL4          0xC3
#define POWER_CONTROL5          0xC4
#define VCOM_CONTROL1           0xC5
#define VCOM_CONTROL2           0xC6
#define VCOM_OFFSET_CONTROL     0xC7
#define WRITE_ID4_VALUE         0xD3
#define NV_MEMORY_FUNCTION1     0xD7
#define NV_MEMORY_FUNCTION2     0xDE
#define POSITIVE_GAMMA_CORRECT  0xE0
#define NEGATIVE_GAMMA_CORRECT  0xE1
#define GAM_R_SEL               0xF2

void lcdSetDataBus(unsigned char data)
{
    int i;
    
    for( i=0; i<8; i++ ){
        digitalWrite(i, (data & 0x01));
        data >>= 1;
    }
}

void lcdReset(void)
{
    digitalWrite(LCD_RESET, 0);
    delay(50);
    digitalWrite(LCD_RESET, 1);
    delay(120);
}    

void lcdWriteCommand(unsigned char address)
{
    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_RS, 0);
    digitalWrite(LCD_CS, 0);

    lcdSetDataBus(address);
    
    digitalWrite(LCD_WR, 0);
    
    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_WR, 1);
    digitalWrite(LCD_CS, 1);
}

void lcdWriteParameter(unsigned char parameter)
{
    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_RS, 1);
    digitalWrite(LCD_CS, 0);

    lcdSetDataBus(parameter);

    digitalWrite(LCD_WR, 0);

    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_WR, 1);
    digitalWrite(LCD_CS, 1);
}

void lcdWriteData(unsigned char dataByte1, unsigned char dataByte2)
{
    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_RS, 1);
    digitalWrite(LCD_CS, 0);
    
    lcdSetDataBus(dataByte1);
    
    digitalWrite(LCD_WR, 0);

    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_WR, 1);

    lcdSetDataBus(dataByte2);

    digitalWrite(LCD_WR, 0);

    digitalWrite(LCD_RD, 1);
    digitalWrite(LCD_WR, 1);
    digitalWrite(LCD_CS, 1);
}

void lcdInitialise(unsigned char orientation)
{
    int i;
    
    for( i=0; i<8; i++){
        pinMode(i,OUTPUT);
        delay(5);
        digitalWrite(i, 0);
    }

    for( i=LCD_RESET; i<=LCD_CS; i++){
        pinMode(i,OUTPUT);
        delay(5);
        digitalWrite(i, 0);
    }

    lcdReset();
    
    lcdWriteCommand(EXIT_SLEEP_MODE);
    delay(5);
    
    lcdWriteCommand(SET_PIXEL_FORMAT);
    lcdWriteParameter(0x05); // 16 bits per pixel
   
    lcdWriteCommand(SET_GAMMA_CURVE);
    lcdWriteParameter(0x04); // Select gamma curve 3
    
    lcdWriteCommand(GAM_R_SEL);
    lcdWriteParameter(0x01); // Gamma adjustment enabled
    
    lcdWriteCommand(POSITIVE_GAMMA_CORRECT);
    lcdWriteParameter(0x3f); // 1st Parameter
    lcdWriteParameter(0x25); // 2nd Parameter
    lcdWriteParameter(0x1c); // 3rd Parameter
    lcdWriteParameter(0x1e); // 4th Parameter
    lcdWriteParameter(0x20); // 5th Parameter
    lcdWriteParameter(0x12); // 6th Parameter
    lcdWriteParameter(0x2a); // 7th Parameter
    lcdWriteParameter(0x90); // 8th Parameter
    lcdWriteParameter(0x24); // 9th Parameter
    lcdWriteParameter(0x11); // 10th Parameter
    lcdWriteParameter(0x00); // 11th Parameter
    lcdWriteParameter(0x00); // 12th Parameter
    lcdWriteParameter(0x00); // 13th Parameter
    lcdWriteParameter(0x00); // 14th Parameter
    lcdWriteParameter(0x00); // 15th Parameter
     
    lcdWriteCommand(NEGATIVE_GAMMA_CORRECT);
    lcdWriteParameter(0x20); // 1st Parameter
    lcdWriteParameter(0x20); // 2nd Parameter
    lcdWriteParameter(0x20); // 3rd Parameter
    lcdWriteParameter(0x20); // 4th Parameter
    lcdWriteParameter(0x05); // 5th Parameter
    lcdWriteParameter(0x00); // 6th Parameter
    lcdWriteParameter(0x15); // 7th Parameter
    lcdWriteParameter(0xa7); // 8th Parameter
    lcdWriteParameter(0x3d); // 9th Parameter
    lcdWriteParameter(0x18); // 10th Parameter
    lcdWriteParameter(0x25); // 11th Parameter
    lcdWriteParameter(0x2a); // 12th Parameter
    lcdWriteParameter(0x2b); // 13th Parameter
    lcdWriteParameter(0x2b); // 14th Parameter
    lcdWriteParameter(0x3a); // 15th Parameter
    
    lcdWriteCommand(FRAME_RATE_CONTROL1);
    lcdWriteParameter(0x08); // DIVA = 8
    lcdWriteParameter(0x08); // VPA = 8
    
    lcdWriteCommand(DISPLAY_INVERSION);
    lcdWriteParameter(0x07); // NLA = 1, NLB = 1, NLC = 1 (all on Frame Inversion)
   
    lcdWriteCommand(POWER_CONTROL1);
    lcdWriteParameter(0x0a); // VRH = 10:  GVDD = 4.30
    lcdWriteParameter(0x02); // VC = 2: VCI1 = 2.65
      
    lcdWriteCommand(POWER_CONTROL2);
    lcdWriteParameter(0x02); // BT = 2: AVDD = 2xVCI1, VCL = -1xVCI1, VGH = 5xVCI1, VGL = -2xVCI1

    lcdWriteCommand(VCOM_CONTROL1);
    lcdWriteParameter(0x50); // VMH = 80: VCOMH voltage = 4.5
    lcdWriteParameter(0x5b); // VML = 91: VCOML voltage = -0.225
    
    lcdWriteCommand(VCOM_OFFSET_CONTROL);
    lcdWriteParameter(0x40); // nVM = 0, VMF = 64: VCOMH output = VMH, VCOML output = VML   
    
    lcdWriteCommand(SET_COLUMN_ADDRESS);
    lcdWriteParameter(0x00); // XSH
    lcdWriteParameter(0x00); // XSL
    lcdWriteParameter(0x00); // XEH
    lcdWriteParameter(0x7f); // XEL (128 pixels x)
   
    lcdWriteCommand(SET_PAGE_ADDRESS);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7f); // 128 pixels y
    
    // Select display orientation
    lcdWriteCommand(SET_ADDRESS_MODE);
    lcdWriteParameter(orientation);

    // Set the display to on
    lcdWriteCommand(SET_DISPLAY_ON);
    lcdWriteCommand(WRITE_MEMORY_START);
}

void lcdClearDisplay(unsigned short colour)
{
    unsigned short pixel;
  
    // Set the column address to 0-127
    lcdWriteCommand(SET_COLUMN_ADDRESS);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7f);

    // Set the page address to 0-127
    lcdWriteCommand(SET_PAGE_ADDRESS);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7f);
  
    // Plot the pixels
    lcdWriteCommand(WRITE_MEMORY_START);
    for(pixel = 0; pixel < 16385; pixel++){
       lcdWriteData(colour >> 8, colour);
    }
}

void lcdPlot(unsigned char x, unsigned char y, unsigned short colour)
{
    // Horizontal Address Start Position
    lcdWriteCommand(SET_COLUMN_ADDRESS);
    lcdWriteParameter(0x00);
    lcdWriteParameter(x);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7f);
  
    // Vertical Address end Position
    lcdWriteCommand(SET_PAGE_ADDRESS);
    lcdWriteParameter(0x00);
    lcdWriteParameter(y);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7f);

    // Plot the point
    lcdWriteCommand(WRITE_MEMORY_START);
    lcdWriteData(colour >> 8, colour);
}

// Draw a line from x0, y0 to x1, y1
// Note:    This is a version of Bresenham's line drawing algorithm
//          It only draws lines from left to right!
void lcdLine(short x0, short y0, short x1, short y1, unsigned short colour)
{
    short dy = y1 - y0;
    short dx = x1 - x0;
    short stepx, stepy;

    if (dy < 0)
    {
        dy = -dy; stepy = -1; 
    }
    else stepy = 1; 

    if (dx < 0)
    {
        dx = -dx; stepx = -1; 
    }
    else stepx = 1; 

    dy <<= 1;                           // dy is now 2*dy
    dx <<= 1;                           // dx is now 2*dx
 
    lcdPlot(x0, y0, colour);

    if (dx > dy) {
        int fraction = dy - (dx >> 1);  // same as 2*dy - dx
        while (x0 != x1)
        {
            if (fraction >= 0)
            {
                y0 += stepy;
                fraction -= dx;         // same as fraction -= 2*dx
            }

            x0 += stepx;
            fraction += dy;                 // same as fraction -= 2*dy
            lcdPlot(x0, y0, colour);
        }
    }
    else
    {
        int fraction = dx - (dy >> 1);
        while (y0 != y1)
        {
            if (fraction >= 0)
            {
                x0 += stepx;
                fraction -= dy;
            }

            y0 += stepy;
            fraction += dx;
            lcdPlot(x0, y0, colour);
        }
    }
}

// Draw a rectangle between x0, y0 and x1, y1
void lcdRectangle(short x0, short y0, short x1, short y1, unsigned short colour)
{
    lcdLine(x0, y0, x0, y1, colour);
    lcdLine(x0, y1, x1, y1, colour);
    lcdLine(x1, y0, x1, y1, colour);
    lcdLine(x0, y0, x1, y0, colour);
}

// Draw a filled rectangle
// Note:    y1 must be greater than y0  and x1 must be greater than x0
//          for this to work
void lcdFilledRectangle(short x0, short y0, short x1, short y1, unsigned short colour)
{
    unsigned short pixels;
            
    // To speed up plotting we define a x window with the width of the 
    // rectangle and then just output the required number of bytes to
    // fill down to the end point
    
    lcdWriteCommand(SET_COLUMN_ADDRESS); // Horizontal Address Start Position
    lcdWriteParameter(0x00);
    lcdWriteParameter(x0);
    lcdWriteParameter(0x00);
    lcdWriteParameter(x1);
  
    lcdWriteCommand(SET_PAGE_ADDRESS); // Vertical Address end Position
    lcdWriteParameter(0x00);
    lcdWriteParameter(y0);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7F);
        
    lcdWriteCommand(WRITE_MEMORY_START);
    
    for (pixels = 0; pixels < (((x1 - x0)+1) * ((y1 - y0)+1)); pixels++)
        lcdWriteData(colour >> 8, colour);
}

// Draw a circle
// Note:    This is another version of Bresenham's line drawing algorithm.
//          There's plenty of documentation on the web if you are curious
//          how this works.
void lcdCircle(short xCentre, short yCentre, short radius, unsigned short colour)
{
    short x = 0, y = radius;
    short d = 3 - (2 * radius);
 
    while(x <= y)
    {
        lcdPlot(xCentre + x, yCentre + y, colour);
        lcdPlot(xCentre + y, yCentre + x, colour);
        lcdPlot(xCentre - x, yCentre + y, colour);
        lcdPlot(xCentre + y, yCentre - x, colour);
        lcdPlot(xCentre - x, yCentre - y, colour);
        lcdPlot(xCentre - y, yCentre - x, colour);
        lcdPlot(xCentre + x, yCentre - y, colour);
        lcdPlot(xCentre - y, yCentre + x, colour);

        if (d < 0) d += (4 * x) + 6;
        else
        {
            d += (4 * (x - y)) + 10;
            y -= 1;
        }

        x++;
    }
}

// LCD text manipulation functions --------------------------------------------------------------------------

// Plot a character at the specified x, y co-ordinates (top left hand corner of character)
void lcdPutCh(unsigned char character, unsigned char x, unsigned char y, unsigned short fgColour, unsigned short bgColour)
{
    unsigned char row, column;
    
    // To speed up plotting we define a x window of 6 pixels and then
    // write out one row at a time.  This means the LCD will correctly
    // update the memory pointer saving us a good few bytes
    
    lcdWriteCommand(SET_COLUMN_ADDRESS); // Horizontal Address Start Position
    lcdWriteParameter(0x00);
    lcdWriteParameter(x);
    lcdWriteParameter(0x00);
    lcdWriteParameter(x+5);
  
    lcdWriteCommand(SET_PAGE_ADDRESS); // Vertical Address end Position
    lcdWriteParameter(0x00);
    lcdWriteParameter(y);
    lcdWriteParameter(0x00);
    lcdWriteParameter(0x7f);
        
    lcdWriteCommand(WRITE_MEMORY_START);
    
    // Plot the font data
    for (row = 0; row < 8; row++)
    {
        for (column = 0; column < 6; column++)
        {
            if (font5x8[character][column] & (1 << row))
                lcdWriteData(fgColour >> 8, fgColour);
            else lcdWriteData(bgColour >> 8, bgColour);
        }
    }
}

// Plot a string of characters to the LCD
void lcdPutS(const char *string, unsigned char x, unsigned char y, unsigned short fgColour, unsigned short bgColour)
{
    unsigned char origin = x;

    for (unsigned char characterNumber = 0; characterNumber < strlen(string); characterNumber++)
    {
        // Check if we are out of bounds and move to 
        // the next line if we are
        if (x > 121)
        {
            x = origin;
            y += 8;
        }

        // If we move past the bottom of the screen just exit
        if (y > 120) break;

        // Plot the current character
        lcdPutCh(string[characterNumber], x, y, fgColour, bgColour);
        x += 6;
    }
}

void printDirectory(File dir, int numTabs) {

  while(true) {

     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       Serial.println("**nomorefiles**");
       break;
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println(" [DIR]");
     } else {
       // files have sizes, directories do not
       Serial.println(entry.size(), DEC);
     }
   }
}
void DisplayBitmap( void )
{
    File dir = mySD.open("/");
    printDirectory(dir, 0);
}


void setup()
{
    pinMode(PIN_LED0,OUTPUT);
    pinMode(PIN_LED1,OUTPUT);
    pinMode(PIN_LED2,OUTPUT);
    pinMode(PIN_LED3,OUTPUT);

    delay(2000);

#if 0
    // SD card
    Serial.begin(38400, SCI_AUTO);
    mySD.begin();  

    DisplayBitmap();
#endif

    // LCD module
    lcdInitialise(LCD_ORIENTATION2);
    
    lcdClearDisplay(decodeRgbValue(0, 0, 0));


    lcdLine(0, 0, 127, 127, decodeRgbValue(31, 31, 31));
    lcdLine(0, 127, 127, 0, decodeRgbValue(31, 31, 31));
    lcdCircle(64, 64, 32, decodeRgbValue(31, 0, 0));
    lcdCircle(64, 64, 40, decodeRgbValue(0, 31, 0));
    lcdCircle(64, 64, 48, decodeRgbValue(0, 0, 31));
    
    lcdPutS("Hello World!", lcdTextX(4), lcdTextY(0), decodeRgbValue(0, 0, 0), decodeRgbValue(31, 31, 31));
    
    lcdPutS("The quick brown fox jumped over the lazy dog 0123456789", lcdTextX(0), lcdTextY(2), decodeRgbValue(0, 31, 31), decodeRgbValue(0, 0, 0));
    
    lcdFilledRectangle(0, 64, 127, 127, decodeRgbValue(0, 0, 0));
    lcdRectangle(0, 64, 127, 127, decodeRgbValue(31, 31, 31));

    // Run the LCD test
    unsigned char ballX = 64, ballY = 96;
    char ballSpeed = 1;
    char xDir = ballSpeed, yDir = ballSpeed;
    
    // Bouncy ball demo
    while(1)
    {
        // Delete the ball
        lcdFilledRectangle(ballX-2, ballY-1, ballX+2, ballY+1, decodeRgbValue(0, 0, 0));
        
        // Delete the bat
        lcdFilledRectangle(ballX-4, 121, ballX+4, 123, decodeRgbValue(0, 0, 0));
        
        // Move the ball
        ballX += xDir;
        ballY += yDir;
        
        // Range check
        if (ballX > 120) xDir = -ballSpeed;
        if (ballX < 7) xDir = ballSpeed;
        
        if (ballY > 120) yDir = -ballSpeed;
        if (ballY < 70) yDir = ballSpeed;
        
        // Plot the ball
        lcdFilledRectangle(ballX-2, ballY-1, ballX+2, ballY+1, decodeRgbValue(31, 31, 31));
        
        // Plot the bat
        lcdFilledRectangle(ballX-4, 121, ballX+4, 123, decodeRgbValue(31, 0, 31));
        
        lcdPutS("Hello World!", lcdTextX(4), lcdTextY(0), decodeRgbValue(0, 0, 0), decodeRgbValue(31, 31, 31));
        delay(10);
    }

}

void loop()
{
    digitalWrite(PIN_LED0, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED1, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED2, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED3, 1);
    delay(INTERVAL);
    digitalWrite(PIN_LED0, 0);
    delay(INTERVAL);
    digitalWrite(PIN_LED1, 0);
    delay(INTERVAL);
    digitalWrite(PIN_LED2, 0);
    delay(INTERVAL);
    digitalWrite(PIN_LED3, 0);
    delay(INTERVAL);
}

