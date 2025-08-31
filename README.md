# Arduino-Alarm-Clock
A fully functioning alarm clock that displays the current time and allows the user to define and set alarms.

Main Components:
- 4 digit 7-segment display: displays the current time according to the RTC module
- 16x2 LCD screen: displays date and shows the text "ALARM" when the user is setting an alarm
- DS3231 Real-time Clock Module: Provides accurate real-time information
- Buttons: allow user to set alarms
- Buzzer: sounds the alarm
- LED: flashes when alarm goes off
- Breadboard, wires, resistors (for prototype version)

Buttons Explained:
- There are four buttons, which each trigger a hardware interrupt when pressed:
  - Mode button: Press to toggle between Mode 1 and Mode 2.
      - Mode 1: Default time display
      - Mode 2: Set alarm mode
  - Up button: Increases time display by single minute increments when Mode 2 is enabled
  - Down button: Decreases time display by single minute increments when Mode 2 is enabled
  - Set button: When Mode 2 is enabled, i.e. when setting an alarm, press once to lock the hours place and enable changing the minutes place, press again to lock the minutes place and officially set the alarm.
 
How it works:
- When you want to set an alarm, press the mode button once to switch from time display mode to alarm setting mode.
- You should now be able to alter the time display. Use the up and down button to set your desired hour value.
- Once you have inputted your desired hour for your alarm, Press the set button once to lock the hours value and enable changing the minute value. Again, use the up and down button to set your desired minute value.
- Press the set button again to lock the minutes place and officially set the alarm
- The alarm will now return to the default time display mode until the mode button is triggered again.
- When the time for the alarm comes around, the alarm will go off by sounding a buzzer and LED. To turn off the alarm, press the mode button once.
 
Code Explanation:

![Code Flowchart 1](https://github.com/user-attachments/assets/c2b46a5b-5e94-4860-bff8-9f703abdf781)

![Code Flowchart 2](https://github.com/user-attachments/assets/237a3796-3292-4ae4-a4fd-e882613fd4b6)

![Code Flowchart 3](https://github.com/user-attachments/assets/f8d6a25d-2cd1-45a8-b6d5-945417e82728)
