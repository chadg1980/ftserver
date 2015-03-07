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
void listDir (const char *dataPort, char s[INET_ADDRSTRLEN]){
	int socket_fd, numByte;
	char buf[1024];
	struct addrinfo sockets, *client, *q;
	int rv, connRV;
	char t[INET_ADDRSTRLEN];
	char MSG[13] = "break";
	FILE *in;
	char *buffer = (char*)malloc(1024);
		
	cout << "starting listDir "<< endl;
	
	memset (&sockets, 0, sizeof(sockets));
	sockets.ai_family = AF_UNSPEC;
	sockets.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo("localhost", dataPort, &sockets, &client)) != 0){
		cout << "Error in getarrdinfo \n\n\n" << gai_strerror(rv) << endl;
	}
	
	for (q = client; q != NULL; q = q->ai_next){
		socket_fd = socket(q->ai_family, q->ai_socktype, q->ai_protocol);
		
		if (socket_fd == -1){
			cout << "\n\nsocket_fd = " << socket_fd << endl << endl;
			cout << "error in list dir socket_fd\n";
			exit(EXIT_FAILURE);
		} 
		
		connRV = connect(socket_fd, q->ai_addr, q->ai_addrlen);
		if(connRV == -1){
			close(socket_fd);
			perror("listdir: connect");
			continue;
		}
		break;
	
	}
	if(q == NULL){
		cout << "connection failed\n";
	}
	
	//inet_ntop(q->ai_family, get_in_addr((struct sockaddr *)q->ai_addr), q, sizeof q);
	//cout << "connected to: " << q << endl;
	in = popen("ls", "r");
	strtok(in, "\n");
	while(fgets(buffer, sizeof(buffer), in) !=NULL){
		send(socket_fd, buffer, (sizeof(buffer)+1), 0);
		cout << buffer << endl;
		memset(&buffer[0], 0, sizeof(buffer));
	}
	
	cout << "List directory requested \n on port " <<  dataPort << endl;
	cout << "Sending Directory \n contents to " <<  dataPort << endl;
	
	send(socket_fd, MSG, (sizeof(MSG)+1), 0);
	close(socket_fd);

}
//function to send the file--NOT COMPLETE
void fileSend(const char *dataPort){
cout << "File Send Function "<< endl; 
exit(EXIT_SUCCESS);

/*
int total //how many bytes sent
int left = *len // how many are left to send
int n

while total < *len 
	n = send(msg, buf+total, bytesleft, 0)
	total +=n
	bytesleft -=n;
	
	*len = total; 
	
	retrun -1 for success, 0 for failure
*/

}
//Function to send an error message
void sendError(int p_fd){
	cout << "sending error" << endl;
	char errorMSG[34] = "error You've broken the internet!";
	send(p_fd, errorMSG, strlen(errorMSG), 0);
	exit(EXIT_SUCCESS);

}

//This function receives the instructions from the client 
//-l will go to the list function
//-g will go to the file transfer function
//Otherwise go to error function
void instructions(int control_fd, const char *serverPort, char s[INET_ADDRSTRLEN]){
	cout << "starting instructions" << endl;
	char *clientIn = (char*)malloc(1028);
	int bytRcv = 0;
	char *standOut = (char*) malloc(1028);
	char connected[10] = "Connected";
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
	 
	send(control_fd, connected, (strlen(connected)+1), 0);
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
		close(control_fd);
		listDir(inputArr[1], s);
	}
	//-g sends to file send funtion
	else if (strcmp(standOut, "-g")==0){
		close(control_fd);
		fileSend(inputArr[1]);
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
	
//Function to listen, is in a loop in main until CTRL-C is pressed
// sends data to the appropriate place
void listening(int socket_fd, const char *this_port){
	
	int listen_status;
	struct sockaddr_storage them;
	socklen_t addr_size;
	int next_fd;
	char s[INET_ADDRSTRLEN];
	pid_t childProc = -5;
	pid_t waitForIt;
	int status;
	listen_status= listen(socket_fd, 10);
	if (listen_status == -1){
		cout << "listening error\n";
		exit(5);
	}
	
	//accept a connection
	addr_size = sizeof(them);
	next_fd = accept(socket_fd, (sockaddr *) &them, &addr_size);
	if (next_fd < 0){
		cout << "Accept error\n";
	}
	
	//Function straight from Beej's guid
	//Prints to STDOUT where the connection is coming from.
	inet_ntop(them.ss_family, get_in_addr(( struct sockaddr *)&them), s, sizeof s);
	cout << "connection from " << s << endl;
	
			instructions(next_fd, this_port, s);
		
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






























