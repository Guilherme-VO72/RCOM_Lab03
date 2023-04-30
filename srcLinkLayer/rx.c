#include "datalinklayer.c"

int main(int argc, char** argv){
    
    if(llopen(argv[1], RECEIVER) != -1){
        printf("\n\nOK\n\n");
    }    
    return 0;
}