import subprocess
import os
import signal
import socket

host = "192.168.43.245"
port = 8002

cwnd = [20,30,35,40,42,45,50,60]
msss = [7000,10000,20000,30000,40000,50000]


socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((host, port))
print("CONNECTED")


for wnd in cwnd :
    for mss in msss : 
        for i in range(7):
            socket.send((str(wnd)+","+str(mss)).encode('utf-8'))
            subprocess.run(["./server_3","1234", str(wnd), str(mss)])
            socket.recv(255)


socket.send(b"")

socket.close()