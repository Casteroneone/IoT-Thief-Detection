# ESP32 People-Distance Alarm System

An ESP32 project that measures how close a person is with an ultrasonic sensor, shows the distance zone on an RGB LED, plays an alarm sound when someone gets too close, and saves each event to an SD card with the date and time. It can be woken from light sleep with an IR remote and put to sleep with a push button or automatically.

## Features

- Distance zones shown on an RGB LED: red (0-20 cm), yellow (20-80 cm), green (over 80 cm).
- Alarm sound (DFPlayer Mini) once each time someone enters the danger zone.
- Event log saved to `/events.csv` on a microSD card, with date and time from a DS3231 RTC.
- IR remote: wake the system, play the sound, run a 5 second rainbow.
- Light sleep: by push button, by remote, or automatically after 10 seconds in the green zone.

## Hardware

| Component | Purpose |
|---|---|
| ESP32 DevKit V1 | Controller |
| HC-SR04P ultrasonic sensor | Measures distance |
| IR receiver + IR remote | Remote control input |
| DS3231 RTC module | Date and time for the log |
| DFPlayer Mini + speaker | Plays the alarm sound |
| microSD card for DFPlayer | Holds the alarm mp3 |
| microSD module + microSD card | Saves the event log |
| RGB LED (common cathode) | Shows the distance zone |
| Push button | Puts the system into light sleep |
| 1 kOhm resistor | In series on the DFPlayer RX line |
| 3 x 220 Ohm resistors | One for each RGB LED color |

## Wiring

| Device | Module pin | ESP32 GPIO |
|---|---|---|
| DS3231 RTC | SDA | 21 |
| DS3231 RTC | SCL | 22 |
| DFPlayer Mini | TX | 16 |
| DFPlayer Mini | RX | 17 (through 1 kOhm) |
| HC-SR04P | TRIG | 25 |
| HC-SR04P | ECHO | 26 |
| IR receiver | OUT | 27 |
| SD module | CS | 5 |
| SD module | SCK (may be labeled CLK) | 18 |
| SD module | MISO (may be labeled DO) | 19 |
| SD module | MOSI (may be labeled DI) | 23 |
| RGB LED | R / G / B | 32 / 33 / 4 (each through 220 Ohm) |
| Push button | One leg | 13 |
| Push button | Other leg | GND |

All pins can be changed in `Config.h`.

### Power

- Connect all GND pins together.
- DS3231 and IR receiver: 3.3 V.
- DFPlayer Mini and SD module: 5 V (VIN pin when powered by USB).
- HC-SR04P: 3.3 V, or 5 V with a voltage divider on the ECHO line.
- The DS3231 SQW and 32K pins are not used. Fit a CR2032 battery so the clock keeps time.

## Software setup

### 1. Board package

1. Open **Arduino IDE** > **Tools** > **Board** > **Boards Manager**.
2. Install **esp32 by Espressif Systems**, version **3.x** (needed for `analogWrite()`).
3. Select **ESP32 Dev Module** (or **DOIT ESP32 DEVKIT V1**) as the board.

### 2. Libraries

Install these in **Sketch** > **Include Library** > **Manage Libraries**:

| Library (search name) | Author | Used for |
|---|---|---|
| **RTClib** | Adafruit | DS3231 clock |
| **Adafruit BusIO** | Adafruit | Required by RTClib (install if prompted) |
| **DFRobotDFPlayerMini** | DFRobot | DFPlayer Mini audio |
| **IRremoteESP8266** | David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff | IR receiver |

Already included with the ESP32 board package (no install needed): `Wire`, `SPI`, `SD`, `esp_sleep.h`, `driver/gpio.h`.

> Do not install the library named "IRremote" by Arminjo. It is a different library and will not work with this code.

### 3. Open the project

1. Unzip the project folder. Do not open the `.ino` from inside the zip.
2. Keep every file in the folder named `people_distance_alarm_tabs`, next to `people_distance_alarm_tabs.ino`.
3. In the Arduino IDE use **File** > **Open** and choose `people_distance_alarm_tabs.ino`. The other files appear as tabs.
4. Connect the ESP32, select the correct port, and click **Upload**.

## Preparing the SD cards

Use two separate microSD cards, both **32 GB or smaller** and formatted as **FAT32**.

**DFPlayer card (alarm sound)**

```
/01/001.mp3
```

Create a folder named `01` and put a file named `001.mp3` inside it. Copy files one at a time.

**Logging card (SD module)**

Leave it empty. The code creates `/events.csv` automatically.

## How to use

1. Power the ESP32 and open the **Serial Monitor** at **115200 baud**.
2. At startup you should see `DS3231 detected`, `Logging SD card ready` and `DFPlayer ready`.
3. The system starts disabled and goes to light sleep. Press the wake button on the remote to start it.
4. Walk toward the sensor. The LED shows the zone and the alarm plays when you enter 0-20 cm.

### Remote buttons

| Action | Config name | Default code |
|---|---|---|
| Wake from light sleep and start | `CMD_WAKE` | `BA45FF00` |
| Play the alarm sound | `CMD_SOUND` | `B946FF00` |
| Rainbow loop for 5 seconds | `CMD_RAINBOW` | `B847FF00` |
| Go to light sleep (optional) | `CMD_SLEEP` | `BB44FF00` |

Your remote may use different codes. To find them, press each button with the Serial Monitor open and read the line:

```
IR received: 0x00FFA25D  (BA45FF00-style: 0xBA45FF00)
```

Copy the value after **BA45FF00-style** into `Config.h`. Codes are accepted in either format.

### Going to sleep

| Method | How |
|---|---|
| Push button | Press the button on GPIO 13 |
| Automatic | Stay in the green zone (over 80 cm) for 10 seconds |
| Remote (optional) | Press the `CMD_SLEEP` button, if the remote sends stable codes |

To wake the system, press the wake button on the remote.

### Distance zones

| Zone | Distance | LED | Action |
|---|---|---|---|
| Danger | 0-20 cm | Red | Alarm plays and one row is saved to the SD card, once per entry |
| Warning | 20-80 cm | Yellow | LED only |
| Safe | Over 80 cm | Green | Starts the 10 second sleep timer |

The alarm and log trigger again only after the person leaves the danger zone and re-enters.

### Log file

`/events.csv` on the logging card:

```
date,time,distance_cm,event
2026-10-17,14:32:08,12.4,DANGER_ENTRY
```

If the RTC is missing, the date and time columns say `NO_RTC`. To read the log, remove the card and open the file on a computer.

## Settings (Config.h)

| Setting | Meaning | Default |
|---|---|---|
| `DANGER_MAX_CM` | Upper limit of the danger zone | 20 |
| `WARNING_MAX_CM` | Upper limit of the warning zone | 80 |
| `GREEN_SLEEP_MS` | Time in green zone before sleep | 10000 |
| `NO_ECHO_COUNTS_AS_GREEN` | Treat "no echo" as green | true |
| `DF_VOLUME` | Alarm volume, 0-30 | 25 |
| `RAINBOW_DURATION_MS` | Rainbow length | 5000 |
| `WAKE_ON_UNDECODABLE` | Wake on any IR pulse if the code can't be decoded | true |
| `CMD_WAKE`, `CMD_SOUND`, `CMD_RAINBOW`, `CMD_SLEEP` | Remote codes | see above |
| `BUTTON_PIN` | Sleep button pin | 13 |

## Project files

| File | Purpose |
|---|---|
| `people_distance_alarm_tabs.ino` | Main loop and distance logic |
| `Config.h` | Pins, zones, remote codes, settings |
| `IRControl.h/.cpp` | IR remote, light sleep and wake |
| `Ultrasonic.h/.cpp` | Non-blocking distance reading |
| `RgbLed.h/.cpp` | Zone colors and rainbow |
| `Audio.h/.cpp` | DFPlayer alarm sound |
| `SDLogger.h/.cpp` | Event log on the SD card |
| `Button.h/.cpp` | Push button for light sleep |

## Troubleshooting

| Problem | Check |
|---|---|
| `DS3231 not detected` | 3.3 V power, SDA on 21 and SCL on 22, shared GND |
| `logging SD card unavailable` | Module on 5 V, wiring on 5/18/19/23, card FAT32 and 32 GB or smaller |
| `DFPlayer initialization failed` | DFPlayer on 5 V, TX/RX not swapped, card inserted with `/01/001.mp3` |
| No sound | Volume in `Config.h`, speaker on SPK1 and SPK2, card layout |
| IR codes keep changing or show `Ignored non-NEC frame` | Receiver wired to 3.3 V, GND and GPIO 27, keep its wires away from the ultrasonic and LED wires |
| Remote does not wake the system | Press `BA45FF00` button, keep `WAKE_ON_UNDECODABLE = true` |
| Card works in one device but not another | Reformat as FAT32, use 32 GB or smaller, try a SanDisk, Samsung or Kingston card |
| ESP32 will not boot or upload | Disconnect the button and sensors during upload |

## Testing the IR receiver

The `ir_test` sketch (separate folder) prints the protocol, code and raw timings of every remote button. Upload it on its own when you need to find a remote's codes.
