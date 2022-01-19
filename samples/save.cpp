#include "Properties.hpp"

#include <iostream>

int main()
{
	Properties prop;
	prop.Add("Nombre", "Zukaritasu");
	prop.Add("Pais", "Venezuela");
	if (!prop.Save("usuario.properties")) {
		/* error de escritura */
		std::cout << "Error al guardar la informacion\n";
		return 1;
	}
	
	std::cout << "Exito al guardar la informacion!\n";
	
	/* En el archivo ->
		
		Nombre=Zukaritasu
		Pais=Venezuela
	*/
	return 0;
}	