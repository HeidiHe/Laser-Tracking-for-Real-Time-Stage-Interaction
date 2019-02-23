"""Small example OSC client

This program sends 10 random values between 0.0 and 1.0 to the /filter address,
waiting for 1 seconds between each value.
"""

import argparse
import random
import time
import math
import statistics

from pythonosc import osc_message_builder
from pythonosc import udp_client

import socket



# /*
#     check angle ranges, merge closer ones

#     build up an array rawData
#     loop through the array and detect gap: 
#         if prev is 3 degree smaller than cur, then there is a gap, then 
#         update variable 3; variable 3 is group number

#         if x is larger than 6, 
#         then divide the gap again with a larger degree gap
    
#     build up a new array -> finalData[x][3]
#     for each group:
#         [0] average the distance
#         [1] average the angle
#         [2] get group size (lastAngle - firstAngle)
    
#     from angle calculate x,y distance -> x = abs(distance*cosA); y = abs(distance*sinA)

# */

def groupData(arr):

    angle = []
    distance = []
    newArr = [] # newArr = [[angle1, distance1, group0],[angle2, distance2, group0]]
    groupIndex = 0
    groupSize = []
    finalArr = [] # final array = [[x, y, groupSize], [x, y groupSize]]
    # print("length of rawData is " + str(len(arr)))

    maxgap = 3 # 5 degree maximum gap
    # print("maximum gap is " + str(maxgap))

    #loop through rawData and seperate angle and distance
    for i in range(len(arr)):
        # angle
        if(i%2 == 0):
            x = float(arr[i])
            dist = float(arr[i+1])
            # print("angle, distance: %f %f " %(x, dist))
            #first angle, create new group
            if(i==0):
                angleGroups = [[x]]
                distanceGroups = [[dist]]
            else:
                if(dist!=0):
                    if abs(x - float(angleGroups[-1][-1]) )<= maxgap:
                        angleGroups[-1].append(x)
                        distanceGroups[-1].append(dist)
                    else:
                        # print("creating new group")
                        angleGroups.append([x])
                        distanceGroups.append([dist])

    #get avg of each group
    for i in range(len(angleGroups)):
        eachAngleGroup = angleGroups[i]
        eachDistanceGroup = distanceGroups[i]  
        avgAngle = statistics.median(eachAngleGroup) #turn degree into radius     
        avgDistance = statistics.median(eachDistanceGroup)

        x = avgDistance * math.cos(math.radians(avgAngle))
        y = avgDistance * math.sin(avgAngle)
        size = len(eachAngleGroup)

        if(size>5):
            print("avgAngle, avgDistance: %f %f %f" % (avgAngle, avgDistance, size))
            print("each angle group")
            print(*eachAngleGroup)
            print("each distance group")
            print(*eachDistanceGroup)
            finalArr.append([x, y ,size])
            print("found people!!!!")
        
    return finalArr

# Python program to get average of a list 
def Average(lst): 
    return sum(lst) / len(lst)             


# client = udp_client.SimpleUDPClient("137.146.123.51", 8080)
# client2 = udp_client.SimpleUDPClient("192.168.8.106", 9999)

s = socket.socket()
host = socket.gethostname()
port = 7777
s.bind(('137.146.126.135',port))
s.listen(5)
while True:
    # print("sending osc loop 1")
    c, addr = s.accept()
    print("Connection accepted from " + repr(addr[1]))
    
    while True:
        # print("rendering osc loop 2")
        data = c.recv(1026).decode("utf-8")
        # print("debug 1")


        if(data):
            # print("Data sent: " + data)
            newData = data.split(",")
            newData.pop()
            strData = ' '.join(newData)
            # print("newData 1 is " + newData[0])
            # print("newData -1 is " + newData[-1])
            # print("newData is " + strData)
            #group data and return an 2d array with poeple's position: x, y, size
            finalData = groupData(newData)
            print("final data is" )
            print(*finalData)
            # client.send_message("/x", newData[0])
            # client.send_message("/y", newData[1])

            # up to six people
            # 2D array people[6][2]



            # client.send_message("/open", newData[2])
            # client2.send_message("/Austin", newData[3])

            # print("Data sent: " + data)
        #print(repr(addr[1]) + ": " + data)




