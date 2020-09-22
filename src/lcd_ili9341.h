#ifndef __LCD_ILI9341__
#define __LCD_ILI9341__
#include "pinmacro.h"
#include <stdint.h>
#include "gd32vf103.h"

#if 0
  LCD_SPI_SPEED (SPI_CR1_BR_2, SPI_CR1_BR_1, SPI_CR1_BR_0)
  LCD_RST (GPIO)
  LCD_CMD (GPIO)
  LCD_DOUT (GPIO)
  LCD_SCK (GPIO)
  LCD_CS (GPIO)
  LCD_MIR_X (0/1)
  LCD_MIR_Y (0/1)
  LCD_ORIENT_V (0/1)
  LCD_ML (0/1)
  LCD_RGB (0/1)
  LCD_MH (0/1)
#endif
//LCD_SPI
#ifndef LCD_SPI
  #define LCD_SPI 0
#endif
#ifndef LCD_SPI_SPEED
  #define LCD_SPI_SPEED SPI_PSC_2 /* 2,4,8,16,32,64,128,256 */
#endif
#ifndef LCD_RST
  #define LCD_RST   A,2,0,GPIO_PP50
#endif
#ifndef LCD_CMD
  #define LCD_CMD   A,3,0,GPIO_PP50
#endif
#ifndef LCD_DOUT
  #define LCD_DOUT  A,7,1,GPIO_APP50
#endif
#ifndef LCD_SCK
  #define LCD_SCK   A,5,1,GPIO_APP50
#endif
#ifndef LCD_CS
  #define LCD_CS    A,4,0,GPIO_PP50
#endif

//LCD_orientation
#ifndef LCD_MIR_X
  #define LCD_MIR_X 0
#endif
#ifndef LCD_MIR_Y
  #define LCD_MIR_Y 1
#endif
#ifndef LCD_ORIENT_V
  #define LCD_ORIENT_V 1
#endif
#ifndef LCD_ML
  #define LCD_ML 0
#endif
#ifndef LCD_RGB
  #define LCD_RGB 1
#endif
#ifndef LCD_MH
  #define LCD_MH 0
#endif

//DMA
#define LCD_USE_DMA

#if LCD_ORIENT_V
  #define LCD_MAXX 239
  #define LCD_MAXY 319
#else
  #define LCD_MAXX 319
  #define LCD_MAXY 239
#endif

#ifdef LCD_USE_DMA
#if LCD_SPI == 0
  #ifdef DMA0_2
    #error DMA0.2 already used
  #endif
  #define DMA0_2
  #define LCD_DMA DMA0
  #define LCD_DMA_CHAN 2
#elif LCD_SPI == 1
  #ifdef DMA0_4
    #error DMA0.4 already used
  #endif
  #define DMA0_4
  #define LCD_DMA_NUM DMA0
  #define LCD_DMA_CHAN 4
#elif LCD_SPI == 2
  #ifdef DMA1_1
    #error DMA1.1 already used
  #endif
  #define DMA1_1
  #define LCD_DMA_NUM DMA1
  #define LCD_DMA_CHAN 1
#else
  #error wrong SPI
#endif

#endif

#define __SPI_NAME(num) SPI ## num
#define _SPI_NAME(num) __SPI_NAME(num)
#define SPI_NAME _SPI_NAME(LCD_SPI)
union lcd_colors_t{
  struct{
    uint16_t fg;
    uint16_t bg;
  };
  uint32_t both;
};
extern union lcd_colors_t lcd_color;
extern uint8_t lcd_font_size;

//переключение на передачу 8-битных данных
#define lcd_size_8() do{\
    SPI_CTL0(SPI_NAME) = SPI_CTL0_MSTMOD | LCD_SPI_SPEED | SPI_CTL0_SPIEN | SPI_CTL0_SWNSS | SPI_CTL0_SWNSSEN;\
  }while(0)
//переключение на передачу 16-битных данных (удобно для цвета)
#define lcd_size_16() do{\
    SPI_CTL0(SPI_NAME) = SPI_CTL0_MSTMOD | SPI_CTL0_FF16 | LCD_SPI_SPEED | SPI_CTL0_SPIEN | SPI_CTL0_SWNSS | SPI_CTL0_SWNSSEN;\
  }while(0)
//передача 1 слова (8 или 16 бит) по SPI и прием ответа
uint16_t lcd_send(uint16_t data);
//ожидание пока SPI освободится для изменения настроек
#define lcd_wait() do{ while(!(SPI_STAT(SPI_NAME) & SPI_STAT_TBE)){} while(SPI_STAT(SPI_NAME) & SPI_STAT_TRANS){} }while(0)
//передача команды и переключение для передачи данных
//!!!должно завершаться lcd_stop()!!!
void lcd_start(uint8_t cmd);
//конец передачи данных
#define lcd_stop() do{lcd_wait(); GPO_OFF(LCD_CS); }while(0)
//передача только команды
#define lcd_cmd(cmd) do{lcd_start(cmd); lcd_stop();}while(0)
//передача данных (с явным переключением команда->данные)
void lcd_data(uint8_t data);
//передача 16-битного слова двумя порциями по 8 бит
void lcd_word(uint16_t data);
//ограничение области рисование и ожидание приема цветов
//!!!должно завершаться lcd_end_area()!!!
void lcd_begin_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
//завершение рисования в области
#define lcd_end_area() lcd_stop()
//задать цвета
#define lcd_set_color(col_fg, col_bg) do{lcd_color.fg=col_fg; lcd_color.bg=col_bg;}while(0)
//очистка дисплея (с ожиданием завершения)
void lcd_clr(uint16_t color);
//перевод из 3 байт r,g,b в код цвета для дисплея
#if LCD_RGB==1
#define rgb2col(r,g,b) (((r<<8)&0b1111100000000000)|((g<<3)&0b0000011111100000)|(b>>3))
#else
#define rgb2col(r,g,b) (((b<<8)&0b1111100000000000)|((g<<3)&0b0000011111100000)|(r>>3))
#endif
//инициализация дисплея
void lcd_init();

void lcd_char(uint16_t x0, uint16_t y0, unsigned char ch);
void lcd_str(uint16_t x, uint16_t y, char ch[]);

//начало передачи цветов по DMA
void lcd_dma_init(uint16_t mem[], uint16_t size);
void lcd_dma_restart(uint16_t mem[], uint16_t size);
//отключение DMA
void lcd_dma_deinit();
//завершена ли передача по DMA?
#define lcd_dma_finish() (DMA_INTF(LCD_DMA) & DMA_FLAG_ADD(DMA_INTF_FTFIF, LCD_DMA_CHAN))
//ожидание завершения передачи по DMA
#define lcd_dma_wait() do{}while(!lcd_dma_finish())


#endif
