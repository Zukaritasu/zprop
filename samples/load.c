#include <include/fileprop.h>
#include <stdio.h>

int main(void)
{
    pzprop prop = NULL; /*¡¡debe ser NULL!!*/

    /* En el archivo usuario.properties ->
		
		Nombre=Zukaritasu
		Pais=Venezuela
	*/
    if (zp_read(&prop, L"archivo.properties")) {
        printf("%s\n", zp_value(prop, "Nombre"));
        /* Salida en consola ->
		
		    Zukaritasu
	    */
        zp_free(&prop);
        return 0;
    }
    
    fprintf(stderr, "Error al leer la informacion del usuario\n");
    return 1;
}
