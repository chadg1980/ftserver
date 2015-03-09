/*
Chad Glaser
CS 372
Project 1
Chatserve.cpp

*/

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
//from Beej's guide section 5.1
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/fcntl.h>


using namespace std;

//function definitions
int tcp_this(const char*);
int tcp_data(const char*);
int listeningData(int, const char*);

/*This function makes sure the command line inputs meet the requirements*/
void validate(int arg_count){
/*Make sure the port number is included in the command line call*/
	if (arg_count !=2){
			cout << "Please take a look at readme.txt" << endl;
			exit(1);
	}

}

//exit function
void cleanup(int){
	cout << endl << "graceful exit " << endl;
	pid_t caughtPid;
	int status;
	caughtPid = waitpid(-1, &status, WNOHANG);
	exit(0);
	

}
//Function from Beej's guide to get the sending host
//I could not decifer all the parantheses and where to put * & on my own
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//Function to send the directory
//The dataport needs to be opened
void listDir (const char *dataPort,  char client[INET_ADDRSTRLEN]){
	int data_fd, next_fd, numByte;
	char buf[1024];
	char msg[1023];
	FILE *in;
	int len;
	char *buffer = (char*)malloc(1024);
	char *input = (char*)malloc(1024);
	char *hello_sent = (char*) malloc(1024);
	in = popen("ls", "r");
		
	data_fd = tcp_data(dataPort);
	next_fd = listeningData(data_fd, dataPort);
	
	numByte = recv(next_fd, msg, 1023, 0);
	
	while(fgets(buffer, sizeof(buffer), in) !=NULL){
		send(next_fd, buffer, (sizeof(buffer)+1), 0);
		memset(&buffer[0], 0, sizeof(buffer));
	}
	
	cout << "List directory requested on port " <<  dataPort << endl;
	cout << "Sending Directory contents to " << client << ":"<< dataPort << endl;
	
	close(data_fd);

}
//function to send the file
void fileSend(const char *fileName, const char *dataPort, int control_fd){
	
	int data_fd, next_fd; 
	FILE *local_fd;
	
	char *buf = (char*)malloc(1024);
	char *msg = (char*)malloc(1024);
	char connected[10] = "good\n";
		
	local_fd = fopen(fileName, "r");
	
	if (local_fd != NULL){
	send(control_fd, connected, (strlen(connected)+1), 0);
	
	data_fd = tcp_data(dataPort);
	next_fd = listeningData(data_fd, dataPort);
	
	
	
		while(fgets(buf, sizeof(buf), local_fd) !=NULL){
			send(next_fd, buf, (sizeof(buf)+1), 0);
			memset(&buf[0], 0, sizeof(buf));
			
		}
	sprintf(msg, "end");
	send(next_fd, msg, (sizeof(msg)+1), 0);
	memset(&msg[0], 0, sizeof(msg));
	close(data_fd);
	fclose(local_fd);
	
	}
	else{
	sprintf(msg, "File Not Found");
	send(control_fd, msg, (sizeof(msg)+1), 0);
	memset(&msg[0], 0, sizeof(msg));
	sprintf(msg, "found");
	send(control_fd, msg, (sizeof(msg)+1), 0);
	
	
	}
	

}
//Function to send an error message
void sendError(int p_fd){
	cout << "sending error" << endl;
	char errorMSG[34] = "error from SERVER SIDE";
	send(p_fd, errorMSG, (strlen(errorMSG)+1), 0);
	return;

}

//This function receives the instructions from the client 
//-l will go to the list function
//-g will go to the file transfer function
//Otherwise go to error function
void instructions(int control_fd, const char *serverPort, char client[INET_ADDRSTRLEN]){
	
	char *clientIn = (char*)malloc(1028);
	int bytRcv = 0;
	char *standOut = (char*) malloc(1028);
	char connected[10] = "good\n";
	char* tok;
	char* inputArr[1028];
	int i = 0;
	int k = 0;
	//Set all the array to all zeros
	memset(&clientIn[0], 0, sizeof(clientIn));
	
	bytRcv = recv(control_fd, clientIn, 1027, 0);
	if (bytRcv > 0){
		snprintf(standOut, bytRcv+1, "%s", clientIn);
	}
	else
		cout << "error\n";
	 
	
	//Close control connection
	//close(client_fd);
	
	//Tokenize function, because I am used to C, and the Beej's guide uses C
	//I am using the strtok function that I am used to
	tok = strtok(standOut, " ");
	while(tok){
		
		inputArr[i] = (char*) malloc(strlen(tok));
		strcpy(inputArr[i], tok);
		
		tok = strtok(NULL, " ");
		
		i++;
	}
	
	//-l send to the list dir fucntion
	if (strcmp(standOut, "-l")==0){
		send(control_fd, connected, (strlen(connected)+1), 0);
		close(control_fd);
		listDir(inputArr[1], client);
	}
	//-g sends to file send funtion
	else if (strcmp(standOut, "-g")==0){
		
		fileSend(inputArr[1], inputArr[2], control_fd);
	}
	//error function
	else{
		sendError(control_fd);
	}
	
}



//the tcp_this function sets up the struct for the socket
//Returns the socket file descriptor
int tcp_this( const char *this_port){
	struct addrinfo serv;
	struct addrinfo *tran;
	int status, bind_status;
	int socket_fd;
	int yes = 1;
	
	memset(&serv, 0, sizeof(serv));
	serv.ai_family = AF_UNSPEC;			//don't care IPv4 or IPv6
	serv.ai_socktype = SOCK_STREAM;		//TCP!
	serv.ai_flags = AI_PASSIVE;			//fill my IP in for me
	if ((status = getaddrinfo(NULL, this_port, &serv, &tran)) != 0){
		cout << "getaddrinfo error " << gai_strerror(status) << endl;
		exit(2);
	}
	//get a socket
	socket_fd = socket(tran->ai_family, tran->ai_socktype, tran->ai_protocol);
	fcntl(socket_fd, F_SETFL, O_ASYNC);
	
	if(socket_fd == -1){
		cout <<"socket_fd error\n";
		exit(3);
	}
	//setsockopt line of code is from Beej's guide
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		cout << "error setsockopt\n";
		exit(4);
	}
	//bind the socket
	bind_status = bind(socket_fd, tran->ai_addr, tran->ai_addrlen);
	if(bind_status == -1){
		close(socket_fd);
		cout <<"error in bind " << bind_status << endl;
	}
	cout << "Server open on port " << this_port << endl;
	freeaddrinfo(tran);
return(socket_fd);
	
}
int tcp_data( const char *this_port){
	struct addrinfo serv;
	struct addrinfo *tran;
	int status, bind_status;
	int socket_fd;
	int yes = 1;
	
	memset(&serv, 0, sizeof(serv));
	serv.ai_family = AF_UNSPEC;			//don't care IPv4 or IPv6
	serv.ai_socktype = SOCK_STREAM;		//TCP!
	serv.ai_flags = AI_PASSIVE;			//fill my IP in for me
	if ((status = getaddrinfo(NULL, this_port, &serv, &tran)) != 0){
		cout << "getaddrinfo error " << gai_strerror(status) << endl;
		exit(2);
	}
	//get a socket
	socket_fd = socket(tran->ai_family, tran->ai_socktype, tran->ai_protocol);
	fcntl(socket_fd, F_SETFL, O_ASYNC);
	
	if(socket_fd == -1){
		cout <<"socket_fd error\n";
		exit(3);
	}
	//setsockopt line of code is from Beej's guide
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		cout << "error setsockopt\n";
		exit(4);
	}
	//bind the socket
	bind_status = bind(socket_fd, tran->ai_addr, tran->ai_addrlen);
	if(bind_status == -1){
		close(socket_fd);
		cout <<"error in bind " << bind_status << endl;
	}
	cout << "Server open on port " << this_port << endl;
	freeaddrinfo(tran);
return(socket_fd);
	
}
	
//Function to listen, is in a loop in main until CTRL-C is pressed
// sends data to the appropriate place
void listening(int socket_fd, const char *this_port){
	
	int listen_status;
	struct sockaddr_storage them;
	socklen_t addr_size;
	int next_fd;
	char client[INET_ADDRSTRLEN];
	pid_t childProc = -5;
	pid_t waitForIt;
	int status;
	listen_status= listen(socket_fd, 10);
	if (listen_status == -1){
		cout << "listening error\n";
		exit(5);
	}
	cout << "listening on port " << this_port << endl;
	//accept a connection
	addr_size = sizeof(them);
	next_fd = accept(socket_fd, (sockaddr *) &them, &addr_size);
	if (next_fd < 0){
		cout << "Accept error\n";
	}
	
	//Function straight from Beej's guid
	//Prints to STDOUT where the connection is coming from.
	inet_ntop(them.ss_family, get_in_addr(( struct sockaddr *)&them), client, sizeof client);
	cout << "connection from " << client << endl;
	
			instructions(next_fd, this_port, client);
		
}

//SET up listening for the data port
int listeningData(int socket_fd, const char *this_port){
	
	int listen_status;
	struct sockaddr_storage them;
	socklen_t addr_size;
	int next_fd;
	char client[INET_ADDRSTRLEN];
		
	listen_status= listen(socket_fd, 10);
	if (listen_status == -1){
		cout << "listening Data error\n";
		exit(5);
	}
	cout << "listening on DataPort " << this_port << endl;
	//accept a connection
	addr_size = sizeof(them);
	next_fd = accept(socket_fd, (sockaddr *) &them, &addr_size);
	if (next_fd < 0){
		cout << "Data Accept error\n";
	}
	
	//Function straight from Beej's guid
	//Prints to STDOUT where the connection is coming from.
	inet_ntop(them.ss_family, get_in_addr(( struct sockaddr *)&them), client, sizeof client);
	
	cout << "Data Connection from " << client << endl;
	return next_fd;
	
	  
	
}


//The majority of this code is from Beej's Guide. I coded along with Beej's guide, 
//I would not have figured this out without Beej's guide. 
int main(int argc, char* argv[]){
	validate(argc);						//validate input before doing anything else
	const char *port = argv[1];
	int main_fd;
	int mySig = 1;
	pid_t anyPID = 0;		//for waitpid, to reap the dead children, runs each loop.
	int status;				//for waitpid
	//Signal Area. I took some of this code from my CS344 project
	//To stop the server use CTRL+C
	struct sigaction srvKll;
	srvKll.sa_handler = cleanup;
	srvKll.sa_flags = 0;
	sigfillset(&(srvKll.sa_mask));
	sigaction(SIGINT, &srvKll, NULL);
	//End signal area
	
	
	//Calling tcp_this to get the connection set up
	main_fd = tcp_this(port);
	
	while (mySig == 1){
		
		listening(main_fd, port);
	}
	
    return 0;
}






























