# zProp

>zProp es una libreria que contiene herramientas para escribir y leer archivos de propiedades **.properties**

## Includes
> **zprop.h** contiene las funciones para agregar, remover, editar, crear, leer y escrir archivos **.properties**. Cuenta tambien con un enumerado *zPropError* que especifica el error ocurrido en una función

> **zpropdef.h** contiene el tipo de dato **zProp** que es usado por las funciones declaradas en **zprop.h**

### Funciones

<link href="syntax.css" rel="stylesheet" type="text/css">

<font>
<li> <func>zCreateProp</func>(): <typedef>zProp</typedef>

<li> <func>zContainsKey</func>(<typedef>zProp</typedef>, <typedef>zText</typedef>): <primitive>bool</primitive>

<li> <func>zRemoveKey</func>(<typedef>zProp</typedef>, <typedef>zText</typedef>): <primitive>bool</primitive>

<li> <func>zFreeProp</func>(<typedef>zProp</typedef>): <primitive>bool</primitive>

<li> <func>zGetValue</func>(<typedef>zProp</typedef>, <typedef>zText</typedef>): <typedef>zText</typedef>

<li> <func>zAddProp</func>(<typedef>zProp</typedef>, <typedef>zText</typedef>, <typedef>zText</typedef>): <typedef>zPropError</typedef>

<li> <func>zGetCountProp</func>(<typedef>zProp</typedef>): <primitive>int</primitive>

<li> <func>zPropReadA</func>(<typedef>zProp</typedef>*, <typedef>zText</typedef>): <typedef>zPropError</typedef>

<li> <func>zPropReadW</func>(<typedef>zProp</typedef>*,  <primitive>const</primitive> <typedef>wchart_t</typedef>*): <typedef>zPropError</typedef>

<li> <func>zPropWriteA</func>(<typedef>zProp</typedef>, <typedef>zText</typedef>): <typedef>zPropError</typedef>

<li> <func>zPropWriteW</func>(<typedef>zProp</typedef>, <primitive>const</primitive> <typedef>wchart_t</typedef>*): <typedef>zPropError</typedef>
</font>