#include <stdint.h>

struct TCPHeader {
    uint16_t sourcePort;       
    uint16_t destinationPort;        
    uint32_t sequenceNumber;   
    uint32_t acknowledgementNumber;        
    uint16_t dataOffsetFlags;  
    uint16_t window;      
    uint16_t checksum;       
    uint16_t urgent;    
};

