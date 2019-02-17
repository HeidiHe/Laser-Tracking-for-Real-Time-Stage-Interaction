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

client = udp_client.SimpleUDPClient("192.168.8.160", 8080)
client2 = udp_client.SimpleUDPClient("192.168.8.106", 9999)

s = socket.socket()
host = socket.gethostname()
port = 8081
s.bind(('localhost',port))
s.listen(5)
while True:
    print("sending osc loop 1")
    c, addr = s.accept()
    print("Connection accepted from " + repr(addr[1]))
    
    while True:
        print("sending osc loop 2")
        data = c.recv(1026).decode("utf-8")

        if(data):
            print("Data sent: " + data)
            newData = data.split(",")
            client.send_message("/x", newData[0])
            client.send_message("/y", newData[1])
            client.send_message("/open", newData[2])
            client2.send_message("/Austin", newData[3])
            print("Data sent: " + data)
        #print(repr(addr[1]) + ": " + data)
