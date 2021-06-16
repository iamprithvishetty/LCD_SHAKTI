#ifndef LCD_SHAKTI_H
#define LCD_SHAKTI_H

#include "utils.h"
#include "Arduino.h"

class LCD_SHAKTI {
    public:
        LCD_SHAKTI(uint16_t LCD_ID, uint16_t wid, uint16_t heigh);
        LCD_SHAKTI( uint16_t wid, uint16_t heigh);
        void Init_LCD(void);
        uint16_t Read_ID(void);
        void Set_Rotation(uint8_t r);
        void Set_Addr_Window(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
        uint16_t Get_Width(void);
        uint16_t Get_Height(void);
        void Vert_Scroll(int16_t top, int16_t scrollines, int16_t offset);
        void Invert_Display(bool i);
        uint16_t Color_To_565(uint8_t r, uint8_t g, uint8_t b);
        int16_t Read_GRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h);
        void Draw_Pixe(int16_t x, int16_t y, uint16_t color);
        void Fill_Rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        uint8_t Get_Rotation(void);
        void Set_Draw_color(uint16_t color);
        void Set_Draw_color(uint8_t r, uint8_t g, uint8_t b);
        uint16_t Get_Draw_color();
        void Draw_Pixel(int16_t x, int16_t y);
        uint16_t Read_Pixel(int16_t x, int16_t y);
        void Fill_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
        void Draw_Fast_VLine(int16_t x, int16_t y, int16_t h);
        void Draw_Fast_HLine(int16_t x, int16_t y, int16_t w);
        void Fill_Screen(uint16_t color);
        void Fill_Screen(uint8_t r, uint8_t g, uint8_t b);
        void Draw_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
        int16_t Abs(int16_t);
        void Draw_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
        void Draw_Round_Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t radius);
        void Fill_Round_Rectangle(int16_t x1, int16_t y1, int16_t x2,int16_t y2, int16_t radius);
        void Draw_Circle(int16_t x, int16_t y, int16_t radius);
        void Draw_Circle_Helper(int16_t x0, int16_t y0, int16_t radius, uint8_t cornername);
        void Fill_Circle(int16_t x, int16_t y, int16_t radius);
        void Fill_Circle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta);
        void Draw_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2);
        void Fill_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2);
        void Set_Text_Cousur(int16_t x, int16_t y);
        int16_t Get_Text_X_Cousur(void);
        int16_t Get_Text_Y_Cousur(void);
        void Set_Text_colour(uint16_t color);
        void Set_Text_colour(uint8_t r, uint8_t g, uint8_t b);
        uint16_t Get_Text_colour(void);
        void Set_Text_Back_colour(uint16_t color);
        void Set_Text_Back_colour(uint8_t r, uint8_t g, uint8_t b);
        uint16_t Get_Text_Back_colour(void);
        void Set_Text_Size(uint8_t s);
        uint8_t Get_Text_Size(void);
        void Set_Text_Mode(boolean mode);
        boolean Get_Text_Mode(void);
        void Draw_Char(int16_t x, int16_t y, uint8_t c, uint16_t color,uint16_t bg, uint8_t size, boolean mode);
        size_t Print(uint8_t *st, int16_t x, int16_t y);
        int mystrlen(char *str);
        void Print_String(const uint8_t *st, int16_t x, int16_t y);
        void Print_String(uint8_t *st, int16_t x, int16_t y);
        void Print_Number_Int(long num, int16_t x, int16_t y, int16_t length, uint8_t filler, int16_t system);
        size_t write(uint8_t c);
        int16_t Get_Display_Width(void);
        int16_t Get_Display_Height(void);
    
    private:
        uint16_t width,height,lcd_model,HEIGHT,WIDTH;
        uint8_t rotation;
        uint16_t draw_color,text_color,text_bgcolor;
        int16_t text_x, text_y;
        uint8_t text_size;
        bool text_mode;
        void Start(uint16_t ID);
        void Reset(void);
        void Push_Command(uint16_t cmd, uint8_t *block, int8_t N);
};

#endif