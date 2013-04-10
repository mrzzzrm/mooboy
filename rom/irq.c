#include <gb/gb.h>
#include <gb/console.h>
#include <stdio.h>


void main() {
    printf("!");
    while(1) {
        waitpad(J_A);   
        printf("?");   
        delay(100);        
    }
}
