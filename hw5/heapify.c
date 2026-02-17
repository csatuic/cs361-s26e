#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<malloc.h>
#include<string.h>

const char* qbf = "The quick brown fox jumped over the lazy dog.";

int main(int argc, char** argv) {
    
    char *chunks[strlen(qbf)];
    for(int i=1;i<strlen(qbf);i++) {
        chunks[i] = malloc(i+1);
        memcpy(chunks[i], qbf, i);
        chunks[i][i]='\0';
    }
    // break here
    for(int i=1;i<strlen(qbf);i++) {
        free(chunks[i]);
        
        // this helps settle the heap to an easier-to-read state
        // it's not something you want in a real program
        free(malloc(1000000));
    }
}
