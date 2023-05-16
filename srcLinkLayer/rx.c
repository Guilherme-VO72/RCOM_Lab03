#include <stdio.h>
#include "datalinklayer.c"

int main(int argc, char** argv){
    
    linkLayer con;
    strcpy(con.serialPort, argv[1]);
    con.baudRate = BAUDRATE_DEFAULT;
    con.numTries = 5;
    con.timeOut = 1;
    con.role = RECEIVER;
    char packet[1000];

    int r = llopen(con);
    if(r != -1){
        printf("\n\nOK\n\n");
        int rb = llread(packet);
        if(rb != -1){
            for (int i = 0; i<rb; i++){
                printf("0x%02x\n",packet[i]);
            }
        }
    }    
    return 0;
}