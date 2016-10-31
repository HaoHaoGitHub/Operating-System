/* Name: Hao Chen / Email: chenh15@rpi.edu */
/* Compile: gcc -o main main.c 
   Another terminal window: telnet 127.0.0.1 listener_port_number */

#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>

# define BUFFER_SIZE 1024
# define listener_port 9753

int main() {

    /* Create hidden directory on server */
    struct stat st = {0};
    
    if (stat( "./.storage", &st) == -1) mkdir( "./.storage", 0700 );
    else system( "exec rm -r ./.storage/*" );


    /* Create the listener socket as TCP socket */
    int sd = socket( PF_INET, SOCK_STREAM, 0 );
    
    if ( sd < 0 ) {
        perror( "socket() failed" );
        exit( EXIT_FAILURE );
    }

    /* socket structures */  
    struct sockaddr_in server;
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( listener_port );
    int len = sizeof( server );
    
    if ( bind( sd, (struct sockaddr *)&server, len ) < 0 ) {
        perror( "bind() failed" );
        exit( EXIT_FAILURE );
    }

    listen( sd, 5 );   /* 5 is the max number of waiting clients */
    printf( "Listening on port %d\n", listener_port );

    struct sockaddr_in client;
    int fromlen = sizeof( client );
    int pid;
    char buffer[ BUFFER_SIZE ];

    while (1) {
        
        int newsock = accept (sd, (struct sockaddr *)&client, (socklen_t*)&fromlen );
        printf( "Received incoming connection from %s\n", inet_ntoa( (struct in_addr)client.sin_addr ) );
        
        pid = fork();

        if ( pid < 0 ) {
            perror( "fork() failed" );
            exit( EXIT_FAILURE );
        } else if ( pid == 0 ) {
            int n;
            
            do { /* blocking on recv() */
                
                n = recv( newsock, buffer, BUFFER_SIZE, 0 );

                if ( n < 0 ) { 
                    perror( "recv() failed" );
                } else if ( n == 0) {
                      printf( "CHILD %d: Rcvd 0 from recv(); closing socket\n", getpid() );
                } else { 

                    buffer[n] = '\0';                /* assuming text ..... */
                    char *token;
                    token = strtok(buffer, "\r\n");  /* The first line of the client's command, break by \n */
                    printf( "[CHILD %d] Rcvd: %s\n", getpid(), token );

                    /* Start analyzing the buffer */

                    FILE *file;
                    char instruction[20];            /* An array holds the instruction: STORE, READ, or DELETE */ 
                    char filename[80];               /* An array holds the file name */
                    char *hold;                      /* A temp variable used for analyze the command array */
                    hold = strtok( buffer, " " ); 
                    char **command_array;            /* Command array */
                    command_array = malloc( 4 * sizeof(char*) );
                    char* message;                       /* A message variable */
                    
                    int count = 0;   
                    /* Filling up the command array by strtok function */
                    int flag = 0;
                    while ( hold != NULL ) {
                        command_array[count] = malloc( (strlen(hold)+1) * sizeof(char) );
                        if (count == 2 || count == 3) {
                            int i;
                            for (i = 0; i < strlen(hold); ++i) {
                                if (!isdigit(hold[i])) {
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                        strcpy( command_array[count++], hold );
                        hold = strtok( NULL, " ");
                    }
                    if (flag) {
                        fprintf(stderr, "ERROR: INVALID COMMAND\n");
                        continue;
                    }

                    int file_exists = 1;              /* A variable that keeps track of the the existing status of the file */
                    int read_success = 0;             /* A variable that keeps track of the reading status in terms of the ranges */
                    /* Get the instruction from the command */
                    strcpy( instruction, command_array[0] ); 
                    if (strcmp(instruction, "STORE") == 0) {
                        if (count != 3) {
                            fprintf(stderr, "ERROR: INVALID COMMAND\n");
                            continue;
                        }
                    } else if (strcmp(instruction, "READ") == 0) {
                        if (count != 4) {
                            fprintf(stderr, "ERROR: INVALID COMMAND\n");
                            continue;
                        }
                    } else {
                        if (count != 2) {
                            fprintf(stderr, "ERROR: INVALID COMMAND\n");
                            continue;
                        }
                    }

                    /* check the first string of the comand */
                    if (strcmp(instruction, "STORE") == 0 || strcmp(instruction, "READ") || strcmp(instruction, "DELETE")) {
                        strcpy( filename, ".storage/" );
                        strcat( filename, command_array[1] ); 

                        file = fopen( filename, "rb" );
                        if (file == NULL) file_exists = 0;
                    }

                    /* Analyzing the each instruction separtely: */
                    /* --------------------------------------------------------------------------------------------------- */

                    if ( strcmp( instruction, "STORE" ) == 0 ) {

                        /* If file already exists: output error message */                         
                        if ( file_exists == 1 ) {

                            message = malloc( 30 * sizeof(char) );
                            strcpy( message, "ERROR: FILE EXISTS\n" );

                        } else { 
                            /*  else, just write file to server */ 
                            int num_of_bytes = atoi( command_array[2] );                      
                            char file_array[num_of_bytes + 1];
                            /* ------------------------------------------------------- */
                            int ret = recv( newsock, file_array, num_of_bytes + 2, 0 );
                            /* ------------------------------------------------------- */

                            if (ret <= 0) {
                                message = malloc( 30 * sizeof(char) );
                                strcpy( message, "ERROR: rcvd() failed\n" );
                            } else {
                                file_array[num_of_bytes] = '\0';
                                
                                /* write file and sent ACK back  */
                                file = fopen( filename, "wb" );
                                fwrite( file_array, sizeof(file_array), 1, file );
                                fclose( file );
                                message = malloc( 4 * sizeof(char) );
                                strcpy( message, "ACK\n" );

                                printf( "[CHILD %d] Stored file '%s' (%d bytes)\n", getpid(), command_array[1], num_of_bytes);                            
                            } 
                        }
                    } 

                    /* --------------------------------------------------------------------------------------------------- */
                    
                    else if ( strcmp( instruction, "READ" ) == 0 ) { 

                        if ( file_exists == 0 ) {
                            /* If the file not exists: */
                            message = malloc( 30 * sizeof(char) );
                            strcpy( message, "ERROR: NO SUCH FILE\n" );
                        } else {

                            int byte_offset = atoi( command_array[2] ); /* From which point to start */
                            int length = atoi( command_array[3] );      /* The length to read */
                            /* check if byte range is valid */
                            fseek( file, 0, SEEK_END );
                            long filesize = ftell( file );
                            rewind ( file ); 
                            
                            int total_length = byte_offset + length;
                            if ( total_length > filesize - 1 ) {
                                /* if the file byte range is invalid, return error message */
                                message = malloc( 40 * sizeof(char) );
                                strcpy( message, "ERROR: INVALID BYTE RANGE\n" );

                            } else { 

                                fseek( file, byte_offset, SEEK_SET );
                                char read_array[length + 1];
                                
                                fread(read_array, sizeof(read_array), 1, file );
                                read_array[length] = '\0';

                                message = malloc( (length + 30) * sizeof(char) );
                                strcpy( message, "ACK " );
                                strcat( message, command_array[3] );
                                strcat( message, "\n" );
                                strcat( message, read_array);
                                strcat( message, "\n" );

                                read_success = 1;
                            }
                        } 
                    } 
                    
                    /* --------------------------------------------------------------------------------------------------- */
   
                    else if ( strcmp( instruction, "DELETE" ) == 0 ) { 

                        /* if file not exists */
                        if ( file_exists  == 0) {
                            message = malloc( 30 * sizeof(char) );
                            strcpy( message, "ERROR: NO SUCH FILE\n" );
                        } else { 
                            int ret;
                            ret = remove( filename );
                            if (ret != 0) {
                                message = malloc( 30 * sizeof(char) );
                                strcpy( message, "ERROR: delete() error\n" );                              
                            } else {
                                message = malloc( 4 * sizeof(char) );
                                strcpy( message, "ACK\n" );
                                printf( "[CHILD %d] Deleted '%s' from server\n", getpid(), command_array[1]);
                            } 
                        }

                    } else if ( strcmp( instruction, "DIR" ) != 0 ){ 
                        /* invalid command input */
                        message = malloc( 30 * sizeof(char) );
                        strcpy( message, "ERROR: INVALID COMMAND\n" );          
                    }

                    /* send message back to client  */
                    /* ---------------------------------------------- */
                    n = send( newsock, message, strlen(message), 0 );
                    /* ---------------------------------------------- */
                    fflush( NULL );
                    if ( n != strlen(message) ) perror( "ERROR: send() failed" );

                    if (read_success != 1 ) {
                        printf( "[CHILD %d] Sent: %s", getpid(), message ); 
                    } else {
                        /* output read disk memory results  */
                        printf( "[CHILD %d] Sent: ACK %d\n", getpid(), atoi(command_array[3]) );
                        int temp1 = atoi(command_array[3]);
                        int temp2 = atoi(command_array[2]);
                        printf( "[CHILD %d] Sent %d bytes of '%s' from offset %d\n",
                            getpid(), temp1, command_array[1], temp2 );
                    }

                    /* free all dynamically allocated memory */
                    /* ---------------------------------------------- */
                    int i = 0;
                    while ( i < count ) free(command_array[i++]);
                    free( command_array );
                    free( message );
                    /* ---------------------------------------------- */
                }

            } while ( n > 0 ); /* end of do while loop */
            
            printf( "[CHILD %d] Client closed its socket....terminating\n", getpid() );
            close( newsock );     
            exit( EXIT_SUCCESS );   /* child terminates */

        } else { 

            /* IN THE PARENT PROCESS */
            /* ---------------------------------------- */
            wait( NULL );
            /* ---------------------------------------- */
            close( newsock );
        }
    }

    close(sd);
    return EXIT_SUCCESS;
}


