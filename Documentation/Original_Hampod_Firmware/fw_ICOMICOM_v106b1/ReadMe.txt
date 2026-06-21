    FIRMWARE UPGRADE OVERVIEW
The following instructions can also be found in APPENDIX C of your Manual.

The procedure to flash the firmware is standard practice as with other devices that utilize a bootloader. There are basically two components used for this procedure:
1.  The bootloader which is already resident within your HamPod.
2.  A Windows based computer running an application that communicates with the bootloader and sends the new firmware to the HamPod.

The bootloader is a special application that resides within the HamPod and runs every time the device is either reset or  powered on. This special application looks for a unique command sequence over serial port 1 of the HamPod which is connected to your computer which is running the companion Windows flash loader  application. Once the bootloader begins, you have approximately 1.5 seconds to initiate the flash loader application running on the computer. During that first 1.5 second window of opportunity,the bootloader is in control of the HamPod and is looking for a connection from the computer over the serial port. When a valid connection is established between the HamPod and the application running on the computer, the flash loader application then sends the new firmware to the HamPod. If no connection can be established, the bootloader times out and the HamPod starts running as usual.

  An improved method to flash the HamPod has been added to the configuration menu. This greatly simplifies the required timing synchronization  between the HamPod and the flashing application running on the computer.
All previous methods for flashing the HamPod still exist but this new method is much easier to perform since it automatically restarts the HamPod for you.

The following steps will walk you through the flashing process:
1.  Important! You must use a null modem serial cable for this connection. Connect a null modem serial cable between serial port 1 of the HamPod (the connector nearest the DC Power jack), and a serial comm port on your personal computer. The comm port on your computer can be any serial port from COM1: to COM8:. The existing straight wired serial cable provided with your HamPod can be used provided you also use the included null modem adapter in addition to this cable.
 2.  Download and extract the ZIP archive update  firmware file to any directory of your choosing on your computer. You may over write any existing files with a new firmware update if you have previously performed this update before. This file can be found from the appropriate page on the HamPod web site at the following URL:
http://www.HAMPod.com

The files contained within the ZIP archive are:
ds30LoaderConsole.exe : The command line flash loader application.
Flash_1.bat : Batch file when using comm port 1 on PC.
Flash_2.bat : Batch file when using comm port 2 on PC.
Flash_3.bat : Batch file when using comm port 3 on PC.
Flash_4.bat : Batch file when using comm port 4 on PC.
Flash_5.bat : Batch file when using comm port 5 on PC.
Flash_6.bat : Batch file when using comm port 6 on PC.
Flash_7.bat : Batch file when using comm port 7 on PC.
Flash_8.bat : Batch file when using comm port 8 on PC.
***.hex : the firmware file to flash to the HamPod (this file name varies depending on your connected devices)
Other informational text files

3.  Once you extract the files into a directory of your choosing, simply run the appropriate batch file for the comm port number on your PC that you have connected to the HamPod. The batch file contains the command to launch the flash loader  application and also set some necessary parameters for the program. When you start the batch file, the file will open a DOS window, validate the new hex firmware file, and prepare to flash the HamPod but will stop and wait for any key to be pressed on the PC before actually initiating the flashing process. The text written into this DOS window by the flashing application should be readable by most screen readers such as JAWS or Window EYES and will inform you of its progress.

4.  Once the application on the computer pauses and is waiting for a key press, enter the configuration menu on the HamPod with a long press of the [C] key. Scroll through the menu items using the [A] or [B] keys until you reach the "Reset and Flashing Disabled" menu item. Press either the [C] or [D] key to enable this item and you will hear, "Enter to Reset or Star to Flash Firmware". Press the [*] key and the HamPod will enter flash mode with the prompt, "Waiting for Firmware, Any Key to Abort". The HamPod is now waiting and watching for the application on the computer to begin sending the new firmware. At this point, if you press any key on the HamPod, it will exit flash mode and reset itself. To initiate the new firmware transfer from the computer, press any key such as the Space Bar on the computer and it will begin sending the firmware to the HamPod. When you pressed the Space Bar key on the PC, the flash loader application tries to establish a connection with the bootloader within the HamPod and if found, the HamPod will respond with a "Flashing" announcement. This will take about 30 to 45 seconds to complete. DO NOT power off the HamPod while the update is in progress!When finish, the HamPod may restart on its own but sometimes you may need to power cycle the HamPod yourself. The flash loader application on the PC will write a series of period characters to the screen as it transfers the firmware file to the HamPod, When finished, it will report the status of the update and wait for a final key press on the PC to terminate the flash application and close the DOS window.

Note for users of older firmware:
  If you are using an older version of firmware, this new flashing method will not be available and you must use one of the previous methods of synchronizing the HamPod with the PC by performing an manual reset of the HamPod.  The easiest way to do this that usually works is to start the flash loader application on the computer and press any key when prompted to initiate the firmware download. You then have about 20 seconds to reset the HamPod while the PC application continues to look for the bootloader to start running on the HamPod. So at this point, you need to quickly reset the HamPod by use of the reset button, reset configuration menu item, or removing and applying power to the HamPod. In some cases an erroneous character will be sent to the PC by the HamPod during the reset which might confuse the application running on the computer and it will fail. If this happens, just try the procedure again or use one of the alternative methods to reset the HamPod. 

5.  After a successful update, you can verify the new firmware has been installed by requesting the product information from the HamPod as specified in the user manual for your specific connected device.
6.   Disconnect from the PC and reconnect the HamPod to your equipment and enjoy the new benefits and features of the firmware!