/* Sample Program to demonstrate the drawing functions in GBDK */
/* Jon Fuge jonny@q-continuum.demon.co.uk */

#include <gb/gb.h>
#include <gb/drawing.h>

void main()
{
 
    color(LTGREY,WHITE,SOLID);
    circle(140,20,15,M_FILL);
    
    color(BLACK,WHITE,SOLID);
    circle(140,20,10,M_FILL);
    color(DKGREY,WHITE,XOR);
    circle(120,40,30,M_FILL);
    
}
