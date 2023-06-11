#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

/*
Author: ADRIAN HARISCHAND
*/


int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];


int handle_stdin( char*buf, int size) {
    char ch;
    int counter = 0;
    while ( (ch = getc(stdin)) != '\n' ) {
        if(errno == EINTR){
            printf("\n");
            break;
        }
        else if (ch == EOF){
           printf("\n");
           exit(EXIT_SUCCESS);
          
    }
     if(counter < size-1){
        buf[counter++] = ch;
    }
        else{counter++;}

    }
    
  if(counter > size-1){
    memset(buf,0 ,size);
  }
  buf[counter+1] = '\0';
  fflush(stdin);
  return counter;
}



int handle_client_socket() {
       
      
    int i =0,  bytes_recvd = recv(client_socket, inbuf,  1, 0);
    if (bytes_recvd == 0){
         fprintf(stderr, "\nConnection to server has been lost.\n");
        return 0;    
}
 while( inbuf[i] != '\0'){
      i++;
      if(i > MAX_MSG_LEN){
      break ;    
}
bytes_recvd = recv(client_socket, inbuf+i, 1, 0);

}

  //  printf("Bytes received : %d\n", bytes_recvd);
    if (bytes_recvd == -1 && errno != EINTR) {
        fprintf(stderr,  "Warning: Failed to receive incoming message.");
        return 0;
    } 
        else {
        // Process the incoming data. If "bye", the client disconnected.
      //  inbuf[bytes_recvd] = '\0';
        if (strcmp(inbuf, "bye") == 0) {
            printf("\nServer initiated shutdown.\n");
            fflush(stdout);
            return 2;
        } else {
           
            printf("\n%s\n", inbuf);
           
          
            fflush(stdout);
            return 1;
        }

        
    } 
   
}

int main(int argc, char **argv) {
        // checks for proper command-line arguments
    if (argc != 3){
        fprintf(stderr,"Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int ip_conversion, port, bytes_recvd; // client_socket,
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    memset(&server_addr, 0 , addrlen); // Zero out structure
    
    /* Checking validity of the IP addy */
    ip_conversion = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (ip_conversion == 0){
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }
    else if (ip_conversion < 0){
        fprintf(stderr, "Error: Failed to convert IP address. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    
    /* Checking the validity of the given port number */
    if (!parse_int(argv[2], &port,"port number")){
        return EXIT_FAILURE;
    }
    if (port < 1024 || port > 65535) {
        fprintf(stderr, "Error: port must be in range [1024, 65535].\n");
        return EXIT_FAILURE;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // getting user name
    if (isatty(STDIN_FILENO)){
    printf("Enter your username: ");
    fflush(stdout);
}
    int  c = handle_stdin(username, MAX_NAME_LEN);
    if(c == 0){
        printf("Enter your username: ");
        fflush(stdout);
    }
    if (c > MAX_NAME_LEN-1){
        printf("Sorry, limit your username to 20 characters.\n");
        printf("Enter your username: ");
        fflush(stdout);   
 }
    while ( c == 0 || c > MAX_NAME_LEN-1 ){
        c = handle_stdin(username , MAX_NAME_LEN);
        if (c > 0 && c < MAX_NAME_LEN-1){
            break;
        }
        else{
            if(c == 0){
                printf("Enter your username: ");
                fflush(stdout);
            }
            if (c > MAX_NAME_LEN-1){
                printf("Sorry, limit your username to 20 characters.\n");
                printf("Enter your username: ");
                fflush(stdout);          
 }
        }
    }
    printf("Hello, %s. Let's try to connect to the server.\n", username);
    
    
    // Create a reliable, stream socket using TCP.
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: Failed to create socket. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    
    // Establish the connection to the  server.
    if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
        fprintf(stderr, "Error: Failed to connect to server. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    
    // receiving from the server
    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0) {
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    else if (bytes_recvd == 0){
        fprintf(stderr, "Error: Failed to connect to server. Connection refused.\n");
        return EXIT_FAILURE;
    }
    else{
        inbuf[bytes_recvd] = '\0';
        printf("\n%s\n\n", inbuf);
       
}
    if (send(client_socket, username, strlen(username)+1, 0) < 0) {
        fprintf(stderr, "Error: Failed to send message to server. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;

    } 
   // fflush(client_socket);
/* --------------------------------------------------------------------------------*/   

    fd_set sockset;
    int max_socket, retval = EXIT_SUCCESS;;
    while(1){
       
//      printf("At top of while loop\n");
        if (isatty(STDIN_FILENO)){
        fprintf(stdout, "[%s]: ", username);
        fflush(stdout);
        }   
      
        FD_ZERO(&sockset);
        FD_SET(STDIN_FILENO, &sockset);
        max_socket = STDIN_FILENO;
        
        if (client_socket > -1){
            FD_SET(client_socket, &sockset);
        }
        if (client_socket > max_socket){
            max_socket = client_socket;
        }
        
        // Wait for activity on one of the sockets.
        // Timeout is NULL, so wait indefinitely.
        if (select(max_socket + 1, &sockset, NULL, NULL, NULL) < 0
                && errno != EINTR) {
            fprintf(stderr, "Error: select() failed. %s.\n", strerror(errno));
            retval = EXIT_FAILURE;
            break;
        }
        

        // check if there is activity on STDIN_FILENO
         if( FD_ISSET(STDIN_FILENO, &sockset)){
           
       //    if (isatty(STDIN_FILENO)){
           int  ch = handle_stdin(outbuf, MAX_MSG_LEN);
            if (ch > MAX_MSG_LEN-1){
                printf("Sorry, limit your message to 1 line of at most %d characters.\n",
                         MAX_MSG_LEN );
                fflush(stdout);   
               }
           else{
        
            if (send(client_socket, outbuf, strlen(outbuf)+1, 0) < 0) {
                fprintf(stderr, "Error: Failed to send message to server. %s.\n",
                        strerror(errno));
                retval =  EXIT_FAILURE;
                break;
                }
       //     fflush(client_socket);
            if (strcmp(outbuf, "bye") == 0){
                printf("Goodbye.\n");
                fflush(stdout);
                retval= EXIT_SUCCESS;
                break;
                }
          } 
          memset(outbuf,0, MAX_MSG_LEN+1);
        
     
      }
 
    // checking for activity on the client server 
    if(FD_ISSET(client_socket, &sockset)){
      
      int d = handle_client_socket();
      if(d == 0){
         retval = EXIT_FAILURE; 
         break;    
         }
 
     else if (d == 2){
        retval = EXIT_SUCCESS;
        break;
        }

   }

 }
  
 if (fcntl(client_socket, F_GETFD) != -1) {
        close(client_socket);
 }
   

 if (fcntl(STDIN_FILENO, F_GETFD) != -1) {
            close(STDIN_FILENO);
 }
 
    return retval;
}
