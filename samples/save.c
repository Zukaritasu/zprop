#include <include/fileprop.h>
#include <stdio.h>

int main(void)
{
    pzprop prop = zp_create();
    if (prop == NULL)
    {
        fprintf(stderr, "error al crear el handle\n");
        return 1;
    }
    
    zp_add  (prop, "Nombre", "Zukaritasu");
    zp_add  (prop, "Pais",   "Venezuela" );
    zp_addul(prop, "Edad",   21          );
    
    if (!zp_write(prop, L"archivo.properties")) {
         fprintf(stderr, "error al guardar la informacion del usuario\n");
         zp_free(&prop);
         return 1;
    }

    /* En el archivo usuario.properties ->
		
		Nombre=Zukaritasu
		Pais=Venezuela
        Edad=21
	*/

    zp_free(&prop);
    return 0;
}