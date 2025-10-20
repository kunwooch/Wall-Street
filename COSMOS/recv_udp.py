import socket
import struct

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind(("10.37.21.1", 2001))
cnt = 0;

while True:
    data, addr = sock.recvfrom(16)
    cnt = cnt+1
    print (cnt)
    print (struct.unpack("ffff", data))
