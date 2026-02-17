#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

union proud {
    uint64_t contracts;
    struct {
        uint16_t pickets;
        uint8_t strikes;
        uint8_t bat_beatings,kneecaps,concrete_shoes;        
    } enforcement;
    uint16_t membership[2][2];    
};

int main(int argc, char** argv) {
    union proud activities;
    activities.contracts = 0x010220F01234;

    printf("%u bat beatings\n", activities.enforcement.bat_beatings);
}
