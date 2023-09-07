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

/*车牌识别变量*/
u16 color_save=0;//保存一个像素的值
u8 R=0,G=0,B=0;//颜色分量
u8 R_a=0,G_a=0,B_a=0;//RGB比较的阈值
u8 TableChangePoint_240[240]={0};//跳变点240个
u16 AA=0,BB=0; //像素 的RGB值
u8 Max_ChangePoint_240=0;//跳变点纵轴始
u8 Min_ChangePoint_240=0;//、末坐标,
u8 Max_bChangePoint=0;//跳变点横轴始
u8 Min_bChangePoint=0;//末坐标 
u8 flag_MaxMinCompare=0;//Max_aChangePoint_reset_1和Max_aChangePoint_reset的标志 
u16 Min_blue=0;
u16 Max_blue=0;//定义车牌蓝色区域的横向最大值和最小值
float V,S,H;//定义HSV值
u8 TableChangePoint_240[240];//横向跳变点最大值240个
u8 TableChangePoint_320[320];//纵向跳变点最大值320个
extern u8 Table[6300];//所有字符集 （10+26）*150 = 5400 字节
extern u8 talble_0[150];//字符3,测试用
extern u8 table_yu[32];//渝字
extern u8 table_min[32];//闽字
extern u8 table_lu[32];//鲁字
extern u8 table_zhe[32];//浙字
extern u8 table_shan[32];//陕字
extern u8 table_cuan[32];//川字
u8 table_char[36]={0,1,2,3,4,5,6,7,8,9,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',};
u8 table_char_char[36]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',};
u16 k1,kk1,k2,kk2,k3,kk3,k4,kk4,k5,kk5,k6,kk6,k7,kk7,k8,kk8;//八个字符边界
u8 table_picture[150];//定义保存图片的数组，储存摄像头返回的数据
	//150个字节，每个字节都有8bit,
	u8 table_cardMeasure[7];//测量的车牌结果
u8 ways0_8=0;


#define  OV7725 1
#define  OV7670 2

//由于OV7725传感器安装方式原因,OV7725_WINDOW_WIDTH相当于LCD的高度，OV7725_WINDOW_HEIGHT相当于LCD的宽度
//注意：此宏定义只对OV7725有效
#define  OV7725_WINDOW_WIDTH		320 // <=320
#define  OV7725_WINDOW_HEIGHT		240 // <=240


const u8*LMODE_TBL[6]={"Auto","Sunny","Cloudy","Office","Home","Night"};//6种光照模式	    
const u8*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7种特效 
extern u8 ov_sta;	//在exit.c里 面定义
extern u8 ov_frame;	//在timer.c里面定义 

void ChangePoint_Analysis_320()//蓝色区域中，320跳变点分析,获得TableChangePoint_320[b]结果
{								//(先二值化，再判断白点个数，=0则是分割线）
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
		num_color=LCD_ReadPoint(b,Mid_ChangePoint_240);//读取像素，代码优化速度有待提升 ？扫描方法也可优化，以提升速度
		
		R1=num_color>>11;
		G1=(num_color>>5)&0x3F;
		B1=num_color&0x1F;
		
		if( (R1>10) && (G1>25) && (B1>15) && (R1<=30) && (G1<=60) && (B1<=30) )//二值化,高阈值：25.55.25，较合适阈值（21,47,21）
		{
			if(max_R<R1) max_R=R1;//获得最大值和最小值
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


	for(b=0;b<320;b++)//各行跳变点计数，数组清零
	{
		TableChangePoint_320[b]=0;
	}
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)								
	{
		for(b=Min_blue+1;b<Max_blue;b++)
		{
			num_color=LCD_ReadPoint(b,a);//读取像素，代码优化速度有待提升 ？扫描方法也可优化，以提升速度
			
			R1=num_color>>11;
			G1=(num_color>>5)&0x3F;
			B1=num_color&0x1F;
			
			if((R1>=mid_R) && (G1>=mid_G) && (B1>=mid_B))//二值化,高阈值：25.55.25，较合适阈值（21,47,21）
			{
				POINT_COLOR=WHITE;
				LCD_DrawPoint(b,a);
				TableChangePoint_320[b]++;//白色，跳变点+1
			}
			else
			{
				POINT_COLOR=BLACK;
				LCD_DrawPoint(b,a);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------横向跳变点显示函数

void ChangePoint_Show_320()//320方向跳变点显示
{
		u16 a=0,b=0;
	for(a=0;a<320;a++)//显示对应的横向跳变点								
	{ 
		POINT_COLOR=RED;
		if(TableChangePoint_320[a]==0)
		{
			LCD_DrawPoint(a,0);//跳变点显示，红色标记
		}
		else
		{
			LCD_DrawPoint(a,TableChangePoint_320[a]);//跳变点显示，红色标记
		}
		
	}
}

//-------------------------------------------------------------------------------------------------------------字符分割函数

u8 ZhiFuFenGe()//字符分割,返回分割的字符个数，用于判断合法性
{
	u16 a=0,b=0;
	u8 i=0;//统计分割的字符个数，不为9说明分割有误
 POINT_COLOR=BLUE;
	for(b=Max_blue;b>Min_blue;b--)
	{
					if(TableChangePoint_320[b]==0)//间隙分割
					{
									for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//划线--调试用
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
	LCD_ShowNum(30,200,i,2,16);//显示分割的字符个数+1，8是正常值
	return i;
}

//-------------------------------------------------------------------------------------------------------------归一化处理函数 统一25*50
//右边、左边
 void GuiYi(u16 k,u16 kk)//归一化 24*50
{
	u16 a=0,b=0,e=0;
	u16 num=0;//保存读取像素
	u8 Mo=0,Yu=0;//取整和取模
	u8 num1=0,num2=0,num3=0;
	u8 Mo_1=0;//
	u8 Min_240=0,Max_240=0;//框紧字符后的上下限
	
//	if((k-kk)<25)
	//只有字符的宽度小于25像素的才可以执行字符归一化
	if((k-kk)<25)
	{
		//框紧字符
		Min_240=Min_ChangePoint_240+1;
		Max_240=Max_ChangePoint_240-1;
		while(Min_240++)//框紧后，得到: Min_240
		{
			for(b=kk+1;b<k;b++)//扫描这一行的时候，如果遇到1像素，那么立刻退出，0像素是黑色的，表示非车牌区域                               
			{
				num=LCD_ReadPoint(b,Min_240);
				if(num) break;
			}
			if(num) break;
		}
		while(Max_240--)//框紧后，得到: Max_240
		{
			for(b=kk+1;b<k;b++)//kk1→k1                                
			{
				num=LCD_ReadPoint(b,Max_240);
				if(num) break;
			}
			if(num) break;
		}
		Min_240-=1;
		Max_240+=2;
		//在选出的字符的左下角和右上角分别画一个点！！
		LCD_DrawPoint(kk,Min_240);
		LCD_DrawPoint(k,Max_240);

		//显示复制的图片
		num3=0;
		for(a=Min_240+1;a<Max_240;a++)
		{
			num2=0;
			for(b=kk+1;b<k;b++)//kk1→k1
			{
				//第一个黑框！！！
				num=LCD_ReadPoint(b,a);
				POINT_COLOR=num;
				LCD_DrawPoint(271-(k-kk-1)+num2,191+num3);//画一行的点，每次画完一个点，行不变，列+1，也就是横向的像素自增。
				
				num2++;
			}
			num3++;
		}
		delay_ms(1000);
		//		—————————————————————————————————横轴拉长————————————————————————————————————————————————
		//行的计数清零
		num3=0;
		Mo=(24-(k-kk-1))/(k-kk-1);//地板除，只取整数部分
		Yu=(24-(k-kk-1))%(k-kk-1);//模运算就是取模，就是取余
		if(Yu != 0) {
			Mo_1=24/Yu;//平均Mo_1个像素，插有一个像素， Yu：代表需要在原图的宽度上插Yu个像素点才能达到目标的24个像素  
		}
// 		LCD_ShowNum(30,20,Mo,3,16);//显示模		<调试用>
// 		LCD_ShowNum(70,20,Yu,3,16);//显示余
// 		LCD_ShowNum(100,20,(k1-kk1),3,16);//显示差值

		for(a=Min_240+1;a<Max_240;a++)//宽放大为25像素
		{
			num2=0;//每次从新的一行的头部开始
			Yu=(24-(k-kk-1))%(k-kk-1);//模运算就是取模，就是取余
			for(b=kk+1;b<k;b++)//kk1→k1 
			{
				//复制图片，用与显示————————————————————————————————————————————————————————————————————————————————————————————
				//这段函数的作用是复制图片，用于显示，从指定位置开始显示。每次内循环，显示一个像素点，每一次外循环，显示一行的像素点！！！！
				num=LCD_ReadPoint(b,a);
				//第二个黑框!!!
				POINT_COLOR=num;//不能少
				LCD_DrawPoint(271+num2,191+num3);
				num2++;
			//图片横向拉伸————————————————————————————————————————————————————————————————————————————————————————————
				//1）如果图片大过25像素，进行处理，如果Mo=1,那么每次内循环就会多插值一个点，这种情况属于图片的宽度只有26的一半，即width=13；
				Mo=(24-(k-kk-1))/(k-kk-1);//地板除，只取整数部分
				while(Mo)
				{
					POINT_COLOR=num;
					LCD_DrawPoint(271+num2,191+num3);
					Mo--;
					num2++;
				}
				//一般不会用到，除非采集到的车牌很糟，即采集到的车牌的宽度小于规定的一般才会发生
				//2）当Mo=1的时候，Yu必为0；,Yu不为0，那么Mo=0；即图片的宽度不为26的一半的时候
				if(Yu!=0)
				{	
					if(((num2+1)%Mo_1==0) && (num2!=1))//改插入的地方7+1
					{
						POINT_COLOR=num;
						LCD_DrawPoint(271+num2,191+num3);
						Yu--;
						num2++;
					}
				}
			}//当前行完成插值
			num3++;//调到下一行
		}
		delay_ms(1000);
		POINT_COLOR=0x07E0;
		LCD_DrawPoint(271,191);//标记点，4个顶角
		LCD_DrawPoint(271,240);
		LCD_DrawPoint(295,191);
		LCD_DrawPoint(295,240);
//————————————————————————————————————————————————————————纵轴拉长———————————————————————————————————————————————————
		if((Max_240-Min_240)<50)
		{
			Mo=(50-(Max_240-Min_240+1))/(Max_240-Min_240+1);//取模
			Yu=(50-(Max_240-Min_240+1))%(Max_240-Min_240+1);//取余
			Mo_1=50/Yu;//平均Mo_1个像素，插有一个像素
			
// 			LCD_ShowNum(30,170,Mo,3);//					<调试用>
// 			LCD_ShowNum(70,170,Yu,3);//
// 			LCD_ShowNum(100,170,Max_ChangePoint_240-Min_ChangePoint_240,3);//
			num2=0;
			for(a=0;a<(Max_240-Min_240);a++)//复制图像,考虑范围是否需要进行修正？
			{
				for(b=271;b<=295;b++)//271开始复制，295才结束
				{
					//第三个黑框！！！
					num=LCD_ReadPoint(b,a+191);
					POINT_COLOR=num;
					LCD_DrawPoint(b+25,191+num2);//复制像素值,第二个黑框
				}
				num2++;
				while(Mo)
				{
					for(b=271;b<=295;b++)//271开始复制，295才结束
					{
						num=LCD_ReadPoint(b,a+191);
						POINT_COLOR=num;
						LCD_DrawPoint(b+25,191+num2+a);//复制像素值
					}
					Mo--;
					num2++;
				}
				if(Yu!=0)
				{
					if((((num2+1) % Mo_1)==0)&& (num2!=1))
					{
						for(b=271;b<=295;b++)//271开始复制，295才结束
						{
							num=LCD_ReadPoint(b,a+191);
							POINT_COLOR=num;
							LCD_DrawPoint(b+25,191+num2);//复制像素值
						}
						Yu--;
						num2++;
					}
				}					
			}
		}
		POINT_COLOR=RED;
		LCD_DrawPoint(320,191);//标记点，1个顶角
	}
}

void String_Picture()//数组到图片
{
	u16 a=0,b=0,e=0,num1=0;
	
	for(a=0;a<50;a++)//50排
	{
		for(b=0;b<24;b++)//24行
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
			LCD_DrawPoint(b+296,a+191);//画点
		}				
	}	
}
//------------------------------------------------------------------------------------------------------图片->数组table_picture  待修改  
void Picture_String()//图片->数组table_picture      有个BUG，先放着吧
{
	u16 a=0,b=0,num1=0,move=0;
	for(a=0;a<150;a++)//归零
	{
		table_picture[a]=0x00;	//u8 table_picture[150];
	}	
	for(a=0;a<50;a++)//50列
	{
		for(b=0;b<24;b++)//24行
		{
		
			num1=LCD_ReadPoint(b+296,a+191);
			if(num1==0xffff)//颜色为白色的像素点
			{
				table_picture[b/8+a*3]|=(1<<(7-b%8));//每隔7/8个点取一个像素点！！
				 //(1<<(7-b%8))   将"1" 变换在单字节中的相应位置上。  
			}
		}
	}
}


void ChangePoint_Show_240()//240方向跳变点显示
{
	u32 a=0,b=0;
	    POINT_COLOR=RED;
				for(a=0;a<240;a++)//建立参考线10、20、30 纵向线
				{
					LCD_DrawPoint(10,a);//10
					LCD_DrawPoint(20,a);//20
					LCD_DrawPoint(30,a);//30
				}
	
				for(a=0;a<240;a++)//显示对应的横向跳变点								
				{
								LCD_DrawPoint(TableChangePoint_240[a],a);//跳变点显示，红色标记
							if(TableChangePoint_240[a]>=15)					//跳变点个数（阈值）设定       阈值调节3-（1）
							{
								   POINT_COLOR=GREEN;
										for(b=35;b<40;b++)						//显示达到阈值标准的点
										{
											LCD_DrawPoint(b,a);//Green			
										}
							}
				}
}
void ChangePoint_Analysis_240()//240跳变点分析
{
	u16 a=0,b=0; 
	Min_ChangePoint_240=240;
	Max_ChangePoint_240=0;
	
	for(a=0;a<240;a++)//240扫描	，获取上下限值	：Min_ChangePoint_240，Max_ChangePoint_240				
	{
		while(TableChangePoint_240[a]<=15)									//阈值调节3-（2）
		{
			a++;
		}
		Min_ChangePoint_240=a;
		
		while(TableChangePoint_240[a]>15)									//阈值调节3-（3）
		{
			a++;
		}
		Max_ChangePoint_240=a;
		if(Max_ChangePoint_240-Min_ChangePoint_240>=15) //说明扫描区域 没有车牌信息
			a=240;//连续性阈值   	//阈值调节2-（1）
	}
	Min_ChangePoint_240=Min_ChangePoint_240-5;//向上微调3像素
	Max_ChangePoint_240=Max_ChangePoint_240+5;//向下微调2像素
	POINT_COLOR=BLUE;
	for(a=30;a<280;a++)//显示上界限				
	{
		LCD_DrawPoint(a,Max_ChangePoint_240);//蓝色
	}
	for(a=30;a<280;a++)//显示下界限						
	{
		LCD_DrawPoint(a,Min_ChangePoint_240); //蓝色
	}
	POINT_COLOR=RED;
	for(a=30;a<280;a++)//显示50,参考50像素位置处，车牌位置不要超过这根线，免得不能字符的归一化处理						
	{
		LCD_DrawPoint(a,Min_ChangePoint_240+50); //红色
	}
	
	flag_MaxMinCompare=1;
	
	if(Min_ChangePoint_240>Max_ChangePoint_240)//判断合法性1：最小值>最大值
	{
		flag_MaxMinCompare=0;
	}
	if(Min_ChangePoint_240==240||Max_ChangePoint_240==0)//判断合法性2：值没有重新赋值
	{
		flag_MaxMinCompare=0;
	}
	if(Max_ChangePoint_240-Min_ChangePoint_240<15)		//判断合法性3：			//阈值调节2-（2）
	{
		flag_MaxMinCompare=0;
	}
}
void RGB_HSV(u16 num)//RGB565转HSV
{
	float max,min;
	u8 r=0,g=0,b=0;
	r=(num>>11)*255/31;g=((num>>5)&0x3f)*255/63;b=(num&0x1f)*255/31;
	
	max=r;min=r;
	if(g>=max)max=g;
	if(b>=max)max=b;
	if(g<=min)min=g;
	if(b<=min)min=b;
	
	V=100*max/255;//转换为百分比
	S=100*(max-min)/max;//扩大100倍显示
	if(max==r) H=(g-b)/(max-min)*60;
	if(max==g) H=120+(b-r)/(max-min)*60;
	if(max==b) H=240+(r-g)/(max-min)*60;
	if(H<0) H=H+360;
}
void ChangePoint_Analysis_Blue()//320蓝色区域分析,采用读取像素，得结果Min_blue,Max_blue
{
	u16 a=0,b=0,num_color=0;
	u16 min_320=0,max_320=0;//各行的最小、最大值
	
	Min_blue=0;Max_blue=320;
	min_320=320;max_320=0;
	
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)								
	{
		for(b=30;b<290;b++)//不用到320    for(b=30;b<320;b++)
		{
			num_color=LCD_ReadPoint(b,a);//读取像素，代码优化速度有待提升 ？扫描方法也可优化，以提升速度
			RGB_HSV(num_color);//RGB565转HSV
			//			if( 245>H && H>190 && 1>S && S>0.35 && 1>V && V>0.3)//HSV 阈值
			if( 245>H && H>190 && 60>S && S>15 && 100>V && V>45)//HSV 阈值
		
			{
				if(b<min_320)//获得横轴的Min和Max值，即蓝色车牌的左右边界
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
	Min_blue=min_320;//获取各行的最大值//修正一点
	Max_blue=max_320-5;//获取各行的最小值//修正一点
	POINT_COLOR=RED;
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//显示左界限				
	{
		LCD_DrawPoint(Min_blue,a);// 
	}
	POINT_COLOR=BLUE;
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//显示右界限					
	{
		LCD_DrawPoint(Max_blue,a);
	}
//  delay_ms(6000);
}
 u8 MoShiShiBie_All(u8 begin,u8 end)//字符匹配，模式识别,选择性匹配begin-end            !!
{
//扣出字来，和字符模板去比对，怎么比对的？
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
						//table_picture是36*150=5400个字节，每个字符150个字节，共36个字节；
									st1=table_picture[b];//取出当前1个字符数组的第b号字节
									st2=Table[150*a+b];//取出模板库中对应的1个字符数组的第b号字节
									for(e=0;e<8;e++)
									{
										s1=st1&(1<<e);//分别取出8位进行比较
										s2=st2&(1<<e);
										if(s1==s2) num1++;
										if(s1!=s2) num1--;
										//num的范围是：0-1200,0不匹配，1200完全匹配
									}
					}
				if(num1>num_save)
				{
					num_save=num1;
					a_save=a;
				}
				LCD_ShowNum(50,220,a_save,2,16);				//显示匹配的字符是"a"			<调试用>
				LCD_ShowNum(70,220,num1,4,16);			//显示匹配的正确像素数
				LCD_ShowNum(120,220,num_save,4,16);//匹配的最大值显示					
	}
	return a_save;
}
void WordShow(u8 num,u16 x,u16 y)//显示汉字16*16
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
			LCD_DrawPoint(b+x,a+y);//画点
		}				
	}	
}

void ZhiFuShiBie()//字符识别
{
	u16 a,b,u,i;
	u8 Result;//识别结果
	for(b=Max_blue-1;b>Min_blue;b--)//由右至左识别，获取各个字符的 K KK值, 即字符边界
	{
		while(TableChangePoint_320[b]==0)b--;//取第1个字符
		k1=b+1;//+1
		while(TableChangePoint_320[b]>0) b-- ;
		kk1=b;
		if((k1-kk1)<4)//省略低于三个像素的位置
		{
			while(TableChangePoint_320[b]==0) b--;//
			k1=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk1=b;
		}
	 
		while(TableChangePoint_320[b]==0) b--;//取第2个字符
		k2=b+1;
		while(TableChangePoint_320[b]>0) b--;
		kk2=b;
		if((k2-kk2)<4)//省略低于3个像素的位置
		{
			while(TableChangePoint_320[b]==0) b--;//
			k2=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk2=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//取第3个字符
		k3=b+1;//+1
		while(TableChangePoint_320[b]>0) b--;
		kk3=b;
		if((k3-kk3)<4)//省略低于3个像素的位置
		{
			while(TableChangePoint_320[b]==0) b--;//
			k3=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk3=b;
		}
	 
		while(TableChangePoint_320[b]==0) b--;//取第4个字符
		k4=b+1;
		while(TableChangePoint_320[b]>0) b--;
		kk4=b;
		if((k4-kk4)<4)//省略低于3个像素的位置
		{
			while(TableChangePoint_320[b]==0) b--;//
			k4=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk4=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//取第5个字符
		k5=b+1;//+1
		while(TableChangePoint_320[b]>0) b--;
		kk5=b;
		if((k5-kk5)<4)//省略低于3个像素的位置
		{
			while(TableChangePoint_320[b]==0) b--;//
			k5=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk5=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//取第6个字符
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
		 
		while(TableChangePoint_320[b]==0) b--;//取第7个字符
		k7=b+1;//+1
		while(TableChangePoint_320[b]>0) b--;
		kk7=b;
		if((k7-kk7)<4)//省略低于3个像素的位置
		{
			while(TableChangePoint_320[b]==0) b--;//
			k7=b+1;//+1
			while(TableChangePoint_320[b]>0) b--;
			kk7=b;
		}
		 
		while(TableChangePoint_320[b]==0) b--;//取第八个字符
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
		b=Min_blue;//以防万一，还满足for循环条件
	}
	for(a=Min_ChangePoint_240;a<Max_ChangePoint_240;a++)//划线
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
//归一化处理：大小为25*50
//第1个字符：
	GuiYi(k1,kk1);//归一化 24*50
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(0,36);//字符匹配，模式识别,返回a,0<= a <36
	if(Result<10)
	{
		LCD_ShowNum(240,220,table_char[Result],1,16);
	}
	else
	{
		POINT_COLOR=GREEN;
		LCD_ShowChar(240,220,table_char[Result],16,0);
	}
	table_cardMeasure[6]=Result;//保存识别的车牌结果
	
//第2个字符：
	GuiYi(k2,kk2);//归一化 25*50
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(0,36);//字符匹配，模式识别
//	printf("RESULT 2 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(230,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(230,220,table_char[Result],16,0);
	}
	table_cardMeasure[5]=Result;//保存识别的车牌结果
	
	GuiYi(k3,kk3);//归一化 25*50
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(0,36);//字符匹配，模式识别
//	printf("RESULT 3 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(220,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(220,220,table_char[Result],16,0);
	}
	table_cardMeasure[4]=Result;//保存识别的车牌结果
	
	GuiYi(k4,kk4);//归一化 25*50
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(0,36);//字符匹配，模式识别
//	printf("RESULT 4 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(210,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(210,220,table_char[Result],16,0);
	}
	table_cardMeasure[3]=Result;//保存识别的车牌结果
	
	GuiYi(k5,kk5);//归一化 25*50
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(0,36);//字符匹配，模式识别
//	printf("RESULT 5 %d",Result);
	if(Result<10)
	{
		LCD_ShowNum(200,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(200,220,table_char[Result],16,0);
	}
	table_cardMeasure[2]=Result;//保存识别的车牌结果
	LCD_ShowChar(190,220,'.',16,0);

	GuiYi(k7,kk7);//归一化 25*50
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(10,36);//字符匹配，模式识别，只匹配字母
	if(Result<10)
	{
		LCD_ShowNum(180,220,table_char[Result],1,16);
	}
	else
	{
		LCD_ShowChar(180,220,table_char[Result],16,0);
	}
	table_cardMeasure[1]=Result;//保存识别的车牌结果
	
	GuiYi(k8,kk8);//归一化 25*50					最后一个汉字，不做识别
	Picture_String();//图片->数组
	Result=MoShiShiBie_All(36,42);//字符匹配，匹配汉字
	WordShow(Result-35,160,220);//显示汉字
	table_cardMeasure[0]=Result-35;//保存识别的车牌结果
		//串口发送车牌信息     //
	if(Result==36)
	{
		printf("俞");
	}
	else if(Result==37)
	{
		printf("闽");
	}
	else if(Result==38)
	{
		printf("沪");
	}
	else if(Result==39)
	{
		printf("浙");
	}
	else if(Result==40)
	{
	 printf("苏");
	}
	else if(Result==41)
	{
		printf("粤");
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
//更新LCD显示(OV7725)
void Data_LCD_ColorChange(void)
{
	u32 i=0,j=0;
 	u16 color=0;	 
  u32 a=0,b=0;
	
	for(a=0;a<240;a++)//各行跳变点计数，数组清零
	{
		TableChangePoint_240[a]=0;
	}
	Min_blue=320;//初始化记录蓝色车牌区域的值
	Max_blue=0;
	
 
	LCD_Scan_Dir(ways0_8);//从上到下,从左到右
	LCD_Set_Window(0,0,OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT);//将显示区域设置到屏幕中央
	LCD_WriteRAM_Prepare();     //开始写入GRAM	
	 
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WRST=0;
	OV7725_WRST=1;
	OV7725_WREN=1;
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1);
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WREN=0;
	OV7725_RRST=0;				//开始复位读指针 
	OV7725_RCK_L;
	OV7725_RCK_H;
	OV7725_RCK_L;
	OV7725_RRST=1;				//复位读指针结束 
	OV7725_RCK_H; 
	
		for(a=0;a<OV7725_WINDOW_HEIGHT;a++)
		{
					for(b=0;b<OV7725_WINDOW_WIDTH;b++)
					{
								OV7725_RCK_L;
								AA=GPIOC->IDR&0XFF;	//读数据
								OV7725_RCK_H; 
										
								OV7725_RCK_L;
								BB=GPIOC->IDR&0XFF;	//读数据
								OV7725_RCK_H; 
				        color=(AA<<8)|BB;
				 
								R=color>>11;
								G=(color>>5)&0x3f;
								B=color&0x1f;
//						
								if((R>R_a) && (G>=G_a) && (B>=B_a))//二值化,高阈值：25.55.25，较合适阈值（21,47,21）
								{
									 color=0xffff;	//白色
								}
								else
								{
									 color=0x0000;	//黑色
								}
//						
								if(color!=color_save)//跳变点
								{
									TableChangePoint_240[a]++;		//该行跳变点计数+1
								}
						   color_save=color;//保存像素值，供下一次判断和比较
 
					    color=(AA<<8)|BB;//还原色彩
								
				     LCD->LCD_RAM=color;  
								
						}
		}
//		LCD_Scan_Dir(ways0_8);	//恢复默认扫描方向 
		ChangePoint_Show_240();
		ChangePoint_Analysis_240();
 if(flag_MaxMinCompare==1)//跳变点筛选成功
	{
		ChangePoint_Analysis_Blue();//320蓝色区域分析,采用读取像素，得结果Min_blue,Max_blue
		if(Min_blue>Max_blue) 
			flag_MaxMinCompare=0;//进行合理性判断1
		if((Min_blue>290)||(Max_blue>290)) 
			flag_MaxMinCompare=0;//进行合理性判断2
	}
		if(flag_MaxMinCompare==1)//跳变点筛选成功
	{
		ChangePoint_Analysis_320();//蓝色区域中，320跳变点分析,获得：TableChangePoint_320[b]结果
		ChangePoint_Show_320();//320方向跳变点显示
	
		i=ZhiFuFenGe();
		printf("分割出 %d 个字符 \r\n",i);
		if(i>=8)//字符分割,返回分割的字符个数，用于判断合法性
		{
			ZhiFuShiBie();//字符识别
		}
    else
		{
			LCD_Clear(0x0000);//黑屏，显示Measure Faill
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
//更新LCD显示(OV7725)
void Data_LCD_ColorChange_Test(void)
{
	u32 i=0,j=0;
 	u16 color=0;	 
  u32 a=0,b=0;
	
	for(a=0;a<240;a++)//各行跳变点计数，数组清零
	{
		TableChangePoint_240[a]=0;
	}
	Min_blue=320;//初始化记录蓝色车牌区域的值
	Max_blue=0;
	
 
		LCD_Scan_Dir(ways0_8);//从上到下,从左到右
		LCD_Set_Window(0,0,OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT);//将显示区域设置到屏幕中央
//		if(lcddev.id==0X1963)
//		LCD_Set_Window((lcddev.width-OV7725_WINDOW_WIDTH)/2,(lcddev.height-OV7725_WINDOW_HEIGHT)/2,OV7725_WINDOW_HEIGHT,OV7725_WINDOW_WIDTH);//将显示区域设置到屏幕中央
		LCD_WriteRAM_Prepare();     //开始写入GRAM	
	 
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WRST=0;
	OV7725_WRST=1;
	OV7725_WREN=1;
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1);
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==0);
	OV7725_WREN=0;
		
		OV7725_RRST=0;				//开始复位读指针 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//复位读指针结束 
		OV7725_RCK_H; 
		
		for(a=0;a<OV7725_WINDOW_HEIGHT;a++)
		{
					for(b=0;b<OV7725_WINDOW_WIDTH;b++)
					{
								OV7725_RCK_L;
								AA=GPIOC->IDR&0XFF;	//读数据
								OV7725_RCK_H; 
										
								OV7725_RCK_L;
								BB=GPIOC->IDR&0XFF;	//读数据
								OV7725_RCK_H; 
				
				        color=(AA<<8)|BB;
				 
								R=color>>11;
								G=(color>>5)&0x3f;
								B=color&0x1f;
//						
								if((R>R_a) && (G>=G_a) && (B>=B_a))//二值化,高阈值：25.55.25，较合适阈值（21,47,21）
								{
									 color=0xffff;
								}
								else
								{
									 color=0x0000;
								}
//						
								if(color!=color_save)//跳变点
								{
									TableChangePoint_240[a]++;		//该行跳变点计数+1
								}
						   color_save=color;//保存像素值，供下一次判断和比较
 
					    color=(AA<<8)|BB;//还原色彩
								
				     LCD->LCD_RAM=color;  
								
						}
		}
		LCD_Scan_Dir(ways0_8);	//恢复默认扫描方向 
		ChangePoint_Show_240();
		ChangePoint_Analysis_240();
  
	 
	}
	
  
 
void PA8_in()//PA8引脚为输入
{
	GPIO_InitTypeDef gpio_init_struct;//结构体
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;//输入上拉
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
	u8 msgbuf[15];//消息缓存区
	u8 tm=0;
	u8 lightmode=0,effect=0;
	s8 saturation=0,brightness=0,contrast=0;
	 
	delay_init();	    	 	//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 		//串口初始化为 115200
 	usmart_dev.init(72);		//初始化USMART		
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD  
	//TPAD_Init(6);				//触摸按键初始化 
 	POINT_COLOR=RED;			//设置字体为红色 
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
 //初始化OV7725_OV7670
		if(OV7725_Init()==0)
		{
			sensor=OV7725;
			LCD_ShowString(30,230,200,16,16,"OV7725 Init OK       ");
 
			OV7725_Window_Set(OV7725_WINDOW_WIDTH,OV7725_WINDOW_HEIGHT,0);//QVGA模式输出
			OV7725_CS=0;
		}
 
	//TIM6_Int_Init(10000,7199);	//10Khz计数频率,1秒钟中断									  
	//EXTI8_Init();				//使能定时器捕获			
  PA8_in();		
	LCD_Clear(BLACK);
		
	R_a=24;
	G_a=53;
	B_a=24;
			while(1)
			{
				if(num<=1)
				{
				Data_LCD_ColorChange();//车牌测定
				}
				if(num>1)
				{
					Data_LCD_ColorChange_Test();// 
					LCD_ShowNum(30,220,21-num,2,16);//
					delay_ms(20);//延时，倒计时
				}
				if(num==20)
				{
					num=0;
				}
				num++;
			}
}













