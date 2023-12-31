# STM32-chepaishibie
## 车牌识别项目

本系统硬件包括STM32F103ZET6单片机、2.8寸TFT-LCD液晶屏、摄像头图像采集ov7670（带FIFO）。

1、stm32单片机通过摄像头采集图像，并实时驱动TFT液晶屏显示相应图像。

2、stm32单片机通过模式识别、匹配货的车牌的识别结果，并在屏幕上进行显示。

3、识别主要过程包括图像采集、二值化分析、识别车牌区域、字符分割、字符匹配五个过程。

//未完成

4、车牌锁定后会有蜂鸣器提醒，在分析获取到车牌后对车停留时间进行计时，并进行计费。

5、在图像采集界面，通过按键可以进入后台计费界面。在车牌识别后进入计费界面，可以通过按键退出计费界面，回到图像采集界面。

6，将识别的车牌号传递给手机app

//未完成

**注意：单片机处理能力及速率有限，目前识别汉字：渝、辽、沪、浙、苏、粤，车牌图片一定要清晰，无反光，容易识别。**

车牌识别操作技巧与按键功能说明：

1、重要一点，通过摄像头前面螺钮可以调焦，拧到直到液晶显示图像最清晰（一般我们调试好的）。

2、尽量让车牌号处于液晶中央位置，让车牌号内容处于两蓝线之间，且两蓝线处于红线上方。

3、位置合适后，进入倒计时，到时蜂鸣器会“嘀”的一声响，表示开始分析识别。识别需要一定时间。

4、在没有识别出车牌前，按下K1可查看已经识别出的车牌信息，并可看到计费信息（模拟停车场），识别出车牌后，显示车牌信息后， 需要按下按键K2，方可返回主界面。

![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb1.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb2.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb3.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb4.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb5.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb6.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb7.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb8.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb9.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb10.png)

**将图片的一维数组， 与模板数组进行比对，比对时，按像素进行对比。
比对成功一次，计数加1。
最左侧代表当前匹配的字符在模板库中是第几个字符
中间表示当前字符匹配的正确像素数；
最右侧表示匹配的像素数目的最大值!!**

![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb11.png)

**难点是黑夜，雨天雾天，车牌模糊这些情况！**

![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb12.png)
![image](https://github.com/ljc108/STM32-chepaishibie/blob/main/LicensePlateRecognition-master/cpsb13.png)
