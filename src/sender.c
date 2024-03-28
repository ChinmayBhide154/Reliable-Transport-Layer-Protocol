#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_PAYLOAD_SIZE 16000

struct RTPHeader {
    uint32_t pktNumber;
    int ack;
    char payload[16000]; // max size for udp transfer
};



void initRTPHeader(struct RTPHeader* header) {
    header->pktNumber = 0;
    header->ack = 0;
}

void updateRTPHeader(struct RTPHeader* header, int ack) {
    header->pktNumber += 1;
    header->ack = (ack == 0 || ack == 1) ? ack : 0;
}

int initSocket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failure in UDP socket initialization");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

struct sockaddr_in prepareDestinationAddress(char *hostname, unsigned short int hostUDPport) {
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(hostUDPport);
    inet_pton(AF_INET, hostname, &destAddr.sin_addr);
    return destAddr;
}

// Improved rsend to handle file sending in chunks
void rsend(char* hostname, unsigned short int hostUDPport, char* filename, size_t bytesToTransfer) {
    int sockfd = initSocket();
    struct sockaddr_in destAddr = prepareDestinationAddress(hostname, hostUDPport);

    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        close(sockfd);
        return;
    }

    struct RTPHeader header;
    initRTPHeader(&header);

    size_t bytesRead;
    while ((bytesRead = fread(header.payload, 1, MAX_PAYLOAD_SIZE, file)) > 0) {
        if (sendto(sockfd, &header, sizeof(header) + bytesRead, 0, (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0) {
            perror("sendto failed");
            break;
        }

        // Wait for ack from server before continuing...
        // This part is left as an exercise. You would likely use recvfrom() here.

        updateRTPHeader(&header, 1); // Assuming we received ack = 1
    }

    fclose(file);
    close(sockfd);
}

int main(int argc, char** argv) {

     if (argc != 5) {
        fprintf(stderr, "Usage: %s receiver_hostname receiver_port filename bytes_to_transfer\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* hostname = argv[1];
    unsigned short int hostUDPport = (unsigned short int)atoi(argv[2]);
    char* filename = argv[3];
    size_t bytesToTransfer = (size_t)atoll(argv[4]);

    rsend(hostname, hostUDPport, filename, bytesToTransfer);

    return (EXIT_SUCCESS);
}   