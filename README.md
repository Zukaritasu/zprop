# zProp

>zProp es una libreria que contiene herramientas para escribir y leer archivos de propiedades **.properties**

## Includes
> **zprop.h** contiene las funciones para agregar, remover, editar, crear, leer y escrir archivos **.properties**. Cuenta tambien con un enumerado *zPropError* que especifica el error ocurrido en una función

> **zpropdef.h** contiene el tipo de dato **zProp** que es usado por las funciones declaradas en **zprop.h**

### Funciones

```cpp
zProp zCreateProp();

bool zContainsKey(zProp, zText);

bool zRemoveKey(zProp, zText);

bool zFreeProp(zProp);

zText zGetValue(zProp, zText);

zPropError zAddProp(zProp, zText, zText);

int zGetCountProp(zProp);

zPropError zPropReadA(zProp*, zText);

zPropError zPropReadW(zProp*, const wchart_t*);

zPropError zPropWriteA(zProp, zText);

zPropError zPropWriteW(zProp, const wchart_t*);
```