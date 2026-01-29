#include <stdio.h>
#include <NuMicro.h>

#include <string.h>        // stlren fonksiyonun icin kutuphane dahil ediliyor
#include <Th08.h>					// Th08 kütüphanesi dahil ediliyor 
#include <lcd16x2.h>

unsigned char MSG[] ={"MOTOR HIZI"};

char lcd1602_yaz[20];


int adc_data=0;
float led_duty;
float motor_duty;

void Uart0_Init()
{
	UART_Open(UART0, 115200);                               // Uart0 Init ayarlari yapiliyor

}

void I2C2_Init(void)
{
	I2C_Open(I2C2, 100000);											        // I2C2 Init ayarlari yapiliyor
}



void SYS_Init(void)
{
	
	// Clock Ayarlari internal osilator ve 48 Mega Hertz olarak ayarlaniyor
	CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk|CLK_PWRCTL_HIRCEN_Msk|CLK_PWRCTL_HIRC48EN_Msk); 
	CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk|CLK_STATUS_HIRCSTB_Msk|CLK_STATUS_HIRC48STB_Msk);
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
	CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_HCLK | CLK_PCLKDIV_APB1DIV_HCLK);
	// I2C2 Clock Ayarlari Enable yapiliyor
	CLK_EnableModuleClock(I2C2_MODULE);																								
	// Timer 0 Clock Ayarlari Enable yapiliyor
	CLK_EnableModuleClock(TMR0_MODULE);																								
	// Uart 0 Clock Ayarlari Enable yapiliyor	
	CLK_EnableModuleClock(UART0_MODULE);		
	CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HIRC_DIV2, 0);
	// Timer 0 Clock Ayarlari yapiliyor
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, MODULE_NoMsk);

	// Uart 0 Clock Ayarlari yapiliyor  
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
	// System Core Clock Guncelleme islemi yapiliyor.					


    /* Enable IP clock */
    CLK_EnableModuleClock(BPWM0_MODULE);
	  CLK_EnableModuleClock(BPWM1_MODULE);
    CLK_EnableModuleClock(EADC_MODULE);

    /* Set IP clock */
    CLK_SetModuleClock(BPWM0_MODULE, CLK_CLKSEL2_BPWM0SEL_PCLK0, MODULE_NoMsk);
		CLK_SetModuleClock(BPWM0_MODULE, CLK_CLKSEL2_BPWM0SEL_PCLK0, MODULE_NoMsk);
    CLK_SetModuleClock(EADC_MODULE, MODULE_NoMsk, CLK_CLKDIV0_EADC(1));

	SystemCoreClockUpdate();	

    SYS->GPA_MFPL = SYS_GPA_MFPL_PA0MFP_BPWM0_CH0;
    SYS->GPB_MFPH = SYS_GPB_MFPH_PB14MFP_EADC0_CH14;
    SYS->GPF_MFPL = SYS_GPF_MFPL_PF3MFP_BPWM1_CH0 | SYS_GPF_MFPL_PF1MFP_ICE_CLK | SYS_GPF_MFPL_PF0MFP_ICE_DAT;
    SYS->GPH_MFPL = SYS_GPH_MFPL_PH7MFP_GPIO | SYS_GPH_MFPL_PH6MFP_GPIO;
		
		// 'B' portunun 12. 13. pini RX TX olarak ayarlaniyor		
	SYS->GPB_MFPH = SYS_GPB_MFPH_PB13MFP_UART0_TXD | SYS_GPB_MFPH_PB12MFP_UART0_RXD;   

	// 'D' portunun 8. 9. pini SCL SDA olarak ayarlaniyor
	SYS->GPD_MFPH = SYS_GPD_MFPH_PD9MFP_I2C2_SCL | SYS_GPD_MFPH_PD8MFP_I2C2_SDA;		
 
    SYS_LockReg();
   
}

void EADC_POT_Okuma()
{

	EADC_START_CONV(EADC,BIT0);

	adc_data = EADC_GET_CONV_DATA(EADC,0);
	
	led_duty= ((float)adc_data*3.3f)/4095.0f;
	
	if (led_duty>3.3f) led_duty= 3.3f;
	if (led_duty<0.0f) led_duty= 0.0f;
	
	motor_duty= ((float)adc_data*100.0f)/4095.0f;
	
	if (motor_duty>100.0f) motor_duty= 100.0f;
	if (motor_duty<0.0f) motor_duty= 0.0f;
	
}


int main()
{
    SYS_Init();
	Uart0_Init();
	I2C2_Init();

	
	//led+motor
	GPIO_SetMode(PH,BIT6,GPIO_MODE_OUTPUT);
	GPIO_SetMode(PH,BIT7,GPIO_MODE_OUTPUT);

	EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);
	EADC_ConfigSampleModule(EADC,0,EADC_SOFTWARE_TRIGGER,14);
	
	BPWM_Start(BPWM0,BPWM_CH_0_MASK);
	BPWM_EnableOutput(BPWM0,BPWM_CH_0_MASK);
	
	BPWM_Start(BPWM1,BPWM_CH_0_MASK);
	BPWM_EnableOutput(BPWM1,BPWM_CH_0_MASK);
	
	PH6=0;
	PH7=1;
	
	//lcd kodlari
	
	GPIO_SetMode(PA,BIT9,GPIO_MODE_OUTPUT);			// 'A' Portunun 9. pini dijital cikis olarak ayarlaniyor.
	GPIO_SetMode(PA,BIT10,GPIO_MODE_OUTPUT);		// 'A' Portunun 10. pini dijital cikis olarak ayarlaniyor.
	GPIO_SetMode(PA,BIT11,GPIO_MODE_OUTPUT);    // 'A' Portunun 11. pini dijital cikis olarak ayarlaniyor.
	
	GPIO_SetMode(PB,BIT4,GPIO_MODE_OUTPUT);     // 'B' Portunun 4. pini dijital cikis olarak ayarlaniyor.
	GPIO_SetMode(PB,BIT5,GPIO_MODE_OUTPUT);			// 'B' Portunun 5. pini dijital cikis olarak ayarlaniyor.
	GPIO_SetMode(PB,BIT6,GPIO_MODE_OUTPUT);			// 'B' Portunun 6. pini dijital cikis olarak ayarlaniyor.
	GPIO_SetMode(PB,BIT7,GPIO_MODE_OUTPUT);			// 'B' Portunun 7. pini dijital cikis olarak ayarlaniyor.
	
	lcd_init();
	ekranTemizle();
	CLK_SysTickLongDelay(1000000);
	
	ekranaYaz(0,0,MSG);
	
	
    while(1){
		
			//led+motor
		EADC_POT_Okuma();
		BPWM_ConfigOutputChannel(BPWM0,0,50000,led_duty*10);
		BPWM_ConfigOutputChannel(BPWM1,0,50,motor_duty);
			
			//lcd yazdirma
		sprintf(lcd1602_yaz, "%.2f \n",motor_duty);
		ekranaYaz(0,1,(unsigned char*)lcd1602_yaz);

			
			
			
		}
}