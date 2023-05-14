#include "datalinklayer.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

int fT; 
int byteCounter, fdr, nT, timeout;
struct termios oldtio,newtio;
volatile int STOP=FALSE;

void halarm(int signal)
{
    printf("alarme # %d\n", acounter);
    aflag=1;
    acounter++;
}

int stAlarm(int timeout, int tries)
{
    
    (void)signal(SIGALRM, halarm);
    //while(acounter < tries){
        if (aflag) {
            aflag=0;
            alarm(timeout);  
        }
    //}

    return 0;
}

int llopen(linkLayer connectionParameters){
    DLRole role;

    nT = connectionParameters.numTries;
    timeout = connectionParameters.timeOut;
    if (connectionParameters.role == 0){
        role = tx;
    }
    else if (connectionParameters.role == 1){
        role = rx;
    }
    else {
        //perror(flag);
        return (-1);
    }

    /* open the device to be non-blocking (read will return immediatly) */
    fdr = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    // fdr = open(argv[1], O_RDWR | O_NOCTTY);
    if (fdr < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fdr, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fdr, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fdr, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("\n____LLOPEN____\n");

    if (role == tx){
        
        unsigned char frbuf[5] = {0}, respbuf[5] = {0};

        frbuf[0] = 0x5C;
        frbuf[1] = 0x01;
        frbuf[2] = 0x03;
        frbuf[3] = 0x01 ^ 0x03;
        frbuf[4] = 0x5C;
        fT = 1;
        

        while(acounter < connectionParameters.numTries){

            if(aflag){
                int rb = write(fdr, frbuf, sizeof(frbuf));
                printf("SET MESSAGE SENT, %d bytes were written.\n", rb);
                stAlarm(connectionParameters.timeOut, connectionParameters.numTries);
                fT = 0;
            }
            

            int rr = read(fdr, respbuf, 5);
            if(rr != -1 && respbuf[0] == 0x5C){
                if(respbuf[1] != 0x01 || respbuf[2] != 0x07 || respbuf[3] != (respbuf[1] ^ respbuf[2])){
                    printf("UA is wrong - 0x%02x%02x%02x%02x%02x\n", respbuf[0],respbuf[1],respbuf[2],respbuf[3],respbuf[4]);
                }
                else{
                    printf("UA correctly recieved - 0x%02x%02x%02x%02x%02x\n", respbuf[0],respbuf[1],respbuf[2],respbuf[3],respbuf[4]);
                    return fdr;
                }
            }
            else if (rr == -1){
                printf("Waiting.\n");
                pause();
            }


        }

        if(acounter >= connectionParameters.numTries){
            printf("Alarm reached max number of tries.\n");
            return -1;
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

int llwrite(char* buf, int bufSize){
    //Should be working (impossivel testar 4now pq falta resposta por parte do receiver), not tested. TO BE FINISHED!!!

    printf("\n____LLWRITE____\n");

    //for now no message beyond 1000-6 bytes
    if(bufSize > 994){
        printf("Too big, sry.\n");
        return -1;
    }

    unsigned char bcc2 = 0x00, frame[1000] = {0}, fresp[5] = {0};
    //BCC calculation
    for (int i = 0; i<bufSize; i++){
        bcc2 = bcc2 ^ buf[i];
    }

    frame[0] = 0x5C;
    frame[1] = 0x01;
    frame[2] = 0x00;
    frame[3] = frame[1] ^ frame[2];

    int j=4;
    for(int i = 0; i<bufSize; i++){
        frame[j] = buf[i];
        j++;
    }

    frame[j] = bcc2;
    frame[j+1] = 0x5C;

    acounter = 0;
    aflag = 1;
    byteCounter=0;

    while(acounter < nT){

        if(aflag){
            int rb = write(fdr, frame, sizeof(frame));
            byteCounter += rb;
            printf("I FRAME SENT, %d bytes were written.\n", rb);
            stAlarm(timeout, nT);
        }
            

        int rr = read(fdr, fresp, 5);
        if(rr != -1 && fresp[0] == 0x5C){
            if(fresp[1] != 0x01 || fresp[2] != 0x21 || fresp[3] != (fresp[1] ^ fresp[2])){
                printf("RR is wrong - 0x%02x%02x%02x%02x%02x\n", fresp[0],fresp[1],fresp[2],fresp[3],fresp[4]);
                pause();
            }
            else{
                printf("RR correctly recieved - 0x%02x%02x%02x%02x%02x\n", fresp[0],fresp[1],fresp[2],fresp[3],fresp[4]);
                break;
            }
        }
        else if (rr == -1){
            printf("Waiting.\n");
            pause();
        }


    }

    if(acounter >= nT){
        printf("Alarm reached max number of tries.\n");
        return -1;
    }

    if(byteCounter < 1){
        return -1;
    } else { return 0; }
}

int llclose(int showStatistics){
    
    //can't use byteCounter para total de bytes, adaptar quando fizer transmissao de + do q 1 frame I

    // can't really do without knowing role, perguntar na aula
    // Se eu der disconnect de um lado como é que dou do outro? É suposto admitir que só o transmissor inicia disconnects?
    // Also, caso seja, dou close do lado do receiver no llread?
    // To be done later depois de falar com a prof.
    
    
    return 0;
}