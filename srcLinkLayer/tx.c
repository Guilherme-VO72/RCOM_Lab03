#include "datalinklayer.c"
#include <stdio.h>

int main(int argc, char** argv){

    linkLayer con;
    strcpy(con.serialPort, argv[1]);
    con.baudRate = BAUDRATE_DEFAULT;
    con.numTries = 5;
    con.timeOut = 1;
    con.role = TRANSMITTER;

    int r = llopen(con);
    if(r != -1){
        printf("\n\nOK\n\n");
        llwrite("Hello World", 12);
    }    
    return 0;
}