#include "msp430g2553.h"
#include "LCD_NOKIA_5110.h"
#include "stdlib.h"
#include "string.h"
/*******************************************************************************\
*	                       DEFINE,BIEN TOAN CUC     		 	*
\*******************************************************************************/
#define GPS_BUFFER_SIZE 250
char  GPS_RX_byte[2], GPS_RX_Buffer[GPS_BUFFER_SIZE], GPS_Transfer_cplt = 0;  //Bien du lieu module GPS
char time[20], status[2], latitude[10], S_N[2], longitude[11], E_W[2], speed[20], dir[20], date[20]; //Du lieu trong chuoi nhan duoc
unsigned char GPS_ans_stt, GPS_send_error=0, GPS_receive_error=0; //Bien trang thai module GPS, bien trang thai error
float a, b;

/*******************************************************************************\
*	                       PROTOTYPE    		 	                *
\*******************************************************************************/
void Init_uart();
unsigned char serialRead();
void serialWrite(unsigned char c);
void serialWrite_String(const char *str);
void serialWrite_Int(unsigned long n);
/*******************************************************************************\
*	                       FUNCTION  		 	                 *
\*******************************************************************************/
void Init_uart(){
P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
P1SEL2 = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
UCA0CTL1 |= UCSSEL_2;                     // SMCLK
UCA0BR0 = 104;                            // 1MHz 9600
UCA0BR1 = 0;                              // 1MHz 9600
UCA0MCTL = UCBRS_1;                        // Modulation UCBRSx = 1
UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt   cho ngat o chan RX
}
unsigned char serialRead(){
	while(!(IFG2&UCA0RXIFG));   //USCI_A0 RX buffer ready ?
	return UCA0RXBUF;

}
void serialWrite(unsigned char c){
	while(!(IFG2&UCA0TXIFG));  // USCI_A0 TX buffer ready ?
	UCA0TXBUF=c; // TX
}
void serialWrite_String(const char *str){
	while(*str)
		serialWrite(*str++);
}
void serialWrite_Int(unsigned long n){
     unsigned char buffer[16];
     unsigned char i,j;
     if(n == 0){
    	 serialWrite('0');
          return;
     }
     for (i = 15; i > 0 && n > 0; i--){
          buffer[i] = (n%10)+'0';
          n /= 10;
     }

     for(j = i+1; j <= 15; j++){
    	 serialWrite(buffer[j]);
     }
}

/*******************************************************************************\
*	                      GPS		                         	*
\*******************************************************************************/
int Search_Char(unsigned char Char, char *Str, unsigned char Time, int Len)
{
    int   i=0;
    int  j=0;
    while((j<Time)&&(i<=Len))
    {
        if(Str[i] == Char)    j++;  
        i++;  
    }
    return (i-1);
}


// Ham xoa buffer GPS
void CLEAR_GPS_RX_Buffer() 
{
	for (int j=0; j<GPS_BUFFER_SIZE; j++)

		GPS_RX_Buffer[j] = 0; //clear Rx_Buffer before receiving new data
}

// Ham lay vi tri chuoi $GPRMC
int GPS_GetGPRMC()
{
	int k = 0;
	for(int k=0; k < (GPS_BUFFER_SIZE); k++)
	{
		if( (GPS_RX_Buffer[k] == 'M') && (GPS_RX_Buffer[k] == 'C') )
		{
			k = k - 5; // get the $
			return k;
		}
	}
	return k;
}

unsigned char GPS_DeviceInfo(char* time, char* status, char* latitude, char* S_N, char* longitude, char* E_W, char* speed, char* dir, char* date)
{
        int i = 0;
	int k = 0;
	int Temp1, Temp2;
	Temp1 = Search_Char(',',GPS_RX_Buffer,2,GPS_BUFFER_SIZE);	//Tim vi tri co ',' lan 2
	//printf("%d\n",Temp1);
	if(GPS_RX_Buffer[Temp1+1] == 'V'){
		return 0;
}
	else{
          //Lay vi do:
		Temp1 = Search_Char(',',GPS_RX_Buffer,3,GPS_BUFFER_SIZE);	 //Tim vi tri co ',' lan 3
		//printf("%d\n",Temp1);
		Temp2 = Search_Char(',',GPS_RX_Buffer,4,GPS_BUFFER_SIZE);	//Tim vi tri co ',' lan 4
		//printf("%d\n",Temp2);
		//Tach chuoi vi do
		k = 0;
		for(i = Temp1+1; i < Temp2; i++){
			latitude[k] = GPS_RX_Buffer[i];
			k++;	
		}
                latitude[i] = 0;
		a = atof(latitude);
          //Lay kinh do:
		Temp1 = Search_Char(',',GPS_RX_Buffer,5,GPS_BUFFER_SIZE);	 //Tim vi tri co ',' lan 5
		//printf("%d\n",Temp1);
		Temp2 = Search_Char(',',GPS_RX_Buffer,6,GPS_BUFFER_SIZE);	//Tim vi tri co ',' lan 6
		//printf("%d\n",Temp2);
		k = 0;
		for(i = Temp1+1 ; i < Temp2; i++){
			longitude[k] = GPS_RX_Buffer[i];
			k++;	
		}
                longitude[i] = 0;
	        b = atof(longitude);
	return 1;	
	}
}





/*******************************************************************************\
*	                       MAIN 		                         	*
\*******************************************************************************/

void main (void) 
{ 
//cau hinh xung nhip

  WDTCTL = WDTPW + WDTHOLD;  //Stop Watchdog Timer 
BCSCTL1 = CALBC1_1MHZ;
DCOCTL = CALDCO_1MHZ; //chon thach anh dao dong noi la 1MHZ
 //cau hinh IO
P1DIR |= BIT6;        //led6 out
//cau hinh lib
LCD5110_Init(30,0,4); // khoi tao LCD
Init_uart();        //khoi tao uart
LCD5110_Clr();      //xoa man hinh lcd
LCD5110_GotoXY(0,0);
LCD5110_String("Thuong dep trai");
__delay_cycles(1000000);
while(1){
  for(int i = 0; i< GPS_BUFFER_SIZE; i++){
  	GPS_RX_Buffer[i] = serialRead();
  }
 GPS_ans_stt = GPS_DeviceInfo(time, status, latitude, S_N, longitude, E_W, speed, dir, date);
    if(GPS_ans_stt){
        __delay_cycles(10000);
        LCD5110_Clr();
        LCD5110_GotoXY(0,0);
        LCD5110_String("Vi do:");
        LCD5110_GotoXY(0,1);
        LCD5110_String(latitude);
        LCD5110_GotoXY(0,2);
        LCD5110_String("Kinh do:");
        LCD5110_GotoXY(0,3);
        LCD5110_String(longitude);
        P1OUT ^= BIT6;
         CLEAR_GPS_RX_Buffer();
    }
    else{
     // __delay_cycles(1000);
        LCD5110_Clr();
        LCD5110_GotoXY(0,0);
        LCD5110_String("Dang do.");
        __delay_cycles(1000);
        LCD5110_Clr();
        LCD5110_String("Dang do..");
        __delay_cycles(1000);
        LCD5110_Clr();
        LCD5110_String("Dang do..."); 
        
    }
  }
}
