Chad H Glaser
Readme.txt

SERVER: ftserver.cpp

to compile on flip1 use the command: 
	g++ -o ftserver ftserver.cpp
to run:
	./ftserver [PORT NUMBER]
		example:
			./ftserver 10101

CLIENT:ftclient.py
to run: The commands -l for list directory, and -g for file transfer
	List the Directory: -l
		python ftclient.py [server name] [server port] -l [data port]
	File transfer: -g
		python ftclient.py [server name] [server port] -g [file name] [data port]

This has only been tested on flip1, using localhost as the server name.
[server port] for the cleint has to be the exact same as the [PORT NUMBER] for the server

I used examples from Beej's Guide, I may have missed a sighting a specific line of code that I mentioned them. 
I am not trying to plagerize any code in any way. But I used Beej's guide as an example, I did not copy and 
paste any code. 
	