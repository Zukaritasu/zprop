#include "Properties.hpp"

#include <iostream>

int main()
{
	Properties prop;

	/* En el archivo usuario.properties ->
		
		Nombre=Zukaritasu
		Pais=Venezuela
	*/
	if (!prop.Load("usuario.properties")) {
		/* error de lectura */
		std::cout << "Error al leer la informacion del usuario\n";
		return 1;
	}
	
	std::cout << prop["Nombre"] << std::endl;
	/* Salida en consola ->
		
		Zukaritasu
	*/

	return 0;
}