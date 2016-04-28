#!/usr/bin/env python3

#Colby Rome 11-1-2015
#Modified 4-28 by Colby Rome and Matt Hawkins

import serial
from time import sleep
import numpy as np
from matplotlib import pyplot as plt
import re
import argparse

NUM_SECS = 300 # Size of time axis for graphing

def receiving(sample, debug):
    ''' This function plots data from the SDI-12 sensor in realtime.
        Input: a valid instance of a serial class
        Output: a real-time plot of the data from an SDI-12 sensor
    '''

    new = True # Initialize graphs to initial y-values (not 0)
    plt.ion() # Interactive plot

    plt.rcParams.update({'axes.titlesize' : 'large'})
    plt.rcParams.update({'axes.labelsize' : 'large'})

    # Create lists for holding sensor data
    waterTemp = [0]*NUM_SECS
    depth = [0]*NUM_SECS
    cond = [0]*NUM_SECS
    airTemp = [0]*NUM_SECS
    windSpeed = [0]*NUM_SECS
    windDir = [0]*NUM_SECS

#    plt.figure(figsize=(12,12)) # Adjust the figsize
    plt.figure(figsize = (20,12))

    # The subplot() command specifies numrows, numcols, fignum where fignum ranges from 1 to numrows*numcols.
    # Water Temperature
    plt.subplot(321) # Divides figure into 3x1 grid
    plt.title('Realtime Water Sensor Readings')
#    plt.xlabel('seconds')
    plt.ylabel('Temperature (C)')
    waterTemp_x, = plt.plot(waterTemp)
    # plot() returns a list of Line2D objects. The comma unpacks
    # the single value into temp_x.

    # Depth
    plt.subplot(323)
#    plt.xlabel('seconds')
    plt.ylabel('Depth (mm)')
    depth_x, = plt.plot(depth)

    # Conductivity
    plt.subplot(325)
    plt.xlabel('seconds')
    plt.ylabel('Conductivity (dS/m)')
    cond_x, = plt.plot(cond)

    # Air Temp
    plt.subplot(322)
    plt.title('Realtime Weather Station Readings')
    plt.ylabel('Air Temp')
    airTemp_x, = plt.plot(airTemp)

    # Wind speed
    plt.subplot(324)
    plt.ylabel('Wind Speed')
    windSpeed_x, = plt.plot(windSpeed)

    # Wind direction
    plt.subplot(326)
    plt.xlabel('seconds')
    plt.ylabel('Wind direction')
    windDir_x, = plt.plot(windDir)

    while True:
        # Read from serial line; decode binary into ascii string
        if(debug):
            parsed = parse_sdi12_line(sample.readline())
            sleep(1)
        else:
            parsed = parse_sdi12_line(sample.readline().decode('ascii'))

        # Example response from sensor (indicating depth, temp, conductivity);
        # 0+130+22.3+283 OR
        # 0-11+22.3+0 (This is invalid; cannot have negative depth)
        # The sensor specifies that physical damage will occur if the
        # temperature is less than 0C, so we will not plot values <0.

        print('received: ', parsed)
        if len(parsed) == 6: # if valid string

            newDepth = -parsed[0]
            newWaterTemp = parsed[1]
            newCond = parsed[2]
            newAirTemp = parsed[3]
            newWindSpeed = parsed[4]
            newWindDir = parsed[5]
            if(new == True): # only occurs once
                # This is to initialize the y-values to the initial readings
                new = False
                waterTemp = [newWaterTemp]*NUM_SECS
                depth = [newDepth]*NUM_SECS
                cond = [newCond]*NUM_SECS
                airTemp = [newAirTemp]*NUM_SECS
                windSpeed = [newWindSpeed]*NUM_SECS
                windDir = [newWindDir]*NUM_SECS

            # Append the new readings to the list. This is operating as
            # a circular buffer
            waterTemp.append(newWaterTemp)
            del waterTemp[0]
            depth.append(newDepth)
            del depth[0]
            cond.append(newCond)
            del cond[0]
            airTemp.append(newAirTemp)
            del airTemp[0]
            windSpeed.append(newWindSpeed)
            del windSpeed[0]
            windDir.append(newWindDir)
            del windDir[0]

            # Define appropriate range for each graph
            minWaterTemp=float(min(waterTemp))-1
            maxWaterTemp=float(max(waterTemp))+1
            minDepth = float(min(depth))-7
            maxDepth = float(max(depth))+7
            minCond = float(min(cond))-50
            maxCond = float(max(cond))+50
            minAirTemp = float(min(airTemp))-1
            maxAirTemp = float(max(airTemp))+1
            minWindSpeed = 0
            maxWindSpeed = float(max(windSpeed))+5
            minWindDir = 0
            maxWindDir = 370

            # Temperature
            plt.subplot(321)
            plt.ylim([minWaterTemp,maxWaterTemp])
            waterTemp_x.set_xdata(np.arange(len(waterTemp)))
            waterTemp_x.set_ydata(waterTemp)

            # Depth
            plt.subplot(323)
            plt.ylim([minDepth,maxDepth])
            depth_x.set_xdata(np.arange(len(depth)))
            depth_x.set_ydata(depth)

            # Conductivity
            plt.subplot(325)
            plt.ylim([minCond,maxCond])
            cond_x.set_xdata(np.arange(len(cond)))
            cond_x.set_ydata(cond)

            # Air temp
            plt.subplot(322)
            plt.ylim([minAirTemp,maxAirTemp])
            airTemp_x.set_xdata(np.arange(len(airTemp)))
            airTemp_x.set_ydata(airTemp)

            # Wind speed
            plt.subplot(324)
            plt.ylim([minWindSpeed,maxWindSpeed])
            windSpeed_x.set_xdata(np.arange(len(windSpeed)))
            windSpeed_x.set_ydata(windSpeed)

            # Wind direction
            plt.subplot(326)
            plt.ylim([minWindDir,maxWindDir])
            windDir_x.set_xdata(np.arange(len(windDir)))
            windDir_x.set_ydata(windDir)

            #Draw to the screen
            plt.draw()
#            plt.savefig("test.svg")

"""
parse SDI-12 measurements like the sample below.

tested with Python3.4

Alan Marchiori
2015

"""
__r = re.compile('(?:[\+-]\d+)(?:\.\d*)?')

sample = """+119+25.3+299
-119+25.3+296
+119+25.3-295
+119+25.3+298
+119-25.3+302
+118+25.3+297
+119+25.3+299
-118+25.3+299
+119-25.3+295"""

def parse_sdi12_line(s):
    """
    Parse a single line of measurements like: +119-25.3+302

    regex explained:
    (?:[\+-]\d+)(?:\.\d*)?
     ----------  -------
                 ?: create a non-capture group for the decimal part
                 \.\d* = a single decimal (. has to be escaped) followed by ZERO or more digits
                 the trailing ? after this non-capture group means the whole group is optional
     ?: create a non-capture group for the whole number part
     [\+-] matches either + or - (plus has to be escaped)
     \d+ matches ONE or more digits

    """
    return [float(i) for i in __r.findall(s)]

if __name__ == '__main__':
    # Modify the serial port as necessary
    # TODO: make the serial port a command line argument?
    parser = argparse.ArgumentParser(description='Plot realtime weather/water data')
    parser.add_argument('--serialPort', '-s', nargs='?', const='0', default='0', help='serial port (/dev/ttyACM[x]) [default: ttyACM0]')
    parser.add_argument('--debug', '-d', nargs='?', const='sample.dat', default=None, help='sample file [sample.dat default]')

    args = parser.parse_args()
    print(args)
    if args.debug is None: # use serial
        ser = serial.Serial('/dev/ttyACM'+args.serialPort, 9600)
        receiving(ser, False)
    else:
        sample = open(args.debug, 'r')
        receiving(sample, True)

    # ser = serial.Serial('/dev/ttyACM1', 9600);
    # receiving(ser)
    # sample = open('sample.dat', 'r')
    # debug = True
    # receiving(sample)
