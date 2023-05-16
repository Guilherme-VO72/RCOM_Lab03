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
        //sleep(4);
        int rb = llread(packet);
        if(rb != -1){
            printf("\n\n____PACKET RECEIVED FROM LINK LAYER____\n0x");
            for (int i = 0; i<rb; i++){
                printf("%02x",packet[i]);
            }
            printf("\n");
        }
        int rbb = llread(packet);
        if(rbb != -1){
            printf("\n\n____PACKET RECEIVED FROM LINK LAYER____\n0x");
            for (int i = 0; i<rbb; i++){
                printf("%02x",packet[i]);
            }
            printf("\n");
        }
        int rbbb = llread(packet);
        if(rbbb != -1){
            printf("\n\n____PACKET RECEIVED FROM LINK LAYER____\n0x");
            for (int i = 0; i<rbbb; i++){
                printf("%02x",packet[i]);
            }
            printf("\n");
        }
    }    
    return 0;
}
