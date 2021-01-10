import subprocess
import socket
import time

file_size = 1202922

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.bind(('', 8002))

f=open("perf_client1.txt", "w")

socket.listen(5)
client, address = socket.accept()

while True :
    
    response = client.recv(255)
    response = response.decode("utf-8")
    if response == "":
        break
    f.write(response +",")

    time.sleep(0.2)
    print("CONNECTED")

    start = time.time()
    subprocess.run(["./client2","192.168.43.245","1234","bengal.jpg","0"])
    end = time.time()
    
    timestamp = end -start

    debit = (file_size/timestamp)/1000000

    f.write( str(debit)+",\n")
    client.send(b"Finished")

print("Close")
client.close()
socket.close()
f.close()