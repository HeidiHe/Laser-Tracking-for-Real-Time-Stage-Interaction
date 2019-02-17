#testing first
#then main.cpp

import socket

s = socket.socket()
host = socket.gethostname()
port = 7777
s.bind(('localhost',port)) 
s.listen(5)
while True:

    c, addr = s.accept()
    #print("Connection accepted from " + repr(addr[1]))\
    while True:
        data = c.recv(1026).decode("utf-8")

        print(repr(addr[1]) + ": " + data)
