#ifndef __CLOCK_H__
#define __CLOCK_H__

#define CLOCK_FAIL -1
#define CLOCK_HSE 2
#define CLOCK_HSI 1

int8_t clock_max(){
  int8_t res = CLOCK_HSE;
  int i;
  uint32_t tmp;
  RCU_CTL &=~ RCU_CTL_HXTALEN;
  RCU_CTL &=~ RCU_CTL_HXTALSTB;
  RCU_CTL |= RCU_CTL_HXTALEN;
  for(i=0;i<0x0FFF;i++){
    if(RCU_CTL & RCU_CTL_HXTALSTB){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)res = CLOCK_HSI;
  
  tmp = RCU_CFG0;
  tmp &=~(RCU_CFG0_PLLMF | RCU_CFG0_PLLSEL);
  if(res == CLOCK_HSE){ //8MHz
    tmp |= RCU_CFG0_PLLSEL;
    RCU_CFG1 = (RCU_CFG1 & RCU_CFG1_PREDV0) | RCU_PREDV0_DIV2; //8/2 = 4 MHz
  }
  tmp |= RCU_PLL_MUL27; //CK_SYS = 4*27 = 108 MHz
  
  tmp &=~ (RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC);
  tmp |= RCU_AHB_CKSYS_DIV1; //AHB = CK_SYS/1 = 108 MHz
  tmp |= RCU_APB1_CKAHB_DIV2; //APB1 = AHB/2 = 54 MHz
  tmp |= RCU_APB2_CKAHB_DIV1; //APB2 = AHB/1 = 108 MHz
  
  RCU_CFG0 = tmp;
  
  RCU_CTL |= RCU_CTL_PLLEN;
  for(i=0;i<0x0FFF;i++){
    if(RCU_CTL & RCU_CTL_PLLSTB){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return CLOCK_FAIL;
  tmp = RCU_CFG0;
  tmp &=~RCU_CFG0_SCS;
  tmp |= RCU_CKSYSSRC_PLL;
  RCU_CFG0 = tmp;
  for(i=0;i<0x0FFF;i++){
    if((RCU_CFG0 & RCU_CFG0_SCSS)==RCU_SCSS_PLL){i=0x1FFF; break;}
  }
  if(i != 0x1FFF)return CLOCK_FAIL;
  return res;
}

#endif
