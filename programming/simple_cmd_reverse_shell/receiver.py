import socket
LISTEN_IP="0.0.0.0"
LISTEN_PORT=8080
BUFFER_SIZE=1024

tuning=0.1
import time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((LISTEN_IP, LISTEN_PORT))
s.listen(1)
conn, addr = s.accept()
print("Connection from" + " " + str(addr))
conn.send(b"\n")
while True:
    data = ""
    time.sleep(tuning)

    while True:
        recv_data = conn.recv(BUFFER_SIZE)
        if not recv_data: break
        data += recv_data.decode("GB2312")
        if (len(recv_data) < BUFFER_SIZE): break

    if not data: break
    print(data, end="")

    tosend = input()
    if tosend == "exit": break
    tosend = tosend + '\r\n'
    conn.send(tosend.encode("GB2312"))

conn.close()