# 关于本项目
-------
eLesson项目，是狗哥专门针对在自媒体上推出的视频例程，所配合的源代码。

目前，eLab尚未独立，包含在eLesson项目里。就在未来的两到三个月内，eLab项目将正式独立，而且会被提升比较重要的位置上去。

## 一、eLab项目是什么？

简而言之，就是一个开发平台。

**eLab**项目，是一个集成了多种开源项目和多个技术特性的嵌入式开发平台。狗哥，将在这个项目，汇集我多年来在产品开发中的开发理念和经验，以现有的开源软件为主，以自己开发的组件为辅，打造一个统一而完善的开发平台。让嵌入式工程师的开发起飞！

**eLab**项目，与**EventOS**和**BasicOS**项目，到底是什么关系，一直让大家困惑不解。**EventOS**和**BasicOS**项目，是狗哥在进行了长达十五年的纯粹技术生涯后，进行的初步转型探索，总体而言，收获了很多。**eLab**项目，是一个集成式项目，是开发平台，创新并不是重点；**EventOS**项目和**BasicOS**项目，都是创新性的，都是带来新东西的。

**EventOS**和**BasicOS**项目，狗哥仍将继续。其中，**BasicOS**项目是一个小而美的项目，一个共享栈空间的协作式RTOS项目，很有意思，我会在2023年4月份完成共享栈这一核心特性的开发。**EventOS**将调整方向，作为**eLab**项目的中间件开发，当独立运行时，专攻小RAM芯片。

## 二、eLab项目的技术特性
**eLab**项目，侧重两大技术方向，一是**产品级的嵌入式软件开发**，二是**跨平台的嵌入式软件开发**。这两个技术方向，最终目的，都是为了让工程师的开发效率，得到颠覆性的提升。尤其很多非软件或者计算机专业毕业的嵌入式工程师们，不必在错误或者跑偏的技术方向上苦苦探索，从而减少技术进阶的年限，早日进入更高的职业阶段。

无论是**产品级的嵌入式软件开发**，还是**跨平台的嵌入式软件开发**，都不是一个单一的技术，这两个方向上，都需要一系列技术族进行支撑，**eLab**项目，便是这些技术族的逐一实现。还需要说明的是，这两个技术方向，能做的事情非常之多，狗哥会慢慢开发下去。同时，狗哥会通过B站视频，对这些技术，进行讲解，同时提供系列开发板，对**eLab**项目进行体验和学习。

#### eLab项目里已经实现的
+ eLab common module
    + Basic definitions
    + Log module
    + Assert function
    + Export function
+ eLab OSAL
    + Win32
    + Linux
    + FreeRTOS
+ Third-party modules for eLab
    + FreeRTOS
    + ini library
    + list library
    + lwrb
    + ucModbus
    + pikaPython
    + QPC
    + Rolla database based on file
    + Shell
    + Unity
+ Basic library of eLab
+ Device framework of eLab
    + Core of device framework
    + **Normal devices**: Serial port, CAN port, ADC, I2C, SPI, Watchdog, RTC, PIN and so on.
    + **User defined devices**: Motor, RS485, Helm, LED, buttons and so on.
    + **Driver**: Linux, Simulators and WIN32.
+ Examples
    + CMSIS RTOS on WIN32
    + Export on PC
    + QPC on CMSIS RTOS
+ Test
+ Unit tests

上述已经实现的模块，已经可以初步支撑高效的嵌入式软件开发，但这个小小生态，还远算不上完整，仍然需要完善，还有很长的路要走。

## 三、关于重启B站视频

前段时间，由于个人原因，狗哥暂停了B站视频的定期发布，改为不定期发布（实际上啥都没发布）。但开源代码，作为狗哥的精神食粮，狗哥一直在做。不仅在做，最近做的成绩不错。**eLab**平台小有规模，已经在实际项目中运行，是一个机器人底盘项目，后续狗哥会以合适的方式将此项目开源。

最近项目告一段落，狗哥要将重点转移到B站视频与金毛板例程上来。人生偶尔会有小小挫折，但人总是好了伤疤忘了疼，这是人性，狗哥也未能免俗。5月，见了一个福建网友一面，受益颇深。**不折腾的人生毫无意义。该到来的，虽然迟来，虽然慢来，总是会来，而且往往以更好的方式到来。有时，慢点更好，快了，反而会慢。**

## 四、狗哥的联系方式

欢迎大家加群，加微信，聊天吹牛逼，交流技术。我曾经待过一个公司，同事们上班，居然不聊天吹牛逼，甚是无趣，我干了三个月就跑了。人生如果没有人一起吹牛，将是巨大遗憾；狗哥吹牛能力不强，但特别爱听，天天在公司竖着耳朵听。欢迎大家进得群来，吹牛逼，这非常重要！

### Q群二维码

<img src="docs/images/pic_qq_group.jpg" width="30%"  />

### 微信二维码
<img src="docs/images/wechat_gouge.jpg" width="30%"  />

### B站二维码
<img src="docs/images/pic_bilibili.jpg" width="30%"  />
