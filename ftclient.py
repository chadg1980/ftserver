"""
Chad H Glaser
CS 372
Winter 2015
Program 2
"""
import socket
import sys
import time

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
if command == '-l':
	dataPort = sys.argv[4]
	#msg sends the command to the server
	msg = (command + " " + dataPort)
elif command == '-g':
	fileName = sys.argv[4]
	dataPort = sys.argv[5]
	#msg sends the command to the server
	msg = (command + " " + fileName + " " + dataPort)
else:
	print command + " not recongnized"
	sys.exit(0)




hello = "hello world from the client"
#control sock is the control connection
print 'client starts command port'
controlSock = createSocket(serverName, serverPort)
controlSock.send(msg)
serverInput = controlSock.recv(1024)
error = serverInput[:5]
good = serverInput [:4]

if error == 'error':
	print serverInput;
	controlSock.close()
	sys.exit(1) 
elif good == 'good':
	controlSock.close()
	time.sleep(1)
	newDataPort = int(dataPort)
	dataSock = createSocket(serverName, newDataPort)
	dataSock.send(hello)
	if command == '-l':
		print 'Receiving Directory from ' + serverName +":"+ dataPort
		newData = dataSock.recv(1024)
		print (newData)
	elif command == '-g':
		print "Receiving " + fileName + " from " + serverName + ":"+dataPort
		file = open(fileName, 'w+')
		data = dataSock.recv(1024)
		with open(fileName, 'w+') as f:
			f.write(data)
		data = dataSock.recv(1024)
		print "file transfer done."
		dataSock.close()
else:
	print(serverInput)
	print 'something went wrong'
	controlSock.close()
sys.exit(0)





















