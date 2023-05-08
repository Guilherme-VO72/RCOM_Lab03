#define MAX_SIZE 1024
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define BAUDRATE B38400
#define TRANSMITTER 1
#define RECEIVER 0

int llopen(char *port, int flag);

int llwrite(int fd, char *buffer, int length);

int llread(int fd, char *buffer);

int llclose(int fd);

typedef enum{
    tx,
    rx,
} DLRole;

typedef enum{
    START,
    FLAG_RCV,
    ADD_RCV,
    C_RCV,
    BCC_OK,
    SMSTOP,
} DLSM;

typedef struct linkLayer{
    char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate; /*Velocidade de transmissão*/
    unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
    unsigned int timeout; /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    char frame[MAX_SIZE]; /*Trama*/
} linkLayer;

