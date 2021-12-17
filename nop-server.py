#!/usr/bin/python

# nop-server.py - This is a server that we use to create head-of-line
#                 blocking for the concurrency test. It accepts a
#                 connection, and then spins forever.
#
# usage: nop-server.py <port>                
#

# For testing concurrency, client calls two servers: tiny and nop-server.
# If conncurency works, then even though the nop server is not responding, proxy will be able to receive and send tiny's response to the client

import socket
import sys

#create an INET, STREAMing socket
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(('', int(sys.argv[1])))
serversocket.listen(5)

# this server does not do anything, even the client requests from this server. 
# use infinite while loop and do not respond. 
while 1:
  channel, details = serversocket.accept()
  while 1:
    continue
