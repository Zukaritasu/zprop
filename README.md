# zProp

----

zProp es una libreria que contiene herramientas para escribir y leer archivos de propiedades **.properties**

## Includes
**zprop.h** contiene las funciones para agregar, remover, editar, crear, leer y escribir archivos **.properties**. Cuenta tambien con un enumerado *zPropError* que especifica el error ocurrido en una función

**zpropdef.h** contiene el tipo de dato **zProp** que es usado por las funciones declaradas en **zprop.h**

### Funciones

```cpp

/* Crea un espacio en memoria en donde se guardaran las propiedades */
zProp zCreateProp();

/* Retorna true si la clave existe */
bool zContainsKey(zProp, zText);

/* Remueve una clave */
bool zRemoveKey(zProp, zText);

/* Libera los recursos utilizados */
bool zFreeProp(zProp);

/* Retorna un valor usando como referencia su clave */
zText zGetValue(zProp, zText);

/* Agrega una nueva propiedad */
zPropError zAddProp(zProp, zText, zText);

/* Retorna la cantidad de propiedades */
int zGetCountProp(zProp);

/* Lee un archivo de propiedades */
zPropError zPropReadA(zProp*, zText);

/* Lee un archivo de propiedades */
zPropError zPropReadW(zProp*, const wchart_t*);

/* Escribe un archivo de propiedades */
zPropError zPropWriteA(zProp, zText);

/* Escribe un archivo de propiedades */
zPropError zPropWriteW(zProp, const wchart_t*);
```
