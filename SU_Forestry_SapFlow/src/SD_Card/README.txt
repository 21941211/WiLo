DO NOT CHANGE THIS FILE!!!

Gateway SSID Password for security reasons is only accessible by reading the official documentation or src file of device. 

This file explicitly states how to manage the CONFIG.txt file used for device parameters.
The CONFIG.txt is used to define operation parameters of the device.

The format of the CONFIG.txt file should not be change!
The CONFIG.txt must end with a new line delimiter (“\n”, i.e. an empty line)
Here is a copy of the original format. Copy and paste if needed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Device_ID,Gateway_ID,SleepTime[s],SampleRate[ms],Time_Bofore_Heat[ms],HeaterOn_Time[ms],HeaterOff_Time[ms]
1,1,20,1000,15000,10000,15000

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In the event of the CONFIG.txt file not being present on the device SD card, the device will malfunction!

The parameters contain the following:
 - Device_ID:	Unique ID [num] of device to identify it's datalog file when transmitted.
 - Gateway_ID:	The unique ID [num] of a gateway shared between a cluster of Nodes. Since any node is the cluster 
		can act as an gateway, this ID should be the same for the specific nodes sharing a server.
 - SleepTime:	This is the amount of time the device will be put to sleep in seconds.
 - SampleRate:	This is the frequency at which the device will call sensor.readTemp() and write to the SD card in milliseconds.
		Note, this does not change the sensors frequency of sample, i.e. it does not change sensor.setRate()
		The sensors are set to take a sample of the temp. every second [1Hz], this can only be changed in 
		the source code (not recommended). See the constants for changing the frequency of the sensor at the end of file.
 - Time_Bofore_Heat:	Time needed for reference temp. measurement, before the heat pulse is applied, in milliseconds.
 - HeaterOn_Time:	Time the heat pulse is on, in milliseconds.
 - HeaterOff_Time:	Time needed for temp. measurement after the heat pulse has been on, in milliseconds.

Incase the official proper documentation is not available, here is a short summary of device operation state:
 - The device has two main state: 1. Sensor Node
				  2. Gateway Node
 - To change between states, the rst button is pressed on the device.
 - LED indicators of states: State 1 -> LED will constantly be HIGH, to indicate server is running.
			     State 2 -> LED will be HIGH for 10 sec. then pulse when measurements is done.
 - State 1: Sensor Node
	> The device will boot, immediately reading CONFIG.txt, initializing parameters, and start to search for WiFi SSID of Gateway
	> If the SSID is found, the datalog file is uploaded to server and devices continues to do measurements
	> If after 10 sec. it is not found, the device still continues operation of measurements
	> After the measurement sequence has concluded, as state by the CONFIG.txt parameters, the device goes to sleep
	> This processes repeats
 - State 2: Gateway Node
	> The device will boot, immediately reading CONFIG.txt, initializing parameters, and start an WiFi access point with Gateway_ID SSID
	> The device goes into a constant loop of client handling
	> To access the server, connect to the TJT Gateway SSID with the password (specified at top) and go to http://tjt.local
	> To end this state the rst button must be pressed again


//  Constants for setting sensor sample rate. Again this is not used for SampleRate above, only for sensor.setRate()!
#define MANUAL 0
#define TWO_MINS 1
#define ONE_MINS 2
#define TEN_SECONDS 3
#define FIVE_SECONDS 4
#define ONE_HZ 5
#define TWO_HZ 6
#define FIVE_HZ 7