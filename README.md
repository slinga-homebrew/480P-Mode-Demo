# 480P Mode Demo
Decompilation attempt of Charles MacDonald's 480P Mode demo for the Sega Saturn. The original ISO is available here: https://segaxtreme.net/threads/progressive-hires-test-demo.15539/. This code allows you to try a number of different resolutions on the Saturn. I don't plan to clean up the code as I want to keep it as close as possible to to the original binary. 

Build the code with joengine or grab an ISO from [releases](https://github.com/slinga-homebrew/480P-Mode-demo/releases).

![Screenshot](screenshot.png)

## Issues
The decompiled code appears to work the same as the original binary on Mednafen and on real hardware.

## Credits
* Charles MacDonald for the original program and for answering questions
* Steve Kwok for explaining 480P settings
* [SegaXtreme](http://www.segaxtreme.net/) forums - best place for Sega Saturn dev questions
* [libyaul](https://github.com/ijacquez/libyaul) - bios_clock_speed_chg() function and VDP2 register definitions
