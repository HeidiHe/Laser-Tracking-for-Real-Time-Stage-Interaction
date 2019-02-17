"""Small example OSC client

This program sends 10 random values between 0.0 and 1.0 to the /filter address,
waiting for 1 seconds between each value.
"""

import argparse
import random
import time

from pythonosc import osc_message_builder
from pythonosc import udp_client

import socket

client = udp_client.SimpleUDPClient("137.146.123.51", 8080)
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
        # print("sending osc loop 2")
        data = c.recv(1026).decode("utf-8")

        if(data):
            print("Data sent: " + data)
            newData = data.split(",")
            client.send_message("/x", newData[0])
            client.send_message("/y", newData[1])

            # up to six people
            # 2D array people[6][2]

            print(len(newData))

            # for i in range(len(newData)/2):
            #     client.send_message("/x", newData[i])
            #     client.send_message("/y", newData[i+1])
            

            # client.send_message("/open", newData[2])
            # client2.send_message("/Austin", newData[3])
            print("Data sent: " + data)
        #print(repr(addr[1]) + ": " + data)
