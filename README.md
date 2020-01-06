# stm32f407_gcore_rtt_prj 开发板 tn19002分支 说明

## 简介

本分支基于 stm32f407_gcore_rtt_prj master分支，开发单逆测试台上层应用

主要功能如下：

- usb-cdc或uart3与上位机通讯交互(使用uart3，2G功能将不可使用)
- uart1与dpsp1000程控电源控制交互
- uart6与逆变器交互，采集状态码
- 12位adc采样，采集程控电源实际输出参数
- ADE7880三相电能芯片驱动与应用层实现

### 将ADE7880快速设置为电表
- 选择相电流、电压和零线电流通道内的PGA增益：Gain寄存器中的位[2:0] (PGA1)、位[5:3] (PGA2)和位[8:6] (PGA3)。 
- 电流采集使用罗氏线圈，使能相电流或零线电流通道内的数字积分器：CONFIG寄存器中的位0 (INTEN)和CONFIG3寄存器中的位3 (ININTEN)。
- 如果fn= 60 Hz，将COMPMODE寄存器中的位14 (SELFREQ)置1 (该项忽略)
- 基于公式49初始化CF1DEN、CF2DEN和CF3DEN寄存器。
- 分别基于公式26、公式37、公式44、公式22和公式42初始化WTHR、VARTHR、VATHR、VLEVEL和VNOM寄存器。
- 使能数据存储器RAM保护，向位于地址0xE7FE的内部8位寄存器写入0xAD，然后向位于地址0xE7E3的内部8位寄存器写入0x80。
- 设Run = 1，启动DSP。


## 上位机介绍

上位机采用C#开发，界面如下：


## 注意事项

暂无

## 联系人信息

维护人:Leon Kwok

