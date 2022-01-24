#include "..\include\fileprop.h"
#include <stdio.h>

int main(void)
{
    pzprop prop = zp_create();
    /* guardar un rectangulo */
    zprect rect = { 10, 58, 36, 78 };
    zp_addr(prop, "rect", &rect);
    /* guardar una posicion */
    zppoint point = { -110, 68 };
    zp_addp(prop, "point", &point);
    /* guardar un tamano */
    zpsize size = { 400, 890 };
    zp_addsz(prop, "size", &size);

    zp_write(prop, L"geometry.properties");
    zp_free(&prop);

    /*
        En el archivo ->

        rect=10,58,36,78
        point=-110,68
        size=400,890
    */

    return 0;
}
