# Lab8_Bit_Banging
In today's lab, you will use a common technique called bit banging to communicate with the display that you have been using throughout the lecture.

# What is Bit Banging
>**Bit banging** is a term of art that describes a method of digital data transmission as using general-purpose input/output (GPIO) instead of computer hardware that is intended specifically for data communication. Controlling software is responsible for satisfying protocol requirements including timing which can be challenging due to limited host system resources and competing demands on the software.

(from: [Wikipedia](https://en.wikipedia.org/wiki/Bit_banging))

So, basically Bit Banging describes a way to realize a serial (or parallel) interface via software instead of dedicated hardware. Think of the laboratory for serial communication. There we used a dedicated module of our microcontroller to realize the communication via UART. If we would have used bit banging instead you would have needed to take care of Rx' and Tx' logical levels via software. Which is especially complicated as we used USART asynchronous, so the timing is essential and communication must not be interrupted.

Today we will use this technique to realize a more forgiving communication. Therefore, we will realize a SPI communication using Bit Banging to send data to our display. The following chapter recapitulates the communication using a Serial Peripheral Interface. Afterward, your exercise is described.

# Repetition of Serial Peripheral Interface (SPI)
Remember the structure of SPI. For this interface we need 3+n wires, where n is the number of slaves.
Why do we need 3+n wires? Try to answer this question. Below image might help with that.
![](images/SPI_single_slave.svg)
(source: [Wikipedia](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#/media/File:SPI_single_slave.svg))
Before we proceed to check which lines are used on our laboratory board for this kind of communication, we need to first make clear to understand the purpose of each line and to know potential different names for those.
Therefore, complete below table (use the script or google if necessary, avoid using ChatGPT!).

| Signal / Line | Name | Purpose | Alternative name |
| :--- | :--- | :--- | :--- |
| $\text{SCLK}$ | **S**erial **Cl**o**ck** | Clock line for communication | CLK |
| $\text{MOSI}$ | **M**aster **O**ut **S**lave **I**n | Data line from master to slave (or chip) | SDI (Slave Data In) |
| $\text{MISO}$ | **M**aster **I**n **S**lave **O**ut | Data line from slave (or chip) to master | SDO (Slave Data Out) |
| $\overline{\text{SS}}$ | (not) **S**lave **S**elect | activate/select chip for communication | (not) Chip Select $\overline{\text{CS}}$, (not) Serial Chip Enable $\overline{\text{SCE}}$ |

Know, that we know the purpose of our lines, we need to check the corresponding connections on our laboratory board.
Open the board's [schematic](https://raw.githubusercontent.com/MicrocontrollerApplications/Lab2_InputOutput/refs/heads/main/images/uCquick_Board_2018_01.svg) and check the "Display" section of it. There you will find all pins connected to the display.

>[!IMPORTANT]
>The logical not, notated using a horizontal bar above a signals name (e.g. $\overline{\text{SS}}$), is alternatively represented using a leading slash - e.g. /SS.
>So, if you see a signals name on the display with such a leading slash, that means that the signal is active low!

Note the displays corresponding SPI signals with their connected pins in below table.
>[!HINT]
>The __D/C__ input of the display is control input, to inform the display about wether __D__ata or __C__onfiguration is send.
> __/RES__ is quite obvious. ;)

| Signal / Line | Display's signal name | Connected Pin |
| :--- | :--- | :--- |
| $\text{SCLK}$ | SCLK | RC3 |
| $\text{MOSI}$ | SDIN | RC5 |
| $\text{MISO}$ | X | X |
| $\overline{\text{SS}}$ | /SCE (not Serial Chip Enable) | RC1 |

Now that we know the names of our signals and the pins to be used, we should once again clarify the timing for SPI communication.

![](images/SPI_Timing.png)
(source: [embeddedrelated.com](https://www.embeddedrelated.com/showarticle/87.php))
As shown in above picture, the following requirements for a communication between SPI-Master and SPI-Slave need to be fulfilled:
1. /CS needs to be pulled down (logical 0)
2. Values are read on each rising edge of the clock (clock does not need to tick regularly, as long as /CS is low and 8 bit are send)
3. The order in that the bits are sent is very relevant! You __must__ send the MSB (7th bit) first and the LSB (0th bit) last!

To sum it up, you need to pull down the corresponding /CS pin and SCLK pin, set the level for the MSB on the pin related to the display's SDI line and set the SCLK pin's logical level to one, afterward. This way the MSB is read. Now continue with pulling down SCLK, setting the required value (or level, i.e. 0 or 1) and raising SCLK to high afterward. Do this for all 8 bit and set /CS to high again, to end communication.

# Exercise
For today, there is only on exercise - implement the necessary Bit Banging to make the display usable (again).
Therefore, you need to implement "GLCD_Bit_Banging()" at the end of the main.c-file.
https://github.com/MicrocontrollerApplications/Lab8_Rotary_Encoder/blob/bc4483f0e9cea8e3c104339f2085b191f684d465/Lab8_Bit_Banging.X/main.c#L42-L46

>[!NOTE]
>You won't find any other GLCD related code within main.c. Honestly, what you're doing today is not good practice in C-programming. But it enables you to focus on the Bit Banging itself, without the need to understand any other parts of the GLCD library.
>If you want to know more about it, feel free to ask your laboratory supervisor for help!

Use the corresponding pins you've found before and set their levels according to the SPI timing explained above.
Keep in mind to activate the SPI-Slave before and de-activate it after sending the single bits.
As soon as you think you've got the right solution, test it by starting the code in Debug mode.

If your Bit Banging is implemented correctly, try to calculate the duration necessary to send one byte to the display. After that switch to the Simulator configuration and test your result using the stopwatch.
