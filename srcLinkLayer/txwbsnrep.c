#include "datalinklayer.c"
#include <stdio.h>

int main(int argc, char** argv){

    linkLayer con;
    strcpy(con.serialPort, argv[1]);
    con.baudRate = BAUDRATE_DEFAULT;
    con.numTries = 5;
    con.timeOut = 1;
    con.role = TRANSMITTER;
    unsigned char pack[100];

    int r = llopen(con);
    if(r != -1){
        printf("\n\nOK\n\n");
        for (int i = 0; i<100; i++){
          if(i == 49){
            pack[i] = 0x72; 
          }
          else{
            pack[i] = 0x5C;
          }
        }
        int rt = llwrite(pack, 100);
        if(rt != 100){
            printf("ERROR\n");
        }
        int rtt = llwrite(pack, 100);
        int rttt = llwrite(pack, 100);
    }    
    return 0;
}
