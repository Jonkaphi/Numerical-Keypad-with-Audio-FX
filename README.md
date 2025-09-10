# Mechanical Numerical Keypad with local Audio FX
The desire to make this little keypad came from idea to reuse a some of the parts from a broken keyboard someone gave me. Another justification was that it was tangentially tied to another future project for a handheld cyberdeck computer. Since for the form factor I am aiming for it would need to be something more bespoke.

The extra little feature of having audio fx being played locally on the keyboard with each button press, was from one of the commenters on reddit related to a post I made for it at the start of the project. The sound files will be read from the placed SD card. Currently the audio that can be played is from 8 to 16 kHz kHz sample rate with 16 bit signed PCM, if you want your file to be play make sure to replace the string with the name of the file you want in "const FILE_NAME audiofile_path[10] = "fx_8k.wav";" on line 74 in "main.c"

The lower level sd card interface for the file system used in the project was made by Khaled Magdy's post on ![DeepBlueMbedded](https://deepbluembedded.com/stm32-sd-card-spi-fatfs-tutorial-examples/)

### Top View
![kuk35721-1920](https://github.com/user-attachments/assets/1b89741d-6d17-401e-94da-00814ed46c40)

### Bottom View

![PXL_20250710_151807012](https://github.com/user-attachments/assets/76e03539-6e57-440e-9c81-85b7b6255b48)

## Issues as of 08/2025
* Current USB port part is not vertically offset, which collides with the PCB, read "Connectors" page in the schematic.
* Encoder for PC system audio still glitches at times, but its significantly better compared to previous build versions
* Audio streaming and polling of the key inputs is done on a single thread, resulting in less than satisfactory responsiveness

## Future work
* Move to a MCU series that include SDIO for the SD card, has more flash to as to implement some sort of RTOS, and or has a second core to handle the audio streaming.
* General rework of the PCB, redesign of the audio front-end to have better amplification of the DAC data stream.
* Make a desktop app that can modify the functionality of pad, such as change the keycodes for each key button, selectable profiles, etc.
