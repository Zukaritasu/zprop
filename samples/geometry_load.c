#include "..\include\fileprop.h"
#include <stdio.h>

int main(void)
{

    pzprop prop = NULL;
    /*
        En el archivo ->

        rect=10,58,36,78
        point=-110,68
        size=400,890
    */
    zp_read(&prop, L"geometry.properties");
    zprect rect;
    zp_valuer(prop, "rect", &rect);

    printf("Rect [X=%d,Y=%d,Width=%d,Height=%d]\n", 
           rect.x, 
           rect.y, 
           rect.width, 
           rect.height);

    /*
        En consola ->

        Rect [X=10,Y=58,Width=36,Height=78]
    */

    zp_free(&prop);

    return 0;
}