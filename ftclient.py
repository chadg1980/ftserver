"""
Chad H Glaser
CS 372
Winter 2015
Program 2
"""
import socket
import sys

RCVHOST = ''

#function to create a socket
def createSocket(sName, sPort):
	
	#create a socket
	thisSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	#connect to a server
	thisSock.connect((sName, sPort))
	return thisSock

def serverIn():
	srvInput = controlSock.recv(1024)
	print srvInput
	
#Main function starts here
if len(sys.argv) < 4:
	print "not enough arguments:"
else:
	serverName = sys.argv[1]
	serverPort = int(sys.argv[2])
	command = sys.argv[3]
	dataPort = sys.argv[4]

msg = (command + " " + dataPort)

	
controlSock = createSocket(serverName, serverPort)
controlSock.send(msg)
controlSock.close()

#examples from https://docs.python.org/2/library/socket.html#example
# and https://docs.python.org/2.7/howto/sockets.html
commandSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
newDataPort = int(dataPort)
commandSock.bind((RCVHOST, newDataPort))
commandSock.listen(1)

dataIn,  addr = commandSock.accept()
print 'connected by ', addr
while 1:
	
	data = dataIn.recv(1024)
	if not data: break
	dataIn.sendall(data)
dataIn.close()

















