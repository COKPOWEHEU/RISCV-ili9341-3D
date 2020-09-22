#include "lcd_ili9341.h"

union lcd_colors_t lcd_color;
uint8_t lcd_font_size;

void lcd_sleep(uint32_t time){
  while(time--){asm volatile ("nop");}
}

void lcd_spi_init(){
#if LCD_SPI == 0
  RCU_APB2EN |= RCU_APB2EN_SPI0EN;
#elif LCD_SPI == 1
  RCU_APB1EN |= RCU_APB1EN_SPI1EN;
#elif LCD_SPI == 2
  RCU_APB1EN |= RCU_APB1EN_SPI2EN;
#else
  #error wrong SPI
#endif
  GPIO_config(LCD_RST);
  GPIO_config(LCD_CMD);
  GPIO_config(LCD_DOUT);
  GPIO_config(LCD_SCK);
  GPIO_config(LCD_CS);
  lcd_size_8();
  SPI_CTL1(SPI_NAME) = 0;//SPI_CTL1_NSSDRV;
  SPI_DATA(SPI_NAME) = 0;
}

uint16_t lcd_send(uint16_t data){
  uint16_t res;
  while(!(SPI_STAT(SPI_NAME) & SPI_STAT_TBE)){}
  res = SPI_DATA(SPI_NAME);
  SPI_DATA(SPI_NAME) = data;
  return res;
}

void lcd_start(uint8_t cmd){
  lcd_wait();
  GPO_ON(LCD_CMD);
  GPO_ON(LCD_CS);
  SPI_DATA(SPI_NAME) = cmd;
  lcd_wait();
  GPO_OFF(LCD_CMD);
}

void lcd_data(uint8_t data){
  lcd_wait();
  GPO_OFF(LCD_CMD);
  GPO_ON(LCD_CS);
  SPI_DATA(SPI_NAME) = data;
  lcd_wait();
  GPO_OFF(LCD_CS);
}

void lcd_word(uint16_t data){
  lcd_send((uint8_t)(data>>8));
  lcd_send((uint8_t)data);
}

void lcd_begin_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
  lcd_wait();
  lcd_size_8();
  lcd_cmd(0x00);
  lcd_start(0x2A); lcd_word(y1); lcd_word(y2); lcd_stop();
  lcd_start(0x2B); lcd_word(x1); lcd_word(x2); lcd_stop();
  lcd_start(0x2C);
}

void lcd_clr(uint16_t color){
  uint32_t i,w=LCD_MAXX+1,h=LCD_MAXY+1,n;
  n = w*h;
  lcd_begin_area(0,0,w-1,h-1);
  
  lcd_size_16();
  
  for(i=0;i<n;i++){
    lcd_send(color);
  }
  lcd_stop();
  lcd_size_8();
}

void lcd_init(){
  GPIO_config(LCD_CS);
  GPIO_config(LCD_CMD);
  GPIO_config(LCD_RST);
  lcd_spi_init();
  
  GPO_ON(LCD_RST);
  lcd_sleep(100000);
  GPO_OFF(LCD_RST);
  lcd_sleep(100000);
  //soft reset
  lcd_cmd(0x01);
  lcd_sleep(100000);
  //power control A
  lcd_start(0xCB); lcd_send(0x39); lcd_send(0x2C); lcd_send(0x00);
                   lcd_send(0x34); lcd_send(0x02); lcd_stop();
  //power control B
  lcd_start(0xCF); lcd_send(0x00); lcd_send(0xC1); lcd_send(0x30); lcd_stop();
  //timing control A
  lcd_start(0xE8); lcd_send(0x85); lcd_send(0x00); lcd_send(0x78); lcd_stop();
  //timing control B
  lcd_start(0xEA); lcd_send(0x00); lcd_send(0x00); lcd_stop();
  //poweOn control
  lcd_start(0xED); lcd_send(0x64); lcd_send(0x03); lcd_send(0x12); lcd_send(0x81); lcd_stop();
  //display function control
  lcd_start(0xB6); lcd_send(0x08); lcd_send(0x82); lcd_send(0x27); lcd_stop();
  //gamma pos
  lcd_start(0xE0); lcd_send(0x0F); lcd_send(0x31); lcd_send(0x2B); lcd_send(0x0C);
                   lcd_send(0x0E); lcd_send(0x08); lcd_send(0x4E); lcd_send(0xF1);
                   lcd_send(0x37); lcd_send(0x07); lcd_send(0x10); lcd_send(0x03);
                   lcd_send(0x0E); lcd_send(0x09); lcd_send(0x00); lcd_stop();
  //gamma neg
  lcd_start(0xE1); lcd_send(0x00); lcd_send(0x0E); lcd_send(0x14); lcd_send(0x03);
                   lcd_send(0x11); lcd_send(0x07); lcd_send(0x31); lcd_send(0xC1);
                   lcd_send(0x48); lcd_send(0x08); lcd_send(0x0F); lcd_send(0x0C);
                   lcd_send(0x31); lcd_send(0x36); lcd_send(0x0F); lcd_stop();
  //pump ratio control
  lcd_cmd(0xF7); lcd_data(0x20);
  //power control 1
  lcd_cmd(0xC0); lcd_data(0x23);
  //power control 2
  lcd_cmd(0xC1); lcd_data(0x10);
  //VCOM control 1
  lcd_cmd(0xC5); lcd_data(0x3e); lcd_data(0x28);
  //VCOM control 2
  lcd_cmd(0xC7); lcd_data(0x86);
  //pixel format
  lcd_cmd(0x3A); lcd_data(0x55);
  //Frame control
  lcd_cmd(0xB1); lcd_data(0x00); lcd_data(0x18);
  //enable 3G
  lcd_cmd(0xF2); lcd_data(0x00);
  //gamma set
  lcd_cmd(0x26); lcd_data(0x01);
  //memory access control
  lcd_start(0x36);
  lcd_send((LCD_MIR_Y<<7)|(LCD_MIR_X<<6)|(LCD_ORIENT_V<<5)|(LCD_ML<<4)|(LCD_RGB<<3)|(LCD_MH<<2));
  lcd_stop();
  
  //sleep out
  lcd_cmd(0x11);
  lcd_sleep(100000);
  //display on
  lcd_cmd(0x29);
  //lcd_stop();
}

#include "lcd_chars.h"
void lcd_char(uint16_t x0, uint16_t y0, unsigned char ch){
  unsigned char temp;
  uint8_t i, j;
#ifndef LCD_CHARS_FULL_TABLE
  ch-=' ';
#endif
  if(ch>=CHAR_NUM)ch=0;
  lcd_begin_area(x0, y0, x0+(CHAR_W+1)*lcd_font_size, y0+8*lcd_font_size-1);
  
  uint16_t clr;
  for(x0=0; x0<8*lcd_font_size*lcd_font_size; x0++)lcd_word(lcd_color.bg);
    
  for(y0=0; y0<CHAR_W; y0++){
    for(i=0; i<lcd_font_size; i++){
      temp = char_table[ch][y0];
      for(x0=0; x0<8; x0++){
        if(temp & (1<<0))clr = lcd_color.fg; else clr = lcd_color.bg;
        for(j=0; j<lcd_font_size; j++){
          lcd_word(clr);
        }
        temp>>=1;
      }
    }
  }
  lcd_end_area();
}

void lcd_str(uint16_t x, uint16_t y, char ch[]){
  while(ch[0]!=0){
    lcd_char(x,y,ch[0]);
    x += (CHAR_W+1)*lcd_font_size;
    ch++;
    if(x >= LCD_MAXX-CHAR_W)break;
  }
}

#ifdef LCD_USE_DMA

void lcd_dma_init(uint16_t mem[], uint16_t size){
  SPI_CTL0(SPI_NAME) = SPI_CTL0_MSTMOD | SPI_CTL0_FF16 | LCD_SPI_SPEED | SPI_CTL0_SWNSS | SPI_CTL0_SWNSSEN;
#if (LCD_SPI == 0) || (LCD_SPI == 1)
  RCU_AHBEN |= RCU_AHBEN_DMA0EN;
#elif LCD_SPI == 2
  RCU_AHBEN |= RCU_AHBEN_DMA1EN;
#endif
  DMA_CHPADDR(LCD_DMA, LCD_DMA_CHAN) = (uint32_t)(&(SPI_DATA(SPI_NAME)));
  DMA_CHMADDR(LCD_DMA, LCD_DMA_CHAN) = (uint32_t)mem;
  DMA_CHCNT(LCD_DMA, LCD_DMA_CHAN) = size;
  DMA_CHCTL(LCD_DMA, LCD_DMA_CHAN) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_PRIORITY_ULTRA_HIGH | DMA_PERIPHERAL_WIDTH_16BIT | DMA_MEMORY_WIDTH_16BIT;
  DMA_CHCTL(LCD_DMA, LCD_DMA_CHAN) |= DMA_CHXCTL_CHEN;
  
  SPI_CTL1(SPI_NAME) |= SPI_CTL1_DMATEN;
  DMA_INTC(LCD_DMA) = DMA_FLAG_ADD(DMA_INTC_FTFIFC, LCD_DMA_CHAN);
  SPI_CTL0(SPI_NAME) |= SPI_CTL0_SPIEN;
}

void lcd_dma_restart(uint16_t mem[], uint16_t size){
  DMA_CHCTL(LCD_DMA, LCD_DMA_CHAN) &=~ DMA_CHXCTL_CHEN;
  DMA_INTC(LCD_DMA) = DMA_FLAG_ADD(DMA_INTC_FTFIFC, LCD_DMA_CHAN);
  DMA_CHMADDR(LCD_DMA, LCD_DMA_CHAN) = (uint32_t)mem;
  DMA_CHCNT(LCD_DMA, LCD_DMA_CHAN) = size;
  DMA_CHCTL(LCD_DMA, LCD_DMA_CHAN) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_PRIORITY_ULTRA_HIGH | DMA_PERIPHERAL_WIDTH_16BIT | DMA_MEMORY_WIDTH_16BIT;
  DMA_CHCTL(LCD_DMA, LCD_DMA_CHAN) |= DMA_CHXCTL_CHEN;
}
void lcd_dma_deinit(){
  SPI_CTL1(SPI_NAME) &=~ SPI_CTL1_DMATEN;
  DMA_INTC(LCD_DMA) = DMA_FLAG_ADD(DMA_INTC_FTFIFC, LCD_DMA_CHAN);
  DMA_CHCTL(LCD_DMA, LCD_DMA_CHAN) &=~ DMA_CHXCTL_CHEN;
  SPI_CTL0(SPI_NAME) &=~ SPI_CTL0_SPIEN;
  
  lcd_size_8();
}

#endif
