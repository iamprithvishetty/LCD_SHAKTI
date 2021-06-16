#include "LCD_SHAKTI.h"
#include "lcd_registers.h"
#include "LCD_font.c"
#include "8bit_parallel.h"
#include "stdbool.h"
#include "Arduino.h"

#define XC ILI9341_COLADDRSET
#define YC ILI9341_PAGEADDRSET
#define CC ILI9341_MEMORYWRITE
#define RC HX8357_RAMRD
#define SC1 0x33
#define SC2 0x37
#define MD ILI9341_MEMORYACCESS
#define VL 0
#define R24BIT 0

#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#define swap(a, b) { int16_t t = a; a = b; b = t; }

LCD_SHAKTI::LCD_SHAKTI( uint16_t LCD_ID,uint16_t wid, uint16_t heigh)
{
    width = wid;
    height = heigh;
    WIDTH = width;
    HEIGHT = height;
    lcd_model = LCD_ID;

    text_color = 0x07E0;
    text_bgcolor = 0xF800;
    draw_color = 0xF800;
    text_size = 1;
    text_mode = 0; //if set,text_bgcolor is invalid
    Serial.println("Hey");

}

LCD_SHAKTI::LCD_SHAKTI( uint16_t wid, uint16_t heigh)
{
    width = wid;
    height = heigh;
    WIDTH = width;
    HEIGHT = height;
    lcd_model = 0xFFFF;

    text_color = 0x07E0;
    text_bgcolor = 0xF800;
    draw_color = 0xF800;
    text_size = 1;
    text_mode = 0; //if set,text_bgcolor is invalid
    Serial.println("Hey");

}

// INITIALIZATION METHODS GOES BELOW

void LCD_SHAKTI::Reset(void)
{
    CS_IDLE;
    RD_IDLE;
    WR_IDLE;
    RES_IDLE;
    delay(2);
    RES_ACTIVE;
    CS_ACTIVE;
    CD_COMMAND;
    setWriteDir();
    write8(0x00);
    for(uint8_t i=0; i<3; i++)
    {
      WR_STROBE; // Three extra 0x00s
    }
    CS_IDLE;
}

//read LCD controller chip ID 
uint16_t LCD_SHAKTI::Read_ID(void)
{
  uint16_t ID;
  setWriteDir();
  CS_ACTIVE;
  writeCmd8(0xD3);
  setReadDir();
  read16(ID);
  //Serial.println(dst); //To Debug
  read16(ID);
  //Serial.println(dst); //To Debug
  setWriteDir();
  return ID;
}

void LCD_SHAKTI::Init_LCD(void)
{

  //CONSTRUCTOR DECLARATION NOT WORKING
  width = 320;
  height = 480;
  WIDTH = width;
  HEIGHT = height;
  lcd_model = 0x9486;
  rotation=0;
  text_color = 0x07E0;
  text_bgcolor = 0xF800;
  draw_color = 0xF800;
  text_size = 1;
  text_mode = 0; //if set,text_bgcolor is invalid
  //CONSTRUCTOR DECLARATION NOT WORKING

  setWriteDir();
  SET_CNTL_BITS; // SET Control Bits to OUTPUT
  RES_ACTIVE;  // RESET pin HIGH
  Reset();
  if(lcd_model == 0xFFFF)
  {
    lcd_model = Read_ID(); 
  }
  Start(lcd_model);

}

void LCD_SHAKTI::Start(uint16_t ID)
{
  Reset();
  delay(200);
  switch(ID)
  {
    case 0x9486:
      uint8_t data_buf[] = {0x55};
      Push_Command(0x3A,data_buf,1);
      uint8_t data_buf1[] = {};
      Push_Command(0x11,data_buf1,0);
      uint8_t data_buf2[] = {0x28};
      Push_Command(0x36,data_buf2,1);
      uint8_t data_buf3[] = {};
      Push_Command(0x29,data_buf3,0);
      break;
    default:
      break;    
  }
  Set_Rotation(rotation); 
  Invert_Display(0);
}

void LCD_SHAKTI::Push_Command(uint16_t cmd, uint8_t *block, int8_t N)
{
    CS_ACTIVE;
    writeCmd16(cmd);
    while (N-- > 0) 
  {
        uint8_t u8 = *block++;
        writeData8(u8);
    }
    CS_IDLE;
}

void LCD_SHAKTI::Set_Rotation(uint8_t r)
{
  rotation = r & 3;           // just perform the operation ourselves on the protected variables
  width = (rotation & 1) ? HEIGHT : WIDTH;
  height = (rotation & 1) ? WIDTH : HEIGHT;
  CS_ACTIVE;
  uint8_t val;
  switch (rotation) 
  {
    case 0:
      val = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR; //0 degree 
      break;
    case 1:
      val = ILI9341_MADCTL_MV | ILI9341_MADCTL_ML | ILI9341_MADCTL_BGR ; //90 degree 
      break;
    case 2:
      val = ILI9341_MADCTL_MY |ILI9341_MADCTL_BGR; //180 degree 
      break;
    case 3:
      val = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; //270 degree
      break;
  }
    writeCmdData8(MD, val); 

  Set_Addr_Window(0, 0, width - 1, height - 1);
  Vert_Scroll(0, HEIGHT, 0);
  CS_IDLE;
}

void LCD_SHAKTI::Set_Addr_Window(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  CS_ACTIVE;
  uint8_t x_buf[] = {x1>>8,x1&0xFF,x2>>8,x2&0xFF}; 
  uint8_t y_buf[] = {y1>>8,y1&0xFF,y2>>8,y2&0xFF}; 

  Push_Command(XC, x_buf, 4); //set x address
  Push_Command(YC, y_buf, 4); //set y address
  CS_IDLE;    
}

//Scroll display
void LCD_SHAKTI::Vert_Scroll(int16_t top, int16_t scrollines, int16_t offset)
{
    int16_t bfa = HEIGHT - top - scrollines; 
    int16_t vsp;
    int16_t sea = top;
    if (offset <= -scrollines || offset >= scrollines)
    {
    offset = 0; //valid scroll
    }
  vsp = top + offset; // vertical start position
    if (offset < 0)
    {
        vsp += scrollines;          //keep in unsigned range
    }
    sea = top + scrollines - 1;

  uint8_t d[6];           // for multi-byte parameters
  d[0] = top >> 8;        //TFA
  d[1] = top;
  d[2] = scrollines >> 8; //VSA
  d[3] = scrollines;
  d[4] = bfa >> 8;        //BFA
  d[5] = bfa;
  Push_Command(SC1, d, 6);
  d[0] = vsp >> 8;        //VSP
  d[1] = vsp;
  Push_Command(SC2, d, 2);
  Push_Command(0x13, NULL, 0);
}

//get lcd width
uint16_t LCD_SHAKTI::Get_Width()
{
  return width;
}

//get lcd height
uint16_t LCD_SHAKTI::Get_Height()
{
  return height;
}

void LCD_SHAKTI::Invert_Display(bool i)
{
  CS_ACTIVE;
  uint8_t val = VL^i;
  writeCmd8(val ? 0x21 : 0x20);
  CS_IDLE;
}

//Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t LCD_SHAKTI::Color_To_565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

//read graph RAM data
int16_t LCD_SHAKTI::Read_GRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h)
{
  uint16_t ret, dummy;
    int16_t n = w * h;
    uint8_t r, g, b, tmp;
    Set_Addr_Window(x, y, x + w - 1, y + h - 1);
    while (n > 0) 
  {
        CS_ACTIVE;
        writeCmd16(RC);
        setReadDir();
        read8(r);
    while (n) 
    {
      if(R24BIT == 1)
      {
        read8(r);
        read8(g);
        read8(b);
        ret = Color_To_565(r, g, b);
      }
      else if(R24BIT == 0)
      {
        read16(ret);
      }
      *block++ = ret;
      n--;
    }
        
//      RD_IDLE;
        CS_IDLE;
        setWriteDir();
    }
  return 0;
}

//set x,y  coordinate and color to draw a pixel point 
void LCD_SHAKTI::Draw_Pixe(int16_t x, int16_t y, uint16_t color)
{
  if((x < 0) || (y < 0) || (x > Get_Width()) || (y > Get_Height()))
  {
    return;
  }
  Set_Addr_Window(x, y, x, y);
  CS_ACTIVE;
  writeCmdData16(CC, color);
  CS_IDLE;
}

//fill area from x to x+w,y to y+h
void LCD_SHAKTI::Fill_Rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  int16_t end;
  if (w < 0) 
  {
        w = -w;
        x -= w;
    }                           //+ve w
    end = x + w;
    if (x < 0)
    {
        x = 0;
    }
    if (end > Get_Width())
    {
        end = Get_Width();
    }
    w = end - x;
    if (h < 0) 
  {
        h = -h;
        y -= h;
    }                           //+ve h
    end = y + h;
    if (y < 0)
    {
        y = 0;
    }
    if (end > Get_Height())
    {
        end = Get_Height();
    }
    h = end - y;
    Set_Addr_Window(x, y, x + w - 1, y + h - 1);//set area
  CS_ACTIVE;
  writeCmd8(CC);  
  if (h > w) 
  {
        end = h;
        h = w;
        w = end;
    }
  while (h-- > 0) 
  {
    end = w;
    do 
    {
        writeData16(color);//set color data
        } while (--end != 0);
  }
  CS_IDLE;
}

//get current rotation
//0  :  0 degree 
//1  :  90 degree
//2  :  180 degree
//3  :  270 degree
uint8_t LCD_SHAKTI::Get_Rotation(void)
{
  return rotation;
}

//GUI METHOD GOES BELOW

//set 16bits draw color
void LCD_SHAKTI::Set_Draw_color(uint16_t color)
{
  draw_color = color;
}

//set 8bits r,g,b color
void LCD_SHAKTI::Set_Draw_color(uint8_t r, uint8_t g, uint8_t b)
{
  draw_color = Color_To_565(r, g, b);
}

//get draw color
uint16_t LCD_SHAKTI::Get_Draw_color() 
{
  return draw_color;
}

//draw a pixel point
void LCD_SHAKTI::Draw_Pixel(int16_t x, int16_t y)
{
  Draw_Pixe(x, y, draw_color);
}

//read color data for point(x,y)
uint16_t LCD_SHAKTI::Read_Pixel(int16_t x, int16_t y)
{
  uint16_t colour;
  Read_GRAM(x, y, &colour, 1, 1);
  return colour;
} 

//fill a rectangle
void LCD_SHAKTI::Fill_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  int w = x2 - x1 + 1, h = y2 - y1 + 1;
    if (w < 0) 
  { 
    x1 = x2; 
    w = -w; 
  }
  if (h < 0) 
  { 
    y1 = y2; 
    h = -h; 
  }
  Fill_Rect(x1, y1, w, h, draw_color);
}

//draw a vertical line 
void LCD_SHAKTI::Draw_Fast_VLine(int16_t x, int16_t y, int16_t h)
{
  Fill_Rect(x, y, 1, h, draw_color);
}

//draw a horizontal line
void LCD_SHAKTI::Draw_Fast_HLine(int16_t x, int16_t y, int16_t w)
{
  Fill_Rect(x, y, w, 1, draw_color);
}

//Fill the full screen with color
void LCD_SHAKTI::Fill_Screen(uint16_t color)
{
  Fill_Rect(0, 0, Get_Width(), Get_Height(), color);
}

//Fill the full screen with r,g,b
void LCD_SHAKTI::Fill_Screen(uint8_t r, uint8_t g, uint8_t b)
{
  uint16_t color = Color_To_565(r, g, b);
  Fill_Rect(0, 0, Get_Width(), Get_Height(), color);
}

//abs Arduino Method
int16_t LCD_SHAKTI::Abs(int16_t data)
{
  if(data < 0)
  {
    return -1*data;
  }
  else
  {
    return data;
  }
}

//draw an arbitrary line from (x1,y1) to (x2,y2)
void LCD_SHAKTI::Draw_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  int16_t steep = Abs(y2 - y1) > Abs(x2 - x1);
    if (steep) 
  {
      swap(x1, y1);
      swap(x2, y2);
  }
  if (x1 > x2) 
  {
      swap(x1, x2);
      swap(y1, y2);
    }
  
    int16_t dx, dy;
    dx = x2 - x1;
    dy = Abs(y2 - y1);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y1 < y2) 
  {
      ystep = 1;
    } 
  else 
  {
      ystep = -1;
  }

  for (; x1<=x2; x1++) 
  {
      if (steep) 
    {
          Draw_Pixel(y1, x1);
      } 
    else 
    {
          Draw_Pixel(x1, y1);
      }
      err -= dy;
      if (err < 0) 
    {
      y1 += ystep;
      err += dx;
      }
    }
}

//draw a rectangle
void LCD_SHAKTI::Draw_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{ 
  int16_t w = x2 - x1 + 1, h = y2 - y1 + 1;
  if (w < 0) 
  { 
    x1 = x2; 
    w = -w; 
  }
  if (h < 0) 
  { 
    y1 = y2; 
    h = -h; 
  }
  Draw_Fast_HLine(x1, y1, w);
  Draw_Fast_HLine(x1, y2, w);
  Draw_Fast_VLine(x1, y1, h);
  Draw_Fast_VLine(x2, y1, h);
}

//draw a round rectangle
void LCD_SHAKTI::Draw_Round_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t radius)
{
  int w = x2 - x1 + 1, h = y2 - y1 + 1;
  if (w < 0) 
  { 
    x1 = x2; 
    w = -w; 
  }
  if (h < 0) 
  { 
    y1 = y2; 
    h = -h; 
  }
  Draw_Fast_HLine(x1+radius, y1, w-2*radius); 
  Draw_Fast_HLine(x1+radius, y1+h-1, w-2*radius); 
  Draw_Fast_VLine(x1, y1+radius, h-2*radius); 
    Draw_Fast_VLine(x1+w-1, y1+radius, h-2*radius);
  Draw_Circle_Helper(x1+radius, y1+radius, radius, 1);
  Draw_Circle_Helper(x1+w-radius-1, y1+radius, radius, 2);
  Draw_Circle_Helper(x1+w-radius-1, y1+h-radius-1, radius, 4);
  Draw_Circle_Helper(x1+radius, y1+h-radius-1, radius, 8);
}

//fill a round rectangle
void LCD_SHAKTI::Fill_Round_Rectangle(int16_t x1, int16_t y1, int16_t x2,int16_t y2, int16_t radius)
{
  int w = x2 - x1 + 1, h = y2 - y1 + 1;
  if (w < 0) 
  { 
    x1 = x2; 
    w = -w; 
  }
  if (h < 0) 
  { 
    y1 = y2; 
    h = -h; 
  }
  Fill_Rect(x1+radius, y1, w-2*radius, h, draw_color);
  Fill_Circle_Helper(x1+w-radius-1, y1+radius, radius, 1, h-2*radius-1);
  Fill_Circle_Helper(x1+radius, y1+radius, radius, 2, h-2*radius-1);  
}

//draw a circle
void LCD_SHAKTI::Draw_Circle(int16_t x, int16_t y, int16_t radius)
{
  int16_t f = 1 - radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * radius;
  int16_t x1= 0;
  int16_t y1= radius;

  Draw_Pixel(x, y+radius);
  Draw_Pixel(x, y-radius);
  Draw_Pixel(x+radius, y);
  Draw_Pixel(x-radius, y);

  while (x1<y1) 
  {
      if (f >= 0) 
    {
          y1--;
          ddF_y += 2;
          f += ddF_y;
      }
      x1++;
      ddF_x += 2;
      f += ddF_x;
  
    Draw_Pixel(x + x1, y + y1);
      Draw_Pixel(x - x1, y + y1);
    Draw_Pixel(x + x1, y - y1);
    Draw_Pixel(x - x1, y - y1);
    Draw_Pixel(x + y1, y + x1);
    Draw_Pixel(x - y1, y + x1);
    Draw_Pixel(x + y1, y - x1);
    Draw_Pixel(x - y1, y - x1);
  }
}

//draw a circular bead
void LCD_SHAKTI::Draw_Circle_Helper(int16_t x0, int16_t y0, int16_t radius, uint8_t cornername)
{
  int16_t f     = 1 - radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * radius;
  int16_t x     = 0;
  int16_t y     = radius;
  while (x<y) 
  {
      if (f >= 0) 
    {
          y--;
          ddF_y += 2;
          f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;
      if (cornername & 0x4) 
    {
      Draw_Pixel(x0 + x, y0 + y);
      Draw_Pixel(x0 + y, y0 + x);
      } 
      if (cornername & 0x2) 
    {
      Draw_Pixel(x0 + x, y0 - y);
      Draw_Pixel(x0 + y, y0 - x);
      }
      if (cornername & 0x8) 
    {
      Draw_Pixel(x0 - y, y0 + x);
      Draw_Pixel(x0 - x, y0 + y);
      }
      if (cornername & 0x1)
    {
      Draw_Pixel(x0 - y, y0 - x);
      Draw_Pixel(x0 - x, y0 - y);
      }
    }
}

//fill a circle
void LCD_SHAKTI::Fill_Circle(int16_t x, int16_t y, int16_t radius)
{
  Draw_Fast_VLine(x, y-radius, 2*radius+1);
  Fill_Circle_Helper(x, y, radius, 3, 0);
}

//fill a semi-circle
void LCD_SHAKTI::Fill_Circle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) 
  {
      if (f >= 0) 
    {
          y--;
          ddF_y += 2;
          f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      if (cornername & 0x1) 
    {
          Draw_Fast_VLine(x0+x, y0-y, 2*y+1+delta);
          Draw_Fast_VLine(x0+y, y0-x, 2*x+1+delta);
      }
      if (cornername & 0x2) 
    {
          Draw_Fast_VLine(x0-x, y0-y, 2*y+1+delta);
          Draw_Fast_VLine(x0-y, y0-x, 2*x+1+delta);
      }
    }
}

//draw a triangle
void LCD_SHAKTI::Draw_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2)
{
  Draw_Line(x0, y0, x1, y1);
  Draw_Line(x1, y1, x2, y2);
    Draw_Line(x2, y2, x0, y0);
}

//fill a triangle
void LCD_SHAKTI::Fill_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2)
{
  int16_t a, b, y, last;
    if (y0 > y1) 
  {
      swap(y0, y1); 
    swap(x0, x1);
    }
    if (y1 > y2) 
  {
      swap(y2, y1); 
    swap(x2, x1);
    }
    if (y0 > y1) 
  {
      swap(y0, y1); 
    swap(x0, x1);
    }

  if(y0 == y2) 
  { 
      a = b = x0;
      if(x1 < a)
      {
      a = x1;
      }
      else if(x1 > b)
      {
      b = x1;
      }
      if(x2 < a)
      {
      a = x2;
      }
      else if(x2 > b)
      {
      b = x2;
      }
      Draw_Fast_HLine(a, y0, b-a+1);
      return;
  }
  
  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;
  if(y1 == y2)
  {
    last = y1; 
  }
    else
    {
    last = y1-1; 
    }

    for(y=y0; y<=last; y++) 
  {
      a   = x0 + sa / dy01;
      b   = x0 + sb / dy02;
      sa += dx01;
      sb += dx02;
      if(a > b)
      {
      swap(a,b);
      }
      Draw_Fast_HLine(a, y, b-a+1);
  }
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) 
  {
      a   = x1 + sa / dy12;
      b   = x0 + sb / dy02;
      sa += dx12;
      sb += dx02;
      if(a > b)
      {
      swap(a,b);
      }
    Draw_Fast_HLine(a, y, b-a+1);
  }
}

//set text coordinate
void LCD_SHAKTI::Set_Text_Cousur(int16_t x, int16_t y)
{
  text_x = x;
  text_y = y;
}

//get text x coordinate
int16_t LCD_SHAKTI::Get_Text_X_Cousur(void) 
{
  return text_x;
}

//get text y coordinate
int16_t LCD_SHAKTI::Get_Text_Y_Cousur(void) 
{
  return text_y;
}

//set text colour with 16bit color
void LCD_SHAKTI::Set_Text_colour(uint16_t color)
{
  text_color = color;
}

//set text colour with 8bits r,g,b
void LCD_SHAKTI::Set_Text_colour(uint8_t r, uint8_t g, uint8_t b)
{
  text_color = Color_To_565(r, g, b);
}

//get text colour
uint16_t LCD_SHAKTI::Get_Text_colour(void)
{
  return text_color;
}

//set text background colour with 16bits color
void LCD_SHAKTI::Set_Text_Back_colour(uint16_t color)
{
  text_bgcolor = color; 
}

//set text background colour with 8bits r,g,b
void LCD_SHAKTI::Set_Text_Back_colour(uint8_t r, uint8_t g, uint8_t b)
{
  text_bgcolor = Color_To_565(r, g, b);
}

//get text background colour
uint16_t LCD_SHAKTI::Get_Text_Back_colour(void)
{
  return text_bgcolor;
}

//set text size
void LCD_SHAKTI::Set_Text_Size(uint8_t s)
{
  text_size = s;
}

//get text size
uint8_t LCD_SHAKTI::Get_Text_Size(void)
{
  return text_size;
}

//set text mode
void LCD_SHAKTI::Set_Text_Mode(boolean mode)
{
  text_mode = mode;
}

//get text mode
boolean LCD_SHAKTI::Get_Text_Mode(void)
{
  return text_mode;
}

//draw a char
void LCD_SHAKTI::Draw_Char(int16_t x, int16_t y, uint8_t c, uint16_t color,uint16_t bg, uint8_t size, boolean mode)
{
  if((x >= Get_Width()) || (y >= Get_Height()) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
  {
      return;
  }   
    if(c >= 176)
    {
    c++; 
    }
  for (int8_t i=0; i<6; i++) 
  {
      uint8_t line;
      if (i == 5)
      {
          line = 0x0;
      }
      else
      {
          line = pgm_read_byte(lcd_font+(c*5)+i);
      }
      for (int8_t j = 0; j<8; j++) 
    {
          if (line & 0x1) 
      {
            if (size == 1)
            {
              Draw_Pixe(x+i, y+j, color);
            }
            else 
        {  
          Fill_Rect(x+(i*size), y+(j*size), size, size, color);
            }
          } 
      else if (bg != color)         
      {
        if(!mode)
        {
              if (size == 1) 
              {
                Draw_Pixe(x+i, y+j, bg);
              }
              else 
          {  
            Fill_Rect(x+i*size, y+j*size, size, size, bg);
          }
        }
      }
          line >>= 1;
      }
    }
}

//print string
size_t LCD_SHAKTI::Print(uint8_t *st, int16_t x, int16_t y)
{
  int16_t pos;
  uint16_t len;
  const char * p = (const char *)st;
  size_t n = 0;
  if (x == CENTER || x == RIGHT) 
  {
    len = mystrlen((const char *)st) * 6 * text_size;   
    pos = (Get_Display_Width() - len); 
    if (x == CENTER)
    {
      x = pos/2;
    }
    else
    {
      x = pos - 1;
    }
  }
    Set_Text_Cousur(x, y);
  while(1)
  {
    unsigned char ch = *(p++);
    if(ch == 0)
    {
      break;
    }
    if(write(ch))
    {
      n++;
    }
    else
    {
      break;
    }
  } 
  return n;
}

int LCD_SHAKTI::mystrlen(char *str)
{
    int len = 0;
    int i;
    
    for (i=0; str[i] != 0; i++) 
    {
        len++;
    }
    return(len-1);
}

//print string
void LCD_SHAKTI::Print_String(const uint8_t *st, int16_t x, int16_t y)
{
  Print((uint8_t *)st, x, y);
}

//print string
void LCD_SHAKTI::Print_String(uint8_t *st, int16_t x, int16_t y)
{
  Print(st, x, y);
}

//print int number
void LCD_SHAKTI::Print_Number_Int(long num, int16_t x, int16_t y, int16_t length, uint8_t filler, int16_t system)
{
  uint8_t st[27] = {0};
  uint8_t *p = st+26;
  boolean flag = false;
  int16_t len = 0,nlen = 0,left_len = 0,i = 0;
  *p = '\0';
  if(0 == num)
  {
    *(--p) = '0';
    len = 1;
  }
  else
  {
    if(num < 0)
    {
      num = -num;
      flag = true;
    }   
  }
  while((num > 0) && (len < 10))
  {
    if(num%system > 9)
    {
      *(--p) = 'A' + (num%system-10);
    }
    else
    {
      *(--p) = '0' + num%system;
    }
    num = num/system;
    len++;
  }
  if(flag)
  {
    *(--p) = '-';
  }
  if(length > (len + flag + 1))
  {
    if(length > sizeof(st))
    {
      nlen = sizeof(st) - len - flag - 1;
    }
    else
    {
      nlen = length - len - flag - 1;
    }
    for(i = 0;i< nlen;i++)
    {
      *(--p) = filler;
    }
    left_len = sizeof(st) - nlen - len - flag - 1;
  } 
  else
  {
    left_len = sizeof(st) - len - flag - 1;
  }
  for(i = 0; i < (sizeof(st) - left_len);i++)
  {
    st[i] = st[left_len + i];
  }
  st[i] = '\0';
  Print(st, x, y);
}


//write a char
size_t LCD_SHAKTI::write(uint8_t c) 
{
  if (c == '\n') 
  {
      text_y += text_size*8;
      text_x  = 0;
  } 
  else if(c == '\r')
  {
  }
  else 
  {
      Draw_Char(text_x, text_y, c, text_color, text_bgcolor, text_size,text_mode);
      text_x += text_size*6;    
    } 
    return 1; 
}

//get lcd width
int16_t LCD_SHAKTI::Get_Display_Width(void)
{
  return Get_Width();
}

//get lcd height 
int16_t LCD_SHAKTI::Get_Display_Height(void) 
{
  return Get_Height();
}