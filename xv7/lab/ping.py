import socket
import sys
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr = ('localhost', int(sys.argv[1]))
buf = "this is a ping!"

while True:
	print("pinging...", file=sys.stderr)
	sock.sendto(buf.encode(), ("127.0.0.1", int(sys.argv[1])))
	time.sleep(1)
