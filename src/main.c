#include "gd32vf103.h"
#include "pinmacro.h"
#include "lcd_ili9341.h"
#include "gl.h"
#include "clock.h"

#define RLED B, 5, 1, GPIO_PP50
#define YLED B, 6, 1, GPIO_PP50
#define GLED B, 7, 1, GPIO_PP50
#define SBTN B, 0, 0, GPIO_HIZ
#define RBTN B, 1, 0, GPIO_HIZ

void sleep(uint32_t time){
  for(;time;time--)asm volatile("nop");
}

#include "../model/fly.h"
#include "../model/lur.h"
#include "../model/shadow.h"
const struct glVector3 v[]={
  {.x=-1,.y=-1,.z=0,.col=rgb2col(0xFF,0,0)},
  {.x=1,.y=-1,.z=0,.col=rgb2col(0xFF,0,0)},
  {.x=-1,.y=1,.z=0,.col=rgb2col(0xFF,0,0)},
  {.x=1,.y=1,.z=0,.col=rgb2col(0xFF,0,0)},
  
  {.x=-1,.y=0,.z=-1,.col=rgb2col(0,0xFF,0)},
  {.x=1,.y=0,.z=-1,.col=rgb2col(0,0xFF,0)},
  {.x=-1,.y=0,.z=1,.col=rgb2col(0,0xFF,0)},
  {.x=1,.y=0,.z=1,.col=rgb2col(0,0xFF,0)},
  
  {.x=0,.y=-1,.z=-1,.col=rgb2col(0,0,0xFF)},
  {.x=0,.y=1,.z=-1,.col=rgb2col(0,0,0xFF)},
  {.x=0,.y=-1,.z=1,.col=rgb2col(0,0,0xFF)},
  {.x=0,.y=1,.z=1,.col=rgb2col(0,0,0xFF)}
};

__attribute__((weak))void* memset(void *s, int c, size_t n){
  char *dst = s;
  for(;n;n--){*(dst++) = c;}
  return s;
}
__attribute__((weak))void *memcpy(void *dest, const void *sorc, size_t n){
  char *dst = dest;
  char *src = src;
  for(;n;n--){*(dst++) = *(src++);}
  return dest;
}

void lcd_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color){
  uint32_t i,n;
  
  lcd_begin_area(x1,y1,x2,y2);
  
  n = 2*(x2-x1+1)*(y2-y1+1);
  
  lcd_size_16();
  
  for(i=0;i<n;i++){
    lcd_send(color);
  }
  lcd_end_area();
  
  lcd_size_8();
}

int main(void){
  RCU_APB2EN |= RCU_APB2EN_AFEN;
  RCU_APB2EN |= RCU_APB2EN_PAEN | RCU_APB2EN_PBEN | RCU_APB2EN_PCEN;
  RCU_AHBEN |= RCU_AHBEN_DMA0EN;
  clock_max();
  
  GPIO_config(RLED); GPIO_config(YLED); GPIO_config(GLED);
  GPIO_config(SBTN); GPIO_config(RBTN);
  
  lcd_init();

  glInit();
  GPO_ON(RLED);
  
  uint16_t i=0,j=0,k=0;
  uint8_t mode=0;
  uint8_t old_state=0;
  while(1){
    glLoadIdentity();
    
    glRotateZu(i>>8);
    glRotateXu(j>>8);
    glRotateYu(k>>8);
    
    i+=0xFF;
    j+=0x010F;
    k+=0x0250;
    
    
    if(mode == 0){
      glDrawTriangle(&v[0],&v[1],&v[2]);
      glDrawTriangle(&v[1],&v[2],&v[3]);
      glDrawTriangle(&v[4],&v[5],&v[6]);
      glDrawTriangle(&v[5],&v[6],&v[7]);
      glDrawTriangle(&v[8],&v[9],&v[10]);
      glDrawTriangle(&v[9],&v[10],&v[11]);
    }else if(mode == 1){
      Draw_lur();
    }else if(mode == 2){
      Draw_fly();
    }else if(mode == 3){
      Draw_shadow();
    }else{
      mode=0;
    }
    
    if(GPI_ON(RBTN)){
      old_state=1;
    }else{
      if(old_state == 1){
        mode++;
        if(mode > 3)mode=0;
      }
      old_state = 0;
    }
      
    glSwapBuffers();
    GPO_T(RLED);
  }
}
