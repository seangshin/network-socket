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
    char status401[] = "401 You are not currently logged in, login first.\n";
    char status402[] = "402 User not allowed to execute this command.\n";
    char status410[] = "410 Wrong UserID or Password\n";
    bool loginStatus = false;
    bool root = false;
    
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

        if (msgGetCount >= 5) {
          msgGetCount = 0; // Reset back to the first message
        }
        send (new_s, status200, strlen(status200), 0); //send status successful
        send (new_s, messages[msgGetCount], strlen(messages[msgGetCount]) + 1, 0);
        msgGetCount++;
      }

      //******************************   MSGSTORE *************************************** //
      if (strcmp(buf, "MSGSTORE\n") == 0) { 

        //check if logged in, if not send back 401, else ...
        if (!loginStatus) {
          send (new_s, status401, strlen(status401), 0);
        } else {
          send (new_s, status200, strlen(status200), 0); //send status successful
        }
      }

      //******************************   LOGIN *************************************** //
      if (strncmp(buf, "LOGIN", 5) == 0) { 

        //string manipulation to extract username and password from user input
        string input(buf);
        string username = input.substr(6, input.find(" ", 6) - 6);
        string password = input.substr(input.find(" ", input.find(" ") + 1) + 1);
        password.erase(password.find_last_not_of("\n") + 1); // remove newline character (\n)

        for (int i = 0; i < 4; i++) {
          string tempUser = users[i].username;
          string tempPassword = users[i].password;
          
          if (username == tempUser && password == tempPassword) {
            if(i == 0) {
              root = true;
            }
            loginStatus = true;
            send (new_s, status200, strlen(status200), 0); //send status successful   
            break; // Exit the loop since we found a match
          }
        }

        if (!loginStatus) {
          send (new_s, status410, strlen(status410), 0); //send status successful
        }   
      }

      //******************************   LOGOUT *************************************** //
      if (strcmp(buf, "LOGOUT\n") == 0) { 

        if (!loginStatus) {
          send (new_s, status401, strlen(status401), 0);
        } else {
          loginStatus = false;
          send (new_s, status200, strlen(status200), 0); //send status successful  
        }
      }

      //******************************   SHUTDOWN *************************************** //
      if (strcmp(buf, "SHUTDOWN\n") == 0) {
        if(!root) {
          send (new_s, status402, strlen(status402), 0);
        } else {
          send (new_s, status200, strlen(status200), 0); //send status successful
          //close(new_s);
          break;
        }

      }

			cout << buf;
			send (new_s, buf, strlen(buf) + 1, 0);
		}

		close(new_s);
    }
} 
 
