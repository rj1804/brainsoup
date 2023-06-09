## **intro**

**brainSOUP** is a Linux/QT based firmware updater for the B* ***BRAINS*** eurorack module.

![screenshot](screenshot.png?raw=true "screenshot")

so far it works to update to the latest tersion **1.0.2**, if a new version comes out I will make an update here as well :)

## **building**

- you need a **QT(5)** development environment, make sure you have **libqt5-qtserialport-devel** or similar
- **qmake** to generate the Makefile (qmake-qt5)
- **make** to build brainsoup
- your mileage will vary...

## **using**
- boot your B* ***Brains*** in update mode, hold both buttons and power on
- start **brainsoup**, it should detect the USB serial port the ***Brains*** is using
- download the S_NTHTR_BE.exe (v2.5.9) from the B* website
- click "load .exe file" and pick the exe file you downloaded
- **brainSOUP** will check for the ***Brains*** firmware inside the .exe and if it is found enable the "Update" button
- press "Update" and wait
- after the update, the ***Brains*** module should reboot with the new firmware 1.0.2
- if something goes wrong, try the update again, or restart the module in update mode and restart **brainSOUP** as well.

- you cannot **brick** the module, if the update keeps failing you can still update it with the Mac or Windows software...


## **have fun!**

proceed at your own risk, yet remember: the module is cheap, fun is invaluable

**MAKE LOVE NOT WAR**
