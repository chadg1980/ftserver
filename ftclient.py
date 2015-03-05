"""
Chad H Glaser
CS 372
Winter 2015
Program 2
"""
import socket
import sys


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

controlSock = createSocket(serverName, dataPort)
controlSock.send(command)
serverIn()
controlSock.close()