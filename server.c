/*
 * server.c
 */

#include <stdio.h>
#include <iostream>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <cstring>
#include <cstdlib>
#include <string>

using namespace std;

#define SERVER_PORT 2477
#define MAX_PENDING 5
#define MAX_LINE 256

//define structure for users
struct Users {
  char username[20];
  char password[20];
};

int main(int argc, char **argv) {

    struct sockaddr_in sin;
    socklen_t addrlen;
    char buf[MAX_LINE];
    int len;
    int s;
    int new_s;

    // *********************** Initialization Steps ******************************** //
    char status200[] = "200 OK\n";
    char status300[] = "300 message format error\n";
    char status401[] = "401 You are not currently logged in, login first.\n";
    char status402[] = "402 User not allowed to execute this command.\n";
    char status410[] = "410 Wrong UserID or Password\n";
    char status411[] = "411 MSGSTORE Full.\n";
    bool loginStatus = false;
    bool root = false;
    char newMessageBuffer[MAX_LINE]; // Buffer used to store char input for MSGSTORE
    int messageLen; // Store number of bytes from recv
    
    /* Message of the day - hardcode */
    char *messages[20];
    for(int i = 0; i < 20; i++) {
        messages[i] = (char *)malloc(50 * sizeof(char)); // Allocate memory for each string
    }
    strcpy(messages[0], "Message 1\n");
    strcpy(messages[1], "Message 2\n");
    strcpy(messages[2], "Message 3\n");
    strcpy(messages[3], "Message 4\n");
    strcpy(messages[4], "Message 5\n");
    int msgGetCount = 0; 
    int nextAvailableSlot = 5;

    /* Users - hardcode */
    struct Users users[4];
    strcpy(users[0].username, "root");
    strcpy(users[0].password, "root01");
    strcpy(users[1].username, "john");
    strcpy(users[1].password, "john01");
    strcpy(users[2].username, "david");
    strcpy(users[2].password, "david01");
    strcpy(users[3].username, "mary");
    strcpy(users[3].password, "mary01");

    // *************************************************************************** //

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons (SERVER_PORT);

    /* setup passive open */
    if (( s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
    }
    int option = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // allows to reuse of address to avoid bind error

    if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("bind");
		exit(1);
    }

    listen (s, MAX_PENDING);

    addrlen = sizeof(sin);
    cout << "The server is up, waiting for connection.." << endl;

    /* wait for connection, then receive and print text */
    while (1) {
		if ((new_s = accept(s, (struct sockaddr *)&sin, &addrlen)) < 0) {
			perror("accept");
			exit(1);
		}
		cout << "new connection from " << inet_ntoa(sin.sin_addr) << endl;
	
		while (len = recv(new_s, buf, sizeof(buf), 0)) {

      buf[len] = '\0';
      
      //******************************   MSGGET *************************************** //
      if (strcmp(buf, "MSGGET\n") == 0) { //MSGGET

        if (msgGetCount >= nextAvailableSlot) {
          msgGetCount = 0; // Reset back to the first message
        }
        send (new_s, status200, strlen(status200), 0); //send status successful
        send (new_s, messages[msgGetCount], strlen(messages[msgGetCount]) + 1, 0);
        msgGetCount++;
      }

      //******************************   MSGSTORE *************************************** //
      else if (strcmp(buf, "MSGSTORE\n") == 0) { 

        //check if logged in, if not send back 401, else ...
        if (!loginStatus) {
          send (new_s, status401, strlen(status401), 0);
        } else if (nextAvailableSlot == 20) {
          send (new_s, status411, strlen(status411), 0); // if MSGSTORE storage is full, send error
        } else {
          send (new_s, status200, strlen(status200), 0); //send status successful
          
          memset(newMessageBuffer, 0, sizeof(newMessageBuffer)); // reset the array
          
          messageLen = recv(new_s, newMessageBuffer, sizeof(newMessageBuffer), 0); // receive data from client
          newMessageBuffer[messageLen] = '\0';
          strcpy(messages[nextAvailableSlot], newMessageBuffer); // store client message into next available slot
          nextAvailableSlot++; // move to next available slot
          send (new_s, status200, strlen(status200), 0); //send status successful
        }
      }

      //******************************   LOGIN *************************************** //
      else if (strncmp(buf, "LOGIN", 5) == 0) { 

        //string manipulation to extract username and password from user input
        string input(buf);
        string username = input.substr(6, input.find(" ", 6) - 6);
        string password = input.substr(input.find(" ", input.find(" ") + 1) + 1);
        password.erase(password.find_last_not_of("\n") + 1); // remove newline character (\n)

        //create string datatypes and hold char for later calculation
        for (int i = 0; i < 4; i++) {
          string tempUser = users[i].username;
          string tempPassword = users[i].password;
          
          //check if username and password matches idatabase
          if (username == tempUser && password == tempPassword) {
            //check if root user/pass
            if(i == 0) {
              root = true;
            }
            loginStatus = true; // set login status to true for remaining session
            send (new_s, status200, strlen(status200), 0); //send status successful   
            break; // Exit the loop since we found a match
          }
        }

        if (!loginStatus) {
          send (new_s, status410, strlen(status410), 0); //send error
        }   
      }

      //******************************   LOGOUT *************************************** //
      else if (strcmp(buf, "LOGOUT\n") == 0) { 

        //check loginStatus to ensure user is logged in, if true then logout user and if not send error
        if (!loginStatus) {
          send (new_s, status401, strlen(status401), 0);
        } else {
          loginStatus = false;
          root = false;
          send (new_s, status200, strlen(status200), 0); //send status successful  
        }
      }

      //******************************   QUIT *************************************** //
      else if (strcmp(buf, "QUIT\n") == 0) {

        //reset root and loginStatus to false, then send successful message
        root = false;
        loginStatus = false;
        send (new_s, status200, strlen(status200), 0); //send status successful

      }

      //******************************   SHUTDOWN *************************************** //
      else if (strcmp(buf, "SHUTDOWN\n") == 0) {
        //check if root user is logged in, if so then send successful status and close/terminate server
        if(!root) {
          send (new_s, status402, strlen(status402), 0);
        } else if (root){
          send (new_s, status200, strlen(status200), 0); //send status successful
          close(new_s);
          exit(1);
        } else {
          send (new_s, status300, strlen(status300), 0); //send status successful
        }
        //reset root and loginStatus to false
        root = false;
        loginStatus = false;
      }

      else {
        send (new_s, buf, strlen(buf) + 1, 0); // unexpected responses will echo client
      }

			cout << buf;
			
		}

		close(new_s);
    }
} 
 
