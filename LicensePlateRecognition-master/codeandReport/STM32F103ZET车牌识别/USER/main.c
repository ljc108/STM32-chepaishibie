#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "string.h"
#include "ov7725.h"
#include "ov7670.h"
#include "tpad.h"
#include "timer.h"
#include "exti.h"
#include "usmart.h"

/*����ʶ�����*/
u16 color_save=0;//����һ�����ص�ֵ
u8 R=0,G=0,B=0;//��ɫ����
u8 R_a=0,G_a=0,B_a=0;//RGB�Ƚϵ���ֵ
u8 TableChangePoint_240[240]={0};//�����240��
u16 AA=0,BB=0; //���� ��RGBֵ
u8 Max_ChangePoint_240=0;//���������ʼ
u8 Min_ChangePoint_240=0;//��ĩ����,
u8 Max_bChangePoint=0;//��������ʼ
u8 Min_bChangePoint=0;//ĩ���� 
u8 flag_MaxMinCompare=0;//Max_aChangePoint_reset_1��Max_aChangePoint_reset�ı�־ 
u16 Min_blue=0;
u16 Max_blue=0;//���峵����ɫ����ĺ������ֵ����Сֵ
float V,S,H;//����HSVֵ
u8 TableChangePoint_240[240];//������������ֵ240��
u8 TableChangePoint_320[320];//������������ֵ320��
extern u8 Table[6300];//�����ַ��� ��10+26��*150 = 5400 �ֽ�
extern u8 talble_0[150];//�ַ�3,������
extern u8 table_yu[32];//����
extern u8 table_min[32];//����
extern u8 table_lu[32];//³��
extern u8 table_zhe[32];//����
extern u8 table_shan[32];//����
extern u8 table_cuan[32];//����
u8 table_char[36]={0,1,2,3,4,5,6,7,8,9,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',};
u8 table_char_char[36]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',};
u16 k1,kk1,k2,kk2,k3,kk3,k4,kk4,k5,kk5,k6,kk6,k7,kk7,k8,kk8;//�˸��ַ��߽�
u8 table_picture[150];//���屣��ͼƬ�����飬��������ͷ���ص�����
	//150���ֽڣ�ÿ���ֽڶ���8bit,
	u8 table_cardMeasure[7];//�����ĳ��ƽ��
u8 ways0_8=0;


#define  OV7725 1
#define  OV7670 2

//����OV7725��������װ��ʽԭ��,OV7725_WINDOW_WIDTH�൱��LCD�ĸ߶ȣ�OV7725_WINDOW_HEIGHT�൱��LCD�Ŀ��
//ע�⣺�˺궨��ֻ��OV7725��Ч
#define  OV7725_WINDOW_WIDTH		320 // <=320
#define  OV7725_WINDOW_HEIGHT		240 // <=240


const u8*LMODE_TBL[6]={"Auto","Sunny","Cloudy","Office","Home","Night"};//6�ֹ���ģʽ	    
const u8*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7����Ч 
extern u8 ov_sta;	//��exit.c�� �涨��
extern u8 ov_frame;	//��timer.c���涨�� 

void ChangePoint_Analysis_320()//��ɫ�����У�320��������,���TableChangePoint_320[b]���
{								//(�ȶ�ֵ�������жϰ׵������=0���Ƿָ��ߣ�
	u16 a,b,num_color;
	u8 R1,G1,B1;
	u8 Mid_ChangePoint_240;
	u8 max_R,max_G,max_B,min_R,min_G,min_B;
	u8 mid_R,mid_G,mid_B;
	
	max_R=0;max_G=0;max_B=0;
	min_R=30;min_G=60;min_B=30;
	
	Mid_ChangePoint_240=(Min_ChangePoint_240+Max_ChangePoint_240)/2;
	for(b=Min_blue;b<Max_blue;b++)
	{
		num_color=LCD_ReadPoint(b,Mid_ChangePoint_240);//��ȡ���أ������Ż��ٶ��д����� ��ɨ�跽��Ҳ���Ż����������ٶ�
		
		R1=num_color>>11;
		G1=(num_color>>5)&0x3F;
		B1=num_color&0x1F;
		
		if( (R1>10) && (G1>25) && (B1>15) && (R1<=30) && (G1<=60) && (B1<=30) )//��ֵ��,����ֵ��25.55.25���Ϻ�����ֵ��21,47,21��
		{
			if(max_R<R1) max_R=R1;//������ֵ����Сֵ
			if(max_G<G1) max_G=G1;
			if(max_B<B1) max_B=B1;
			
			if(min_R>R1) min_R=R1;
			if(min_G>G1) min_G=G1;
			if(min_B>B1) min_B=B1;		
		}
	}
	mid_R=(max_R+min_R)/2;
	mid_G=(max_G+min_G)/2;
	mid_B=(max_B+	min_B)/2;


	for(b=0;b<320;b++)//����������������������
	{
		TableChangePoint_320[b]=0;
	}
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)								
	{
		for(b=Min_blue+1;b<Max_blue;b++)
		{
			num_color=LCD_ReadPoint(b,a);//��ȡ���أ������Ż��ٶ��д����� ��ɨ�跽��Ҳ���Ż����������ٶ�
			
			R1=num_color>>11;
			G1=(num_color>>5)&0x3F;
			B1=num_color&0x1F;
			
			if((R1>=mid_R) && (G1>=mid_G) && (B1>=mid_B))//��ֵ��,����ֵ��25.55.25���Ϻ�����ֵ��21,47,21��
			{
				POINT_COLOR=WHITE;
				LCD_DrawPoint(b,a);
				TableChangePoint_320[b]++;//��ɫ�������+1
			}
			else
			{
				POINT_COLOR=BLACK;
				LCD_DrawPoint(b,a);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------�����������ʾ����

void ChangePoint_Show_320()//320�����������ʾ
{
		u16 a=0,b=0;
	for(a=0;a<320;a++)//��ʾ��Ӧ�ĺ��������								
	{ 
		POINT_COLOR=RED;
		if(TableChangePoint_320[a]==0)
		{
			LCD_DrawPoint(a,0);//�������ʾ����ɫ���
		}
		else
		{
			LCD_DrawPoint(a,TableChangePoint_320[a]);//�������ʾ����ɫ���
		}
		
	}
}

//-------------------------------------------------------------------------------------------------------------�ַ��ָ��

u8 ZhiFuFenGe()//�ַ��ָ�,���طָ���ַ������������жϺϷ���
{
	u16 a=0,b=0;
	u8 i=0;//ͳ�Ʒָ���ַ���������Ϊ9˵���ָ�����
 POINT_COLOR=BLUE;
	for(b=Max_blue;b>Min_blue;b--)
	{
					if(TableChangePoint_320[b]==0)//��϶�ָ�
					{
									for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//����--������
									{
											LCD_DrawPoint(b,a+1);
									}
									i++;b--;
									while(TableChangePoint_320[b]==0)
									{
										  b--;
										 if(b<=Min_blue) 
											break;
									}
					}
	}
	i--;
	LCD_ShowNum(30,200,i,2,16);//��ʾ�ָ���ַ�����+1��8������ֵ
	return i;
}

//-------------------------------------------------------------------------------------------------------------��һ�������� ͳһ25*50
//�ұߡ����
 void GuiYi(u16 k,u16 kk)//��һ�� 24*50
{
	u16 a=0,b=0,e=0;
	u16 num=0;//�����ȡ����
	u8 Mo=0,Yu=0;//ȡ����ȡģ
	u8 num1=0,num2=0,num3=0;
	u8 Mo_1=0;//
	u8 Min_240=0,Max_240=0;//����ַ����������
	
//	if((k-kk)<25)
	//ֻ���ַ��Ŀ��С��25���صĲſ���ִ���ַ���һ��
	if((k-kk)<25)
	{
		//����ַ�
		Min_240=Min_ChangePoint_240+1;
		Max_240=Max_ChangePoint_240-1;
		while(Min_240++)//����󣬵õ�: Min_240
		{
			for(b=kk+1;b<k;b++)//ɨ����һ�е�ʱ���������1���أ���ô�����˳���0�����Ǻ�ɫ�ģ���ʾ�ǳ�������                               
			{
				num=LCD_ReadPoint(b,Min_240);
				if(num) break;
			}
			if(num) break;
		}
		while(Max_240--)//����󣬵õ�: Max_240
		{
			for(b=kk+1;b<k;b++)//kk1��k1                                
			{
				num=LCD_ReadPoint(b,Max_240);
				if(num) break;
			}
			if(num) break;
		}
		Min_240-=1;
		Max_240+=2;
		//��ѡ�����ַ������½Ǻ����ϽǷֱ�һ���㣡��
		LCD_DrawPoint(kk,Min_240);
		LCD_DrawPoint(k,Max_240);

		//��ʾ���Ƶ�ͼƬ
		num3=0;
		for(a=Min_240+1;a<Max_240;a++)
		{
			num2=0;
			for(b=kk+1;b<k;b++)//kk1��k1
			{
				//��һ���ڿ򣡣���
				num=LCD_ReadPoint(b,a);
				POINT_COLOR=num;
				LCD_DrawPoint(271-(k-kk-1)+num2,191+num3);//��һ�еĵ㣬ÿ�λ���һ���㣬�в��䣬��+1��Ҳ���Ǻ��������������
				
				num2++;
			}
			num3++;
		}
		delay_ms(1000);
		//		��������������������������������������������������������������������������������������������������������������������������������������������������������������������������
		//�еļ�������
		num3=0;
		Mo=(24-(k-kk-1))/(k-kk-1);//�ذ����ֻȡ��������
		Yu=(24-(k-kk-1))%(k-kk-1);//ģ�������ȡģ������ȡ��
		if(Yu != 0) {
			Mo_1=24/Yu;//ƽ��Mo_1�����أ�����һ�����أ� Yu��������Ҫ��ԭͼ�Ŀ���ϲ�Yu�����ص���ܴﵽĿ���24������  
		}
// 		LCD_ShowNum(30,20,Mo,3,16);//��ʾģ		<������>
// 		LCD_ShowNum(70,20,Yu,3,16);//��ʾ��
// 		LCD_ShowNum(100,20,(k1-kk1),3,16);//��ʾ��ֵ

		for(a=Min_240+1;a<Max_240;a++)//��Ŵ�Ϊ25����
		{
			num2=0;//ÿ�δ��µ�һ�е�ͷ����ʼ
			Yu=(24-(k-kk-1))%(k-kk-1);//ģ�������ȡģ������ȡ��
			for(b=kk+1;b<k;b++)//kk1��k1 
			{
				//����ͼƬ��������ʾ����������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������
				//��κ����������Ǹ���ͼƬ��������ʾ����ָ��λ�ÿ�ʼ��ʾ��ÿ����ѭ������ʾһ�����ص㣬ÿһ����ѭ������ʾһ�е����ص㣡������
				num=LCD_ReadPoint(b,a);
				//�ڶ����ڿ�!!!
				POINT_COLOR=num;//������
				LCD_DrawPoint(271+num2,191+num3);
				num2++;
			//ͼƬ�������졪��������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������
				//1�����ͼƬ���25���أ����д������Mo=1,��ôÿ����ѭ���ͻ���ֵһ���㣬�����������ͼƬ�Ŀ��ֻ��26��һ�룬��width=13��
				Mo=(24-(k-kk-1))/(k-kk-1);//�ذ����ֻȡ��������
				while(Mo)
				{
					POINT_COLOR=num;
					LCD_DrawPoint(271+num2,191+num3);
					Mo--;
					num2++;
				}
				//һ�㲻���õ������ǲɼ����ĳ��ƺ��㣬���ɼ����ĳ��ƵĿ��С�ڹ涨��һ��Żᷢ��
				//2����Mo=1��ʱ��Yu��Ϊ0��,Yu��Ϊ0����ôMo=0����ͼƬ�Ŀ�Ȳ�Ϊ26��һ���ʱ��
				if(Yu!=0)
				{	
					if(((num2+1)%Mo_1==0) && (num2!=1))//�Ĳ���ĵط�7+1
					{
						POINT_COLOR=num;
						LCD_DrawPoint(271+num2,191+num3);
						Yu--;
						num2++;
					}
				}
			}//��ǰ����ɲ�ֵ
			num3++;//������һ��
		}
		delay_ms(1000);
		POINT_COLOR=0x07E0;
		LCD_DrawPoint(271,191);//��ǵ㣬4������
		LCD_DrawPoint(271,240);
		LCD_DrawPoint(295,191);
		LCD_DrawPoint(295,240);
//������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������
		if((Max_240-Min_240)<50)
		{
			Mo=(50-(Max_240-Min_240+1))/(Max_240-Min_240+1);//ȡģ
			Yu=(50-(Max_240-Min_240+1))%(Max_240-Min_240+1);//ȡ��
			Mo_1=50/Yu;//ƽ��Mo_1�����أ�����һ������
			
// 			LCD_ShowNum(30,170,Mo,3);//					<������>
// 			LCD_ShowNum(70,170,Yu,3);//
// 			LCD_ShowNum(100,170,Max_ChangePoint_240-Min_ChangePoint_240,3);//
			num2=0;
			for(a=0;a<(Max_240-Min_240);a++)//����ͼ��,���Ƿ�Χ�Ƿ���Ҫ����������
			{
				for(b=271;b<=295;b++)//271��ʼ���ƣ�295�Ž���
				{
					//�������ڿ򣡣���
					num=LCD_ReadPoint(b,a+191);
					POINT_COLOR=num;
					LCD_DrawPoint(b+25,191+num2);//��������ֵ,�ڶ����ڿ�
				}
				num2++;
				while(Mo)
				{
					for(b=271;b<=295;b++)//271��ʼ���ƣ�295�Ž���
					{
						num=LCD_ReadPoint(b,a+191);
						POINT_COLOR=num;
						LCD_DrawPoint(b+25,191+num2+a);//��������ֵ
					}
					Mo--;
					num2++;
				}
				if(Yu!=0)
				{
					if((((num2+1) % Mo_1)==0)&& (num2!=1))
					{
						for(b=271;b<=295;b++)//271��ʼ���ƣ�295�Ž���
						{
							num=LCD_ReadPoint(b,a+191);
							POINT_COLOR=num;
							LCD_DrawPoint(b+25,191+num2);//��������ֵ
						}
						Yu--;
						num2++;
					}
				}					
			}
		}
		POINT_COLOR=RED;
		LCD_DrawPoint(320,191);//��ǵ㣬1������
	}
}

void String_Picture()//���鵽ͼƬ
{
	u16 a=0,b=0,e=0,num1=0;
	
	for(a=0;a<50;a++)//50��
	{
		for(b=0;b<24;b++)//24��
		{
			if(talble_0[b/8+a*3]&(1<<(7-b%8)))
			{
				num1=0xffff;
			}
			else
			{
				num1=0x0000;
			}
		  POINT_COLOR=num1;
			LCD_DrawPoint(b+296,a+191);//����
		}				
	}	
}
//------------------------------------------------------------------------------------------------------ͼƬ->����table_picture  ���޸�  
void Picture_String()//ͼƬ->����table_picture      �и�BUG���ȷ��Ű�
{
	u16 a=0,b=0,num1=0,move=0;
	for(a=0;a<150;a++)//����
	{
		table_picture[a]=0x00;	//u8 table_picture[150];
	}	
	for(a=0;a<50;a++)//50��
	{
		for(b=0;b<24;b++)//24��
		{
		
			num1=LCD_ReadPoint(b+296,a+191);
			if(num1==0xffff)//��ɫΪ��ɫ�����ص�
			{
				table_picture[b/8+a*3]|=(1<<(7-b%8));//ÿ��7/8����ȡһ�����ص㣡��
				 //(1<<(7-b%8))   ��"1" �任�ڵ��ֽ��е���Ӧλ���ϡ�  
			}
		}
	}
}


void ChangePoint_Show_240()//240�����������ʾ
{
	u32 a=0,b=0;
	    POINT_COLOR=RED;
				for(a=0;a<240;a++)//�����ο���10��20��30 ������
				{
					LCD_DrawPoint(10,a);//10
					LCD_DrawPoint(20,a);//20
					LCD_DrawPoint(30,a);//30
				}
	
				for(a=0;a<240;a++)//��ʾ��Ӧ�ĺ��������								
				{
								LCD_DrawPoint(TableChangePoint_240[a],a);//�������ʾ����ɫ���
							if(TableChangePoint_240[a]>=15)					//������������ֵ���趨       ��ֵ����3-��1��
							{
								   POINT_COLOR=GREEN;
										for(b=35;b<40;b++)						//��ʾ�ﵽ��ֵ��׼�ĵ�
										{
											LCD_DrawPoint(b,a);//Green			
										}
							}
				}
}
void ChangePoint_Analysis_240()//240��������
{
	u16 a=0,b=0; 
	Min_ChangePoint_240=240;
	Max_ChangePoint_240=0;
	
	for(a=0;a<240;a++)//240ɨ��	����ȡ������ֵ	��Min_ChangePoint_240��Max_ChangePoint_240				
	{
		while(TableChangePoint_240[a]<=15)									//��ֵ����3-��2��
		{
			a++;
		}
		Min_ChangePoint_240=a;
		
		while(TableChangePoint_240[a]>15)									//��ֵ����3-��3��
		{
			a++;
		}
		Max_ChangePoint_240=a;
		if(Max_ChangePoint_240-Min_ChangePoint_240>=15) //˵��ɨ������ û�г�����Ϣ
			a=240;//��������ֵ   	//��ֵ����2-��1��
	}
	Min_ChangePoint_240=Min_ChangePoint_240-5;//����΢��3����
	Max_ChangePoint_240=Max_ChangePoint_240+5;//����΢��2����
	POINT_COLOR=BLUE;
	for(a=30;a<280;a++)//��ʾ�Ͻ���				
	{
		LCD_DrawPoint(a,Max_ChangePoint_240);//��ɫ
	}
	for(a=30;a<280;a++)//��ʾ�½���						
	{
		LCD_DrawPoint(a,Min_ChangePoint_240); //��ɫ
	}
	POINT_COLOR=RED;
	for(a=30;a<280;a++)//��ʾ50,�ο�50����λ�ô�������λ�ò�Ҫ��������ߣ���ò����ַ��Ĺ�һ������						
	{
		LCD_DrawPoint(a,Min_ChangePoint_240+50); //��ɫ
	}
	
	flag_MaxMinCompare=1;
	
	if(Min_ChangePoint_240>Max_ChangePoint_240)//�жϺϷ���1����Сֵ>���ֵ
	{
		flag_MaxMinCompare=0;
	}
	if(Min_ChangePoint_240==240||Max_ChangePoint_240==0)//�жϺϷ���2��ֵû�����¸�ֵ
	{
		flag_MaxMinCompare=0;
	}
	if(Max_ChangePoint_240-Min_ChangePoint_240<15)		//�жϺϷ���3��			//��ֵ����2-��2��
	{
		flag_MaxMinCompare=0;
	}
}
void RGB_HSV(u16 num)//RGB565תHSV
{
	float max,min;
	u8 r=0,g=0,b=0;
	r=(num>>11)*255/31;g=((num>>5)&0x3f)*255/63;b=(num&0x1f)*255/31;
	
	max=r;min=r;
	if(g>=max)max=g;
	if(b>=max)max=b;
	if(g<=min)min=g;
	if(b<=min)min=b;
	
	V=100*max/255;//ת��Ϊ�ٷֱ�
	S=100*(max-min)/max;//����100����ʾ
	if(max==r) H=(g-b)/(max-min)*60;
	if(max==g) H=120+(b-r)/(max-min)*60;
	if(max==b) H=240+(r-g)/(max-min)*60;
	if(H<0) H=H+360;
}
void ChangePoint_Analysis_Blue()//320��ɫ�������,���ö�ȡ���أ��ý��Min_blue,Max_blue
{
	u16 a=0,b=0,num_color=0;
	u16 min_320=0,max_320=0;//���е���С�����ֵ
	
	Min_blue=0;Max_blue=320;
	min_320=320;max_320=0;
	
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)								
	{
		for(b=30;b<290;b++)//���õ�320    for(b=30;b<320;b++)
		{
			num_color=LCD_ReadPoint(b,a);//��ȡ���أ������Ż��ٶ��д����� ��ɨ�跽��Ҳ���Ż����������ٶ�
			RGB_HSV(num_color);//RGB565תHSV
			//			if( 245>H && H>190 && 1>S && S>0.35 && 1>V && V>0.3)//HSV ��ֵ
			if( 245>H && H>190 && 60>S && S>15 && 100>V && V>45)//HSV ��ֵ
		
			{
				if(b<min_320)//��ú����Min��Maxֵ������ɫ���Ƶ����ұ߽�
				{
					min_320=b;
				}
				if(b>max_320)
				{
					max_320=b;
				}
			}
		}
	}
	Min_blue=min_320;//��ȡ���е����ֵ//����һ��
	Max_blue=max_320-5;//��ȡ���е���Сֵ//����һ��
	POINT_COLOR=RED;
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//��ʾ�����				
	{
		LCD_DrawPoint(Min_blue,a);// 
	}
	POINT_COLOR=BLUE;
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//��ʾ�ҽ���					
	{
		LCD_DrawPoint(Max_blue,a);
	}
//  delay_ms(6000);
}
 u8 MoShiShiBie_All(u8 begin,u8 end)//�ַ�ƥ�䣬ģʽʶ��,ѡ����ƥ��begin-end            !!
{
//�۳����������ַ�ģ��ȥ�ȶԣ���ô�ȶԵģ�
//Result=MoShiShiBie_All(0,36);
u16 Compare_num=0;
	int num_save=0;
	u8 a=0,b=0,e=0,a_save=0,st1=0,st2=0,s1=0,s2=0;
	int num1=0;
	for(a=begin;a<end;a++)//0-35
	{
					num1=0;
					for(b=0;b<150;b++)
					{
						//table_picture��36*150=5400���ֽڣ�ÿ���ַ�150���ֽڣ���36���ֽڣ�
									st1=table_picture[b];//ȡ����ǰ1���ַ�����ĵ�b���ֽ�
									st2=Table[150*a+b];//ȡ��ģ����ж�Ӧ��1���ַ�����ĵ�b���ֽ�
									for(e=0;e<8;e++)
									{
										s1=st1&(1<<e);//�ֱ�ȡ��8λ���бȽ�
										s2=st2&(1<<e);
										if(s1==s2) num1++;
										if(s1!=s2) num1--;
										//num�ķ�Χ�ǣ�0-1200,0��ƥ�䣬1200��ȫƥ��
									}
					}
				if(num1>num_save)
				{
					num_save=num1;
					a_save=a;
				}
				LCD_ShowNum(50,220,a_save,2,16);				//��ʾƥ����ַ���"a"			<������>
				LCD_ShowNum(70,220,num1,4,16);			//��ʾƥ�����ȷ������
				LCD_ShowNum(120,220,num_save,4,16);//ƥ������ֵ��ʾ					
	}
	return a_save;
}
void WordShow(u8 num,u16 x,u16 y)//��ʾ����16*16
{
	u16 a,b,num1;
	u8 table1[32];
	if(num==1)
	{
		for(a=0;a<32;a++)
		{
			table1[a]=table_yu[a];	
		}		
	}
	if(num==2)
	{
		for(a=0;a<32;a++)
		{
			table1[a]=table_min[a];	
		}		
	}
	if(num==3)
	{
		for(a=0;a<3;a++)
		{
			table1[a]=table_lu[a];	
		}		
	}
	if(num==4)
	{
		for(a=0;a<32;a++)
		{
			table1[a]=table_zhe[a];	
		}		
	}
	if(num==5)
	{
		for(a=0;a<32;a++)
		{
			table1[a]=table_shan[a];	
		}		
	}
	if(num==6)
	{
		for(a=0;a<32;a++)
		{
			table1[a]=table_cuan[a];	
		}		
	}
	for(a=0;a<16;a++)
	{
		for(b=0;b<16;b++)
		{
			if(table1[b/8+a*2]&(1<<(7-b%8)))
			{
				num1=0xffff;
			}
			else
			{
				num1=0x0000;
			}
			POINT_COLOR=num1;
			LCD_DrawPoint(b+x,a+y);//����
		}				
	}	
}

void ZhiFuShiBie()//�ַ�ʶ��
{
	u16 a,b,u,i;
	u8 Result;//ʶ����
	for(b=Max_blue-1;b>Min_blue;b--)//��������ʶ�𣬻�ȡ�����ַ��� K KKֵ, ���ַ��߽�
	{
		while(TableChangePoint_320[b]==0)b--;//ȡ��1���ַ�
		k1=b+1;//+1
		while(TableChangePoint_320[b]>0) b-- ;
		kk1=b;
		if((k1-kk1)<4)//ʡ�Ե����������ص�λ��
		{
			while(TableChangePoint_320[b]==0) b--;//
			k1=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk1=b;
		}
	 
		while(TableChangePoint_320[b]==0) b--;//ȡ��2���ַ�
		k2=b+1;
		while(TableChangePoint_320[b]>0) b--;
		kk2=b;
		if((k2-kk2)<4)//ʡ�Ե���3�����ص�λ��
		{
			while(TableChangePoint_320[b]==0) b--;//
			k2=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk2=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//ȡ��3���ַ�
		k3=b+1;//+1
		while(TableChangePoint_320[b]>0) b--;
		kk3=b;
		if((k3-kk3)<4)//ʡ�Ե���3�����ص�λ��
		{
			while(TableChangePoint_320[b]==0) b--;//
			k3=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk3=b;
		}
	 
		while(TableChangePoint_320[b]==0) b--;//ȡ��4���ַ�
		k4=b+1;
		while(TableChangePoint_320[b]>0) b--;
		kk4=b;
		if((k4-kk4)<4)//ʡ�Ե���3�����ص�λ��
		{
			while(TableChangePoint_320[b]==0) b--;//
			k4=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk4=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//ȡ��5���ַ�
		k5=b+1;//+1
		while(TableChangePoint_320[b]>0) b--;
		kk5=b;
		if((k5-kk5)<4)//ʡ�Ե���3�����ص�λ��
		{
			while(TableChangePoint_320[b]==0) b--;//
			k5=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk5=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//ȡ��6���ַ�
		k6=b+1;
		while(TableChangePoint_320[b]>0) b--;
		kk6=b;
		if((k6-kk6)<4)
		{
			while(TableChangePoint_320[b]==0)b--;
			k6=b+1;
			while(TableChangePoint_320[b]>0)b--;
			kk6=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//ȡ��7���ַ�
		k7=b+1;//+1
		while(TableChangePoint_320[b]>0) b--;
		kk7=b;
		if((k7-kk7)<4)//ʡ�Ե���3�����ص�λ��
		{
			while(TableChangePoint_320[b]==0) b--;//
			k7=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk7=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//ȡ�ڰ˸��ַ�
		k8=b+1;
 		while(TableChangePoint_320[b]>0) 
		{
			if(b<=Min_blue)
			{
				break;
			}
			b--;
		}
		kk8=b;
		b=Min_blue;//�Է���һ��������forѭ������
	}
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//����
	{
		POINT_COLOR=BLUE;
		LCD_DrawPoint(k1,a+1);
		LCD_DrawPoint(kk1,a+1);
		LCD_DrawPoint(k2,a+1);
		LCD_DrawPoint(kk2,a+1);
		LCD_DrawPoint(k3,a+1);
		LCD_DrawPoint(kk3,a+1);
		LCD_DrawPoint(k4,a+1);
		LCD_DrawPoint(kk4,a+1);
		LCD_DrawPoint(k5,a+1);
		LCD_DrawPoint(kk5,a+1);
		LCD_DrawPoint(k6,a+1);
		LCD_DrawPoint(kk6,a+1);
		LCD_DrawPoint(k7,a+1);
		LCD_DrawPoint(kk7,a+1);
		LCD_DrawPoint(k8,a+1);
		LCD_DrawPoint(kk8,a+1);
	}
//��һ��������СΪ25*50
//��1���ַ���
	GuiYi(k1,kk1);//��һ�� 24*50
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(0,36);//�ַ�ƥ�䣬ģʽʶ��,����a,0<= a <36
	if(Result<10)
	{
		LCD_ShowNum(240,220,table_char[Result],1,16);
	}
	else
	{
		POINT_COLOR=GREEN;
		LCD_ShowChar(240,220,table_char[Result],16,0);
	}
	table_cardMeasure[6]=Result;//����ʶ��ĳ��ƽ��
	
//��2���ַ���
	GuiYi(k2,kk2);//��һ�� 25*50
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(0,36);//�ַ�ƥ�䣬ģʽʶ��
//	printf("RESULT 2 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(230,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(230,220,table_char[Result],16,0);
	}
	table_cardMeasure[5]=Result;//����ʶ��ĳ��ƽ��
	
	GuiYi(k3,kk3);//��һ�� 25*50
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(0,36);//�ַ�ƥ�䣬ģʽʶ��
//	printf("RESULT 3 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(220,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(220,220,table_char[Result],16,0);
	}
	table_cardMeasure[4]=Result;//����ʶ��ĳ��ƽ��
	
	GuiYi(k4,kk4);//��һ�� 25*50
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(0,36);//�ַ�ƥ�䣬ģʽʶ��
//	printf("RESULT 4 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(210,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(210,220,table_char[Result],16,0);
	}
	table_cardMeasure[3]=Result;//����ʶ��ĳ��ƽ��
	
	GuiYi(k5,kk5);//��һ�� 25*50
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(0,36);//�ַ�ƥ�䣬ģʽʶ��
//	printf("RESULT 5 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(200,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(200,220,table_char[Result],16,0);
	}
	table_cardMeasure[2]=Result;//����ʶ��ĳ��ƽ��
	LCD_ShowChar(190,220,'.',16,0);

	GuiYi(k7,kk7);//��һ�� 25*50
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(10,36);//�ַ�ƥ�䣬ģʽʶ��ֻƥ����ĸ
	if(Result<10)
	{
		LCD_ShowNum(180,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(180,220,table_char[Result],16,0);
	}
	table_cardMeasure[1]=Result;//����ʶ��ĳ��ƽ��
	
	GuiYi(k8,kk8);//��һ�� 25*50					���һ�����֣�����ʶ��
	Picture_String();//ͼƬ->����
	Result=MoShiShiBie_All(36,42);//�ַ�ƥ�䣬ƥ�人��
	WordShow(Result-35,160,220);//��ʾ����
	table_cardMeasure[0]=Result-35;//����ʶ��ĳ��ƽ��
		//���ڷ��ͳ�����Ϣ     //
	if(Result==36)
	{
		printf("��");
	}
	else if(Result==37)
	{
		printf("��");
	}
	else if(Result==38)
	{
		printf("��");
	}
	else if(Result==39)
	{
		printf("��");
	}
	else if(Result==40)
	{
	 printf("��");
	}
	else if(Result==41)
	{
		printf("��");
	}
	printf("%c",table_char_char[table_cardMeasure[1]]);
	printf("."); 
	printf("%c",table_char_char[table_cardMeasure[2]]);
	printf("%c",table_char_char[table_cardMeasure[3]]);
	printf("%c",table_char_char[table_cardMeasure[4]]);
	printf("%c",table_char_char[table_cardMeasure[5]]);
	printf("%c",table_char_char[table_cardMeasure[6]]);
  while(1);


}
//����LCD��ʾ(OV7725)
void Data_LCD_ColorChange(void)
{
	u32 i=0,j=0;
 	u16 color=0;	 
  u32 a=0,b=0;
	
	for(a=0;a<240;a++)//����������������������
	{
		TableChangePoint_240[a]=0;
	}
	Min_blue=320;//��ʼ����¼��ɫ���������ֵ
	Max_blue=0;
	
 
	LCD_Scan_Dir(ways0_8);//���ϵ���,������
	LCD_Set_Window(0,0,OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT);//����ʾ�������õ���Ļ����
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
	 
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WRST=0;
	OV7725_WRST=1;
	OV7725_WREN=1;
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1);
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WREN=0;
	OV7725_RRST=0;				//��ʼ��λ��ָ�� 
	OV7725_RCK_L;
	OV7725_RCK_H;
	OV7725_RCK_L;
	OV7725_RRST=1;				//��λ��ָ����� 
	OV7725_RCK_H; 
	
		for(a=0;a<OV7725_WINDOW_HEIGHT;a++)
		{
					for(b=0;b<OV7725_WINDOW_WIDTH;b++)
					{
								OV7725_RCK_L;
								AA=GPIOC->IDR&0XFF;	//������
								OV7725_RCK_H; 
										
								OV7725_RCK_L;
								BB=GPIOC->IDR&0XFF;	//������
								OV7725_RCK_H; 
				        color=(AA<<8)|BB;
				 
								R=color>>11;
								G=(color>>5)&0x3f;
								B=color&0x1f;
//						
								if((R>R_a) && (G>=G_a) && (B>=B_a))//��ֵ��,����ֵ��25.55.25���Ϻ�����ֵ��21,47,21��
								{
									 color=0xffff;	//��ɫ
								}
								else
								{
									 color=0x0000;	//��ɫ
								}
//						
								if(color!=color_save)//�����
								{
									TableChangePoint_240[a]++;		//������������+1
								}
						   color_save=color;//��������ֵ������һ���жϺͱȽ�
 
					    color=(AA<<8)|BB;//��ԭɫ��
								
				     LCD->LCD_RAM=color;  
								
						}
		}
//		LCD_Scan_Dir(ways0_8);	//�ָ�Ĭ��ɨ�跽�� 
		ChangePoint_Show_240();
		ChangePoint_Analysis_240();
 if(flag_MaxMinCompare==1)//�����ɸѡ�ɹ�
	{
		ChangePoint_Analysis_Blue();//320��ɫ�������,���ö�ȡ���أ��ý��Min_blue,Max_blue
		if(Min_blue>Max_blue) 
			flag_MaxMinCompare=0;//���к������ж�1
		if((Min_blue>290)||(Max_blue>290)) 
			flag_MaxMinCompare=0;//���к������ж�2
	}
		if(flag_MaxMinCompare==1)//�����ɸѡ�ɹ�
	{
		ChangePoint_Analysis_320();//��ɫ�����У�320��������,��ã�TableChangePoint_320[b]���
		ChangePoint_Show_320();//320�����������ʾ
	
		i=ZhiFuFenGe();
		printf("�ָ�� %d ���ַ� \r\n",i);
		if(i>=8)//�ַ��ָ�,���طָ���ַ������������жϺϷ���
		{
			ZhiFuShiBie();//�ַ�ʶ��
		}
    else
		{
			LCD_Clear(0x0000);//��������ʾMeasure Faill
			LCD_ShowChar(8*1,200,'M',16,0);
			LCD_ShowChar(8*2,200,'e',16,0);
			LCD_ShowChar(8*3,200,'a',16,0);
			LCD_ShowChar(8*4,200,'s',16,0);
			LCD_ShowChar(8*5,200,'u',16,0);
			LCD_ShowChar(8*6,200,'r',16,0);
			LCD_ShowChar(8*7,200,'e',16,0);
			LCD_ShowChar(8*9,200,'F',16,0);
			LCD_ShowChar(8*10,200,'a',16,0);
			LCD_ShowChar(8*11,200,'i',16,0);
			LCD_ShowChar(8*16,200,'l',16,0);
			LCD_ShowChar(8*13,200,'l',16,0);
			delay_ms(800);
		}
	}
	
  
}
//����LCD��ʾ(OV7725)
void Data_LCD_ColorChange_Test(void)
{
	u32 i=0,j=0;
 	u16 color=0;	 
  u32 a=0,b=0;
	
	for(a=0;a<240;a++)//����������������������
	{
		TableChangePoint_240[a]=0;
	}
	Min_blue=320;//��ʼ����¼��ɫ���������ֵ
	Max_blue=0;
	
 
		LCD_Scan_Dir(ways0_8);//���ϵ���,������
		LCD_Set_Window(0,0,OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT);//����ʾ�������õ���Ļ����
//		if(lcddev.id==0X1963)
//		LCD_Set_Window((lcddev.width-OV7725_WINDOW_WIDTH)/2,(lcddev.height-OV7725_WINDOW_HEIGHT)/2,OV7725_WINDOW_HEIGHT,OV7725_WINDOW_WIDTH);//����ʾ�������õ���Ļ����
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
	 
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WRST=0;
	OV7725_WRST=1;
	OV7725_WREN=1;
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1);
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WREN=0;
		
		OV7725_RRST=0;				//��ʼ��λ��ָ�� 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//��λ��ָ����� 
		OV7725_RCK_H; 
		
		for(a=0;a<OV7725_WINDOW_HEIGHT;a++)
		{
					for(b=0;b<OV7725_WINDOW_WIDTH;b++)
					{
								OV7725_RCK_L;
								AA=GPIOC->IDR&0XFF;	//������
								OV7725_RCK_H; 
										
								OV7725_RCK_L;
								BB=GPIOC->IDR&0XFF;	//������
								OV7725_RCK_H; 
				
				        color=(AA<<8)|BB;
				 
								R=color>>11;
								G=(color>>5)&0x3f;
								B=color&0x1f;
//						
								if((R>R_a) && (G>=G_a) && (B>=B_a))//��ֵ��,����ֵ��25.55.25���Ϻ�����ֵ��21,47,21��
								{
									 color=0xffff;
								}
								else
								{
									 color=0x0000;
								}
//						
								if(color!=color_save)//�����
								{
									TableChangePoint_240[a]++;		//������������+1
								}
						   color_save=color;//��������ֵ������һ���жϺͱȽ�
 
					    color=(AA<<8)|BB;//��ԭɫ��
								
				     LCD->LCD_RAM=color;  
								
						}
		}
		LCD_Scan_Dir(ways0_8);	//�ָ�Ĭ��ɨ�跽�� 
		ChangePoint_Show_240();
		ChangePoint_Analysis_240();
  
	 
	}
	
  
 
void PA8_in()//PA8����Ϊ����
{
	GPIO_InitTypeDef gpio_init_struct;//�ṹ��
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;//��������
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &gpio_init_struct);
}
 
 int main(void)
{
	u8 sensor=0;
	u8 key;
 	u8 i=0;	 
  unsigned int num=0;    
	u8 msgbuf[15];//��Ϣ������
	u8 tm=0;
	u8 lightmode=0,effect=0;
	s8 saturation=0,brightness=0,contrast=0;
	 
	delay_init();	    	 	//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 		//���ڳ�ʼ��Ϊ 115200
 	usmart_dev.init(72);		//��ʼ��USMART		
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD  
	//TPAD_Init(6);				//����������ʼ�� 
 	POINT_COLOR=RED;			//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"WarShip STM32");	
	LCD_ShowString(30,70,200,16,16,"OV7725_OV7670 TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2017/11/1"); 
	LCD_ShowString(30,130,200,16,16,"KEY0:Light Mode");
	LCD_ShowString(30,150,200,16,16,"KEY1:Saturation");
	LCD_ShowString(30,170,200,16,16,"KEY2:Brightness");
	LCD_ShowString(30,190,200,16,16,"KEY_UP:Contrast");
	LCD_ShowString(30,210,200,16,16,"TPAD:Effects");	 
  LCD_ShowString(30,230,200,16,16,"OV7725_OV7670 Init...");	
 //��ʼ��OV7725_OV7670
		if(OV7725_Init()==0)
		{
			sensor=OV7725;
			LCD_ShowString(30,230,200,16,16,"OV7725 Init OK       ");
 
			OV7725_Window_Set(OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT,0);//QVGAģʽ���
			OV7725_CS=0;
		}
 
	//TIM6_Int_Init(10000,7199);	//10Khz����Ƶ��,1�����ж�									  
	//EXTI8_Init();				//ʹ�ܶ�ʱ������			
  PA8_in();		
	LCD_Clear(BLACK);
		
	R_a=24;
	G_a=53;
	B_a=24;
			while(1)
			{
				if(num<=1)
				{
				Data_LCD_ColorChange();//���Ʋⶨ
				}
				if(num>1)
				{
					Data_LCD_ColorChange_Test();// 
					LCD_ShowNum(30,220,21-num,2,16);//
					delay_ms(20);//��ʱ������ʱ
				}
				if(num==20)
				{
					num=0;
				}
				num++;
			}
}













