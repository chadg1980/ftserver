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

#msg sends the command to the server
msg = (command + " " + dataPort)

#control sock is the control connection
controlSock = createSocket(serverName, serverPort)
controlSock.send(msg)
serverInput = controlSock.recv(1024)
error = serverInput[:5]

if(error == 'error'):
	print serverInput;
	controlSock.close()
	sys.exit(0) 
print (serverInput)
controlSock.close()

#examples from https://docs.python.org/2/library/socket.html#example
# and https://docs.python.org/2.7/howto/sockets.html
commandSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
newDataPort = int(dataPort)

serverAddress = (serverName, newDataPort)
commandSock.bind(serverAddress)
commandSock.listen(1)
print "starting command socket on localhost:" + dataPort

dataIn,  addr = commandSock.accept()
file = open('newFile', 'w+')
while 1:
	
	print 'connected by ', addr
	data = dataIn.recv(1024)
	if data == 'break':
		print "break received"
		break;
	with open('newFile', 'w+') as f:
		f.write(data)
	fileOpen = open('newFile', 'r+')
	strToPrint = fileOpen.read()
	fileOpen.close()
	print str
	dataIn.sendall(data)
dataIn.close()




















