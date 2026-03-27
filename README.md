The software is based on TPA firmware, it behaves the same way: power up, reset, communication, configurateion, etc... is all based on TPA firmware.
The software gives you at this stage basic interaction, like volume change and input change, the rest of the configuration is read from the switches on the BIII board.
This software is easily scalable, you can replace any of the classes with your own or modify existing.
I have added some "interfaces" for editing, like eeprom or touch (didnt test the touch, but its easy to figure out how it works and correct it).
I also prepared the settings page for the tft screen so one can add it later on.


Main function is literaly 20 lines of code, it just takes the action from interface and apply to the DAC class.


As I said, the heart of the DAC Class is TPA Software.
The configuration file is the copy of TPA, ES9028_38.h file.
DAC class constructor is build of TPA functions, slightly modified for Arduino needs ( i use Wire.h for I2C communication ), but the funtions:
powerDACup();
initializeDAC();
configureDAC();
are practically the same you get with TPA firmware, those work the same way and are called the way TPA is doing it.


Simplified soft looks like this:
//--------------------------------------
setup:

dac = new DAC();
remoteInterface = new RemoteInterface();
touchInterface = new TouchInterface();
tftGraphics = new TFTGraphics();

//---------------------
loop:

action = remoteInterface->getAction();
if( action == NONE )
action = touchInterface->getAction( MAIN_MENU );


if( action == NONE ){
dac->readSwitchStates();
}


switch ( action ){
case NONE:
break;
case CHANNEL_LEFT:
tftGraphics->printChannel( dac->decreaseInput() );
break;
case CHANNEL_RIGHT:
tftGraphics->printChannel( dac->increaseInput() );
break;
case VOLUME_UP:
tftGraphics->printVolume( dac->increaseVolume() );
break;
case VOLUME_DOWN:
tftGraphics->printVolume( dac->decreaseVolume() );
break;
case ENTER:
tftGraphics->printVolume( dac->muteVolume() );
break;
default:
break;
}
action = NONE;

//-------------------------


All you need is Atom + Platform IO + ( Arduino Due, remote, TFT) and the repository:


see for more details:
https://www.diyaudio.com/community/threads/c-easy-and-scalable-software-for-biii38pro.337580/



UPGRADE to Viewe UEDX80480070E-WB-A, LCD ESP32 S3 Board


Occupied:

Purpose	GPIOs
RGB data (16-bit)	1, 3, 4, 5, 6, 7, 8, 9, 14, 15, 16, 21, 45, 46, 47, 48
RGB timing	39 (HSYNC), 41 (VSYNC), 40 (DE), 42 (PCLK)
Touch I2C	19 (SDA), 20 (SCL)
Backlight PWM	2
UART (Serial)	43 (TX), 44 (RX)
DAC Wire1	10 (SDA), 18 (SCL)
DAC reset/mux	11, 12, 13
Free GPIOs:

17 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38

Warning: On ESP32-S3-WROOM-1 modules, GPIOs 22–25 are wired to internal QSPI flash/PSRAM and cannot be used as user I/O. Stick to 17 or 26–38.
