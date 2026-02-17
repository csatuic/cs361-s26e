#include<stdio.h>
#include<stdlib.h>

unsigned char hex_value(char c) {
    switch(c) {
        case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9':
            return c-'0';            
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            return c-'A'+0xA;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            return c-'a'+0xA;
        default: 
            fprintf(stderr,"What kind of digit is %c anyway?",c);
            exit(1);
    }
}

unsigned long to_long(char *s) {

}
void to_ints(char *s, unsigned int* ints) {

}
void to_shorts(char *s, unsigned short* shorts) {

}
void to_bytes(char *s, unsigned char* bytes) {
    
}

int main(int argc, char** argv) {
    char *s = "8877665040a0b1c2"; 
    printf("The string is '%s'\n",s);

    printf("As bytes, it is ");
    unsigned char bytes[8];
    to_bytes(s,bytes);
    for(int i=0;i<8;i++) {
        printf("0x%x ",bytes[i]);
    }
    printf("\n");

    printf("As shorts, it is ");
    unsigned short shorts[4];
    to_shorts(s,shorts);
    for(int i=0;i<4;i++) {
        printf("0x%x ",shorts[i]);
    }
    printf("\n");

    printf("As ints, it is ");
    unsigned int ints[2];
    to_ints(s,ints);
    for(int i=0;i<2;i++) {
        printf("0x%x ",ints[i]);
    }
    printf("\n");

    printf("As a long, it is 0x%lx\n",to_long(s)); 
}
