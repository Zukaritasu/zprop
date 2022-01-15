# zprop

----

**zprop** es una libreria que contiene funciones para escribir y leer archivos de propiedades **.properties**

## Includes
**fileprop.h** contiene las funciones para agregar, remover, editar, conversion y otras mas **.properties**.

### Funciones

```h

/* 
 * Retorna true si la clave existe en el conjunto de propiedades. Si los parámetros son inválidos
 * la función retorna false o en el caso que no exista la clave
 */
bool zp_contains(pzprop prop, cstr_t key);

/* 
 * Remueve una clave y su valor del conjunto de propiedades. Si los parámetros son invalidos o
 * la clave no existe la función retorna false. Tambien puede retornar false si ocurrió un error
 * al reasignar memoria, en este caso debe consultar errno 
 */
bool zp_remove(pzprop prop, cstr_t key);

/* 
 * Reemplaza el valor de una clave por otro. El valor puede ser una cadena vacía o NULL. La
 * función puede retornar false si los parámetro son inválidos, la clave no existe o un error al
 * reasignar memoria, para lo ultimo mencionado debe consultar errno 
 */
bool zp_replace(pzprop prop, cstr_t key, cstr_t value);

/* 
 * Agrega una nueva clave y su valor al conjunto de propiedades. Para que la función tenga éxito
 * la clave antes que nada no debe existir en el conjunto de propiedades, los parámetros deben
 * ser validos además la clave no puede ser una cadena vacía y por último es posible que ocurra
 * un error al reasignar memoria por lo cual debe consultar errno
 */
bool zp_add(pzprop prop, cstr_t key, cstr_t value);

/* 
 * Retorna el tamaño o la cantidad de propiedades existentes. La funcion puede retornar 0 si el
 * parametro es invalido o no existen propiedades 
 */
size_t zp_size(pzprop prop);

/* 
 * Lee un archivo de propiedades .properties. La función lee todo el conjunto de propiedades en
 * el archivo.
 * 
 * Una propiedad está constituida por su clave y su valor como un diccionario. Una propiedad
 * puede o no tener un valor especificado, como por ejemplo “clave= <vacio>”. Si no se encuentra
 * un separador como ‘=’ o ‘:’ y solo se encuentra un espacio entre la clave y el valor, ese
 * espacio es considerado un separador entre los dos, ejemplo “clave <espacio> valor”.
 * 
 * El carácter \ seguido de 't', 'f', 'r', u una 'n' es convertido a un carácter de control
 * cuando es leído el valor. La función puede retornar false si los parámetros son inválidos, el
 * archivo no existe, error de lectura o error al reasignar memoria. Debe consultar errno
 *
 * Tenga en cuenta que la función también crea un espacio en memoria para el puntero pzprop como
 * la función ‘zp_create’ si el puntero hace referencia a NULL, ejemplo:
 *
 * pzprop prop = NULL;
 * zp_read(&prop, “archive.properties”);
 */
bool zp_read_a(pzprop* prop, cstr_t filename);
bool zp_read_w(pzprop* prop, cwstr_t filename);

/*
 * Escribe un archivo de propiedades. Si el valor contiene caracteres de control como ‘\t’, ‘\f’,
 * ‘\r’ o ‘\n’ serán caracteres “visibles en el archivo” como por ejemplo:
 *
 * 		“clave=lunes’\n’martes’\n’\t’’”
 *
 * La función puede retornar false si los parámetros son inválidos o ocurrió un error en la
 * apertura, escritura del archivo o al reasignar memoria. Debe consultar errno
 */
bool zp_write_a(pzprop prop, cstr_t filename);
bool zp_write_w(pzprop prop, cwstr_t filename);

// Compatibilidad con ASCII y UNICODE
#ifdef UNICODE
#  define zp_read zp_read_w
#  define zp_write zp_write_w
#else
#  define zp_read zp_read_a
#  define zp_write zp_write_a
#endif

/*
 * Crea un espacio en memoria para el puntero 'pzprop'. La funcion puede retornar false si ocurre
 * un error al solicitar memoria
 */
pzprop zp_create(void);

/*
 * Libera la memoria usada por el puntero 'pzprop' y el conjunto de propiedades
 */
void zp_free(pzprop prop);

/*
 * Retorna el valor de la clave solicitada. La función puede retornar NULL si los parámetros son
 * inválidos o la clave no existe en el conjunto de propiedades aunque en tal caso una clave
 * puede tener un valor NULL, hay que tomar eso en cuenta
 */
cstr_t zp_value(pzprop prop, cstr_t key);

/*
 * Las siguientes funciones convierte el valor en el tipo de dato que retorna cada función. Si el
 * valor es inválido siendo NULL o un valor que no se puede convertir, la función retorna 0 como
 * valor por defecto
 */
int zp_int(pzprop prop, cstr_t key);
int64_t zp_int64(pzprop prop, cstr_t key);
bool zp_bool(pzprop prop, cstr_t key);
double zp_double(pzprop prop, cstr_t key);
```