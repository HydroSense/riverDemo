#!/usr/bin/env python3

#Colby Rome 11-13-2015

import serial
from time import sleep
import numpy as np
from matplotlib import pyplot as plt

NUM_SECS = 300

def receiving(ser):
    new = True
    plt.ion()

# Create lists for holding sensor data
    temp = [0]*NUM_SECS
    rain = [0]*NUM_SECS
    wind = [0]*NUM_SECS
    plt.figure(figsize=(12,12)) # Adjust the figsize

    # Temperature
    plt.subplot(311) # Divides figure into 3x1 grid
    plt.title('Realtime Weather Station Data')
    plt.xlabel('seconds')
    plt.ylabel('Temperature (C)')
    temp_x, = plt.plot(temp)
    # plot() returns a list of Line2D objects. The comma unpacks
    # the single value into temp_x.

    # Rain
    plt.subplot(312)
    plt.xlabel('seconds')
    plt.ylabel('Daily Rainfall (in)')
    rain_x, = plt.plot(rain)

    # wind
    plt.subplot(313)
    plt.xlabel('seconds')
    plt.ylabel('windspeed (mph)')
    wind_x, = plt.plot(wind)

    while True:
        # Read from serial line; decode binary into ascii string
        parsed = [i for i in ser.readline().decode('ascii').split('+')];
#        parsed = parse_sdi12_line(ser.readline().decode('ascii'))

        # Example response from sensor (indicating depth, temp, conductivity);
        # 0+130+22.3+283 OR
        # 0-11+22.3+0 (This is invalid; cannot have negative depth)
        # The sensor specifies that physical damage will occur if the 
        # temperature is less than 0C, so we will not plot values <0.
        
        print('received: ', parsed)
        if len(parsed) == 3: # if valid string

            newTemp = parsed[0]
            newRain = parsed[1]
            newWind = parsed[2];
            if(new == True): # only occurs once
                # This is to initialize the y-values to the initial readings
                new = False
                temp = [newTemp]*NUM_SECS
                rain = [newRain]*NUM_SECS
                wind = [newWind]*NUM_SECS

            # Append the new readings to the list. This is operating as
            # a circular buffer
            temp.append(newTemp)
            del temp[0]
            rain.append(newRain)
            del rain[0]
            wind.append(newWind)
            del wind[0]

            # Define appropriate range for each graph
            minTemp=float(min(temp))-1
            maxTemp=float(max(temp))+1
            minRain = 0
            maxRain = float(max(rain))+1
            minWind = 0
            maxWind = float(max(wind))+10

            # Temperature
            plt.subplot(311)
            plt.ylim([minTemp,maxTemp])
            temp_x.set_xdata(np.arange(len(temp)))
            temp_x.set_ydata(temp)

            # Rain
            plt.subplot(312)
            plt.ylim([minRain,maxRain])
            rain_x.set_xdata(np.arange(len(rain)))
            rain_x.set_ydata(rain)
            
            # Wind
            plt.subplot(313)
            plt.ylim([minWind,maxWind])
            wind_x.set_xdata(np.arange(len(wind)))
            wind_x.set_ydata(wind)

            #Draw to the screen
            plt.draw()

if __name__ == '__main__':
    # Modify the serial port as necessary
    # TODO: make the serial port a command line argument?
    ser = serial.Serial('/dev/ttyACM0', 9600);
    receiving(ser)

