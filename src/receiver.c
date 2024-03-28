#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>
#include <signal.h> // Added this temporarily until we do acknowledgements.


int sockfd; // Global socket file descriptor
FILE *file = NULL; // Global file pointer

// Signal handler for SIGINT
void sigintHandler(int sig_num) {
    // Close the socket
    if (sockfd >= 0) {
        close(sockfd);
        printf("\nSocket closed.\n");
    }

    // Close the file if it's open
    if (file) {
        fclose(file);
        printf("File closed and data saved.\n");
    }

    // Exit the program
    exit(0);
}

void bindSocket(unsigned short int myUDPport) {
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    recvAddr.sin_port = htons(myUDPport);
    
    if (bind(sockfd, (const struct sockaddr *)&recvAddr, sizeof(recvAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void receiveMessages(char* destinationFile, unsigned long long int writeRate) {
    file = fopen(destinationFile, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    char buffer[1024]; // Adjust buffer size as needed
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    ssize_t receivedBytes;

    while (1) {
        receivedBytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (receivedBytes < 0) {
            perror("recvfrom failed");
            break; // Exit or handle error appropriately
        }

        // Write received data to the file
        fwrite(buffer, 1, receivedBytes, file);
    }

    fclose(file); // Close the file
}

// New rrecv function encapsulating the setup and receive process
void rrecv(unsigned short int myUDPport, char* destinationFile, unsigned long long int writeRate) {
    signal(SIGINT, sigintHandler); // Register the signal handler

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        exit(EXIT_FAILURE);
    }

    bindSocket(myUDPport);
    receiveMessages(destinationFile, writeRate); // Now directly uses the destinationFile and writeRate

    close(sockfd); // Close the socket
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s UDP_port filename_to_write\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned short int udpPort = atoi(argv[1]);
    char* destinationFile = argv[2];

    // Call the rrecv function with the provided command line arguments
    rrecv(udpPort, destinationFile, 0); // Assuming writeRate is not utilized in this implementation

    return 0;
}