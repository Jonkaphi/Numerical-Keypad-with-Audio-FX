# Mechanical Numerical Keypad with local Audio FX
The desire to make this little keypad came from idea to reuse a some of the keycaps from a broken keyboard someone gave me. Another justification was that it was tangentially tied to another future project for a handheld cyberdeck computer. Since for the form factor I am aiming for it would need to be something more bespoke.

The extra funky little feature of having audio fx being played locally on the keyboard with each button press, was from a random commenter on reddit related to a post I made for it at the start of the project. The sound file will be read from the SD card placed into the keypad, and files will be accessed with a FAT32 files system.  

The file system used in the project was made by Khaled Magdy and taken from his post on ![DeepBlueMbedded](https://deepbluembedded.com/stm32-sd-card-spi-fatfs-tutorial-examples/)
### Top View
![kuk35721-1920](https://github.com/user-attachments/assets/1ec762f2-8f67-406b-99af-951139aefd11)
### Bottom View
![PXL_20250710_151807012](https://github.com/user-attachments/assets/13076207-b315-4b91-a7cb-812efdf57400)


## Issues as of 07/2025
* Current USB port part is not vertically offset, which collides with the PCB, read "Connectors" page in the schematic.
* Encoder for PC system audio still glitches at times, but its significantly better compared to previous build versions
* Audio front-end has not been tested, operation not verified
* Current file system takes a significant portion of FLASH memory
