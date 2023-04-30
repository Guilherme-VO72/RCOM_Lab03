#include "datalinklayer.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

extern int fT; 
int nT=5, frameCounter, fdr;
struct termios oldtio,newtio;
volatile int STOP=FALSE;

int llopen(char *port, int flag){
    linkLayer dll;
    DLRole role;

    strcpy(dll.port, port);

    if (flag == 1){
        role = tx;
    }
    else if (flag == 0){
        role = rx;
    }
    else {
        //perror(flag);
        return (-1);
    }

    fdr = open(port, O_RDWR | O_NOCTTY );
    
    if (fdr < 0) { 
        perror (port);
        return (-1); 
    }

    if ( tcgetattr(fdr, &oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        return (-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fdr, TCIOFLUSH);

    if (tcsetattr(fdr,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        return (-1);
    }

    printf("\n____LLOPEN____\n");

    if (role == tx){

        unsigned char frbuf[5] = {0}, respbuf[5] = {0};

        frbuf[0] = 0x5C;
        frbuf[1] = 0x01;
        frbuf[2] = 0x03;
        frbuf[3] = 0x01 ^ 0x03;
        frbuf[4] = 0x5C;

        //fT = 1;    

        int rb = write(fdr, frbuf, sizeof(frbuf));
        printf("SET MESSAGE SENT, %d bytes were written.\n", rb);

        int rr = read(fdr, respbuf, 5);
        if(rr != -1 && respbuf[0] == 0x5C){
            if(respbuf[1] != 0x01 || respbuf[2] != 0x07 || respbuf[3] != (respbuf[1] ^ respbuf[2])){
                printf("UA is wrong - 0x%02x%02x%02x%02x%02x\n", respbuf[0],respbuf[1],respbuf[2],respbuf[3],respbuf[4]);
                return (-1);
            }
            else{
                printf("UA correctly recieved - 0x%02x%02x%02x%02x%02x\n", respbuf[0],respbuf[1],respbuf[2],respbuf[3],respbuf[4]);
            }
        }
        else{
            printf("Wrong answer from reciever.\n");
            return (-1);
        }


    }
    else if(role == rx){
        unsigned char frbuf[5] = {0}, reb[5] = {0};
        DLSM sm = START;
        int to_read =1;

        while(STOP == FALSE){
            if(to_read){
                int rb = read(fdr, reb, 1);
                if (rb <= 0) continue;
            }


            switch (sm){
                case START:
                    if(reb[0] == 0x5C){
                        sm = FLAG_RCV;
                    }
                    break;

                case FLAG_RCV:
                    if(reb[0] == 0x01){
                        sm = ADD_RCV;
                    }
                    else if(reb[0] == 0x5C){
                        sm = FLAG_RCV;
                    }
                    else{
                        sm = START;
                    }
                    break;

                case ADD_RCV:
                    if(reb[0] == 0x03){
                        sm = C_RCV;
                    }
                    else if(reb[0] == 0x5C){
                        sm = FLAG_RCV;
                    }
                    else{
                        sm = START;
                    }
                    break;

                case C_RCV:
                    if(reb[0] == (0x01 ^ 0x03)){
                        sm = BCC_OK;
                    }
                    else if(reb[0] == 0x5C){
                        sm = FLAG_RCV;
                    }
                    else{
                        sm = START;
                    }
                    break;

                case BCC_OK:
                    if(reb[0] == 0x5C){
                        sm = SMSTOP;
                        to_read = 0;
                    }
                    else{
                        sm = START;
                    }
                    break;

                case SMSTOP:
                    STOP = TRUE;
                    printf("SET MESSAGE READ WITH SUCCESS\n");
                    break;
            }

        }

        frbuf[0] = 0x5C;
        frbuf[1] = 0x01;
        frbuf[2] = 0x07;
        frbuf[3] = frbuf[1] ^ frbuf[2];
        frbuf[4] = 0x5C;

        int rb = write(fdr, frbuf, sizeof(frbuf));
        printf("UA MESSAGE SENT, %d bytes were written.\n", rb);
    }

    return fdr;
}