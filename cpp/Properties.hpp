/* Copyright (C) 2021-2022 Zukaritasu

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace std {
	/**
	 * @brief Para tener compatibilidad	con la plantilla que tiene el
	 * metodo 'Properties::Add' se creo esta funcion para que aceptara
	 * (const char* o string), tambien acompañado con la funcion
	 * (string to_string(const string& value))
	 * 
	 * @param value 
	 * @return string 
	 */
	inline string to_string(const string& value) {
		return value;
	}

	/**
	 * @brief Convierte un caracter en un string. Basicamente es un
	 * string con un solo caracter. Esta funcion trabaja con el metodo
	 * Properties::Add que tiene una plantilla
	 * 
	 * @param value El caracter a convertir
	 * @return string 
	 */
	inline string to_string(char value) {
		string str;
		str += value;
		return str;
	}
}

class Properties
{
protected:

	std::vector<std::string*> data;
private:

	/**
	 * @brief La funcion retorna la posicion de donde se encuentra la
	 * clave 'key'. En el caso que la clave sea una cadena vacia o que
	 * no exista la clave en el conjunto de propiedades la funcion
	 * retorna 'false'
	 * 
	 * @param key la clave de la cual se obtendra la posicion
	 * @return size_t (UINT32_MAX) o la posicion
	 */
	size_t IndexOf(const std::string& key);

	/**
	 * @brief Verifica que una cadena no este vacia "tomando en cuenta
	 * los espacios en blanco" y si lo esta retorna true, de lo contrario
	 * false. Esta funcion es usada para verificar que la clave no sea
	 * una cadena vacia 
	 * 
	 * @param key    la clave que se analizara su contenido
	 * @return 
	 */
	bool IsEmpty(const std::string& key);

	/**
	 * @brief Lee una linea de un archivo de propiedades. Los espacios
	 * "si existen en el inicio de la nueva linea" y los comentarios son
	 * ignorados. La funcion puede retornar false si se llego al final
	 * del archivo o ocurrio un error al leer la linea
	 * 
	 * @param line   el string donde se guarda la linea leida
	 * @param in     entrada de lectura de archivo
	 * @return 
	 */
	bool ReadLine(std::string& line, std::ifstream& in);

	/**
	 * @brief Convierte un caracter interpretado a un caracter de
	 * control los cuales son '\t', '\f', '\r' y '\n'
	 * 
	 * Se puede decir que un caracter interpretado se entiende como por
	 * ejemplo escribir un salto de linea '\n' en un archivo con un editor
	 * de texto de esta manera "\n" identicamente pero como tal no es un
	 * caracter sino que se interpreta como un caracter de salto de linea
	 * sin ni siquiera serlo, es solo para que sea <visible> de que existe
	 * tal caracter de control en el valor de la clave
	 * 
	 * La funcion recibe un string a convetir
	 * 
	 * @param value el string a convetir
	 * @return std::string* resultado
	 */
	std::string* Convert(const std::string &value);
	
	/**
	 * @brief Cambia los caracteres de control '\t', '\f', '\r' y '\n'
	 * agregando una barra diagonal '\' y luego el caracter que representa
	 * ese caracter para que desde el archivo se pueda presenciar los
	 * caracteres de control.
	 * 
	 * @param value el string al que cambiaran los caracteres
	 * @param out Donde se guardara el string con los cambios
	 */
	void ResolveString(std::ofstream& out, const std::string& value);

	/**
	 * @brief Agrega un valor al conjunto de propiedades. Este valor
	 * puede ser una clave o el valor de la clave, el parametro 'is_value'
	 * lo indica. En el caso que sea un valor se debe llamar el metodo
	 * Properties::Convert  para que cambie los caracteres de control
	 * interpretados por los no interpretados
	 * 
	 * @param value el valor a gregar (puede ser una clave o el valor de
	 * la clave)
	 * @param is_value indica si 'value' es una clave o el valor de la
	 * clave
	 */
	void AddValue(const std::string &value, bool is_value = false);

	/**
	 * @brief Agrega una propiedad en el conjunto de propiedades. Esta
	 * metodo es llamada desde el metodo Properties::Read para procesar
	 * la linea del archivo que contiene la propiedad. La clave y el
	 * valor estan separados por los caracteres '=' o ':' tambien es
	 * valido un espacio en blanco. La clave puede o no contener un valor,
	 * en ese caso se asigna 'nullptr' como valor por defecto
	 * 
	 * @param line la linea del archivo a procesar
	 */
	void AddProperty(const std::string &line);

	/**
	 * @brief Remueve un bloque de elementos del vector indicando la
	 * posicion inicial y la final. Antes de remover los strings en ese
	 * rango se destruyen llamando al destructor y luego es posible
	 * removerlos del vector
	 * 
	 * @param begin 
	 * @param end 
	 */
	void RemoveRange(uint32_t begin, uint32_t end);
	
	/**
	 * @brief Agrega una nueva clave y su valor al conjunto de
	 * propiedades. Este metodo es llamado por el metodo Properties:Add
	 * 
	 * @param key La clave
	 * @param value EL valor de la clave
	 * @return 
	 */
	bool AddValueForType(const std::string& key, const std::string& value);
public:

	Properties() {}
	Properties(const Properties&) = delete;
	~Properties();

	/**
	 * @brief Retorna true si la clave existe en el conjunto de
	 * propiedades. La clave no puede ser una cadena vacia y ademas
	 * debe existir en el conjunto de propiedades
	 * 
	 * @param key La clave a validar si existe
	 * @return 
	 */
	bool Contains(const std::string& key);

	/**
	 * @brief Remueve una clave y su valor del conjunto de propiedades.
	 * Si la clave no existe el metodo retorna false
	 * 
	 * @param key La clave a remover (y su valor)
	 * @return
	 */
	bool Remove(const std::string& key);

	/**
	 * @brief Reemplaza el valor de una clave por otro. El valor puede
	 * ser una cadena. El metodo puede retornar false si la clave no
	 * existe en el conjunto de propiedades
	 * 
	 * @param key La clave
	 * @param value El valor de la clave (puede ser una cadena vacia)
	 * @return
	 */
	bool Replace(const std::string& key, const std::string& value);

	/**
	 * @brief Agrega una nueva clave y su valor al conjunto de
	 * propiedades. Para que el metodo tenga éxito la clave no debe 
	 * existir en el conjunto de propiedades además la clave no puede
	 * ser una cadena vacía.
	 * 
	 * Este metodo aceptar los siguientes tipos (string, char, int,
	 * unsigned, long, long long, unsigned long, unsigned long long,
	 * float, double y long double)
	 * 
	 * @tparam V Para lograr aceptar cualquier tipo de valor (enteros,
	 * decimales, booleanos o string)
	 * @param key La clave
	 * @param value El valor de la clave
	 * @return 
	 */
	template<typename V> 
	bool Add(const std::string& key, V value) {
		return AddValueForType(key, std::to_string(value));
	}

	/**
	 * @brief Retorna el tamano o la cantidad de propiedades existentes.
	 * @return size_t 
	 */
	size_t Size() const;

	/**
	 * @brief Lee un archivo de propiedades .properties. La función lee
	 * todo el conjunto de propiedades en el archivo.
	 * 
	 * Una propiedad está constituida por su clave y su valor como un
	 * diccionario. Una propiedad puede o no tener un valor especificado,
	 * como por ejemplo “clave= <vacio>”. Si no se encuentra un separador
	 * como ‘=’  o ‘:’ y solo se encuentra un espacio entre la clave y el
	 * valor, ese espacio es considerado un separador entre los dos,
	 * ejemplo:
	 * 
	 * 		“clave <espacio> valor”.
	 * 
	 * El carácter '\' seguido de 't', 'f', 'r', o una 'n' es convertido
	 * a un carácter de control cuando es leído el valor. El metodo puede
	 * retornar false si no existe el archivo o en el caso que ocurra un
	 * error en la lectura del archivo
	 * 
	 * @param filename El nombre del arhivo .properties
	 * @return 
	 */
	bool Load(const std::string& filename);

	/**
	 * @brief Escribe un archivo de propiedades. Si el valor contiene
	 * caracteres de control como ‘\t’, ‘\f’, ‘\r’ o ‘\n’ esos caracteres
	 * serán "visibles en el archivo" como por ejemplo:
	 * 
	 * 		“clave=lunes’\n’martes’\n’\t’’”
	 * 
	 * El metodo puede retornar false si ocurrio un error al crear el
	 * archivo en el caso si ocurre un error al escribir en el archivo
	 * 
	 * @param filename El nombre del arhivo .properties
	 * @return
	 */
	bool Save(const std::string& filename);

	/**
	 * @brief Imprime todo el conjunto de propiedades. El metodo muestra
	 * la clave y el valor con este formato "clave=valor" y deja un salto
	 * de linea. Si el valor de la clave es nullptr se muestra con este
	 * formato "clave="
	 */
	void Print() const;

	/**
	 * @brief Retorna el valor solicitado por la clave del valor. Si la
	 * clave no existe el metodo retorna nullptr aunque tambien puede
	 * retornar nullptr si la clave no tiene valor o en un comienzo se
	 * agrego el valor como una cadena vacia
	 * 
	 * @param key La clave del valor
	 * @return const char* El valor
	 */
	const char* GetValue(const std::string& key);

#define ZP_GET_VALUE(func, def_v)                       \
		auto value = GetValue(key);                     \
		if (value != nullptr) { return func##(value); } \
		return def_v;

	/**
	 * @brief Convierte el valor de la clave en un valor entero 'int'
	 * usando la funcion 'std::stoi'. Si el valor no se pudo convertir, el
	 * valor por defecto que retorna este metodo es 0 
	 * 
	 * @param key La clave del valor
	 * @return int 
	 */
	int GetInt(const std::string& key) {
		ZP_GET_VALUE(std::stoi, 0);
	}

	/**
	 * @brief Convierte el valor de la clave en un valor entero sin signo
	 * 'unsigned'usando la funcion 'std::stoul'. Si el valor no se pudo
	 * convertir, el valor por defecto que retorna este metodo es 0 
	 * 
	 * @param key La clave del valor
	 * @return int 
	 */
	unsigned GetUnsigned(const std::string& key) {
		ZP_GET_VALUE(std::stoul, 0);
	}

	/**
	 * @brief Convierte el valor de la clave en un valor entero
	 * largo 'long long' usando la funcion 'std::stoll'. Si el valor no se
	 * pudo convertir, el valor por defecto que retorna esta funcion es 0 
	 * 
	 * @param key La clave del valor
	 * @return long long 
	 */
	long long GetInt64(const std::string& key) {
		ZP_GET_VALUE(std::stoll, 0);
	}

	/**
	 * @brief Convierte el valor de la clave en un valor entero
	 * largo 'unsigned long long' usando la funcion 'std::stoull'. Si el
	 * valor no se pudo convertir, el valor por defecto que retorna esta
	 * funcion es 0 
	 * 
	 * @param key La clave del valor
	 * @return unsigned long long 
	 */
	unsigned long long GetULongLong(const std::string& key) {
		ZP_GET_VALUE(std::stoull, 0);
	}

	/**
	 * @brief Convierte el valor de la clave en un valor de tipo 'bool'
	 * usando como condicion si el valor es "true" o "1". Si el valor es
	 * diferente a lo mencionado la funcion retorna false
	 * 
	 * @param key La clave del valor
	 * @return bool
	 */
	bool GetBool(const std::string& key) {
		auto value = GetValue(key);
		if (value != nullptr) { 
			return std::strcmp(value, "true") == 0 || value[0] == '1';
		}
		return false;
	}

	/**
	 * @brief Convierte el valor de la clave en un valor de tipo 'double'
	 * usando la funcion 'std::stod'. Si el valor no se pudo convertir, el
	 * valor por defecto que retorna esta funcion es 0
	 * 
	 * @param key La clave del valor
	 * @return double 
	 */
	double GetDouble(const std::string& key) {
		ZP_GET_VALUE(std::stod, 0);
	}

	/**
	 * @brief Convierte el valor de la clave en un valor de tipo
	 * 'long double' usando la funcion 'std::stold'. Si el valor no se pudo
	 * convertir, el valor por defecto que retorna esta funcion es 0
	 * 
	 * @param key La clave del valor
	 * @return long double 
	 */
	long double GetLongDouble(const std::string& key) {
		ZP_GET_VALUE(std::stold, 0);
	}

	/**
	 * @brief Hace lo mismo que el metodo Properties::GetValue pero en este
	 * con el uso de un operador []
	 * 
	 * @param key  La clave del valor
	 * @return const std::string& 
	 */
	const char* operator[](const std::string& key) {
		return GetValue(key);
	}
};
