# Network Sockets

## Description 
The motivation of this project is to desgin a socket interface for a client-server application. The client and server processes communicate using TCP sockets. It will implment the yamotd protocol to return and store messages from the client. 

## Table of Contents
- [Function](#Function)
- [Instructions](#Instructions)
- [Known bugs](#Known bugs)
- [Sample outputs](#Sample outputs)
Software tools required
Putty - SSH Client
VS Code - Code editor
WinSCP - File Transfer
Global Protect - VPN

## Function

Server.c
1. Initialization steps
Hardcode users. Define status messages. Define login status and root user flags. Define message buffer to hold message input from client. Hardcode 5 "message of the day" and counter. 
2. MSGGET
When the client sends the "MSGGET" command, the server will return a message stored in the messages array along with a status 200. The function keeps track of the message sent by using a global counter that will increment after each call. Once the last message is reached in the stack, the counter will reset to 0. 
3. MSGSTORE
When the client sends the "MSGSTORE" command, the server will check if the user is logged in and return a status 200 unless the user is not logged in which it will return an error status. It will then call the recv function which is a blocking system call. It will wait for the client to send a message and then store it into the messages array. The message array has a maximum capacity of 20 messages, once exceeded it will return an error. 
4. LOGIN
When the client sends the "LOGIN user pass" command, the server will decompose the user input and store the username and password into a string data type for comparison. It will then loop through the hardcoded users and password to check if the input matches. If the user is matched, then a login status flag is set. If the user logs in with the root user/pass, then a flag will be set that the user is the root user. The server will send a status 200 if successful. Otherwise an error status is returned.
5. LOGOUT
When the client sends the "LOGOUT" command, the server will check if the user is logged in. If so, it will reset all flags and send a status 200. If not, it will send an error status.
6. QUIT
When the client sends the "QUIT" command, the server will reset all flags and send a status 200.
7. SHUTDOWN
When the client sends the "SHUTDOWN" command, the server will check if the root user is logged in. If so, then it will send a status 200 and close the server socket and exit the program. If not the root user, it will send an error status. All flags are reset.

## Instructions
1. Open Putty app (or any other SSH client) and connect to login to the following server: login.umd.umich.edu
2. Navigate to the root directory where the source code is located and execute the Makefile using the `make` command 
3. Run the server using the following command: `./server`
4. Open another window of the Putty app and navigate to the root directory, then run the client using the following command (note: the IP address provided is a local host address for testing): `./client 127.0.0.1`
5. In the client window, send any of the following commands: MSGGET, MSGSTORE, LOGIN, LOGOUT, QUIT, SHUTDOWN.

## Known bugs

## Sample outputs
1. C: MSGGET, S: 200 OK, S: Message 1
2. C: LOGIN john john01, S: 200 OK
3. C: MSGSTORE, S: 200 OK, C: New message, S: 200 OK (while logged in)