// Directivas necesarias para la conexión ODBC a SQL Server
#include <iostream>     // Directiva de entrada/salida estándar
#include <windows.h>    // Incluye funciones del sistema Windows
#include <sql.h>       // Incluye funciones core de ODBC
#include <sqlext.h>    // Incluye funciones extendidas de ODBC
#include <string>      // Manejo de cadenas de texto

using namespace std; 

// Función principal
int main() {
   // Declaración de variables para manejar la conexión ODBC
   SQLHENV henv;   // Manejador del ambiente ODBC
   SQLHDBC hdbc;   // Manejador de la conexión a la base de datos
   SQLRETURN ret;  // Variable para almacenar el resultado de las operaciones ODBC

   // Paso 1: Crear y asignar el manejador del ambiente ODBC
   // SQLAllocHandle asigna memoria para el ambiente ODBC
   ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
   if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
       cout << "Error al crear el ambiente ODBC" << endl;
       return 1;
   }

   // Paso 2: Configurar la versión de ODBC a utilizar
   SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

   // Paso 3: Crear y asignar el manejador de conexión
   // Este manejador se utilizará para establecer la conexión con SQL Server
   ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
   if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
       cout << "Error al crear el handle de conexión" << endl;
       SQLFreeHandle(SQL_HANDLE_ENV, henv);
       return 1;
   }

   // Paso 4: Establecer la conexión con la base de datos
   // La cadena de conexión especifica el driver, servidor, base de datos y tipo de autenticación
   SQLWCHAR connStr[] = L"DRIVER={SQL Server};SERVER=ALEX-GH;DATABASE=UNI_Empleados;Trusted_Connection=yes;";
   ret = SQLDriverConnectW(hdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

   // Paso 5: Verificar si la conexión fue exitosa
   if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
       cout << "Error al conectar con la base de datos" << endl;
       // Variables para almacenar información detallada del error
       SQLWCHAR sqlState[6], msg[SQL_MAX_MESSAGE_LENGTH];
       SQLINTEGER nativeError;
       SQLSMALLINT msgLen;

       // Obtener información detallada del error
       SQLGetDiagRecW(SQL_HANDLE_DBC, hdbc, 1, sqlState, &nativeError, msg, SQL_MAX_MESSAGE_LENGTH - 1, &msgLen);
       msg[SQL_MAX_MESSAGE_LENGTH - 1] = L'\0'; // Asegurar que la cadena termine correctamente

       // Mostrar información detallada del error
       wcout << L"SQL State: " << sqlState << L" Error: " << msg << endl;

       // Liberar recursos en caso de error
       SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
       SQLFreeHandle(SQL_HANDLE_ENV, henv);
       return 1;
   }

   cout << "Conexion exitosa a la base de datos!" << endl;

   // Paso 6: Cerrar la conexión y liberar recursos
   SQLDisconnect(hdbc);                        // Desconectar de la base de datos
   SQLFreeHandle(SQL_HANDLE_DBC, hdbc);        // Liberar el manejador de conexión
   SQLFreeHandle(SQL_HANDLE_ENV, henv);        // Liberar el manejador del ambiente

   return 0;
}