// Directivas necesarias para la conexión ODBC a SQL Server
#include <iostream>     // Directiva de entrada/salida estándar
#include <windows.h>    // Incluye funciones del sistema Windows
#include <sql.h>       // Incluye funciones core de ODBC
#include <sqlext.h>    // Incluye funciones extendidas de ODBC
#include <string>      // Manejo de cadenas de texto
#include <vector>      // Para almacenar los nombres de columnas
#include <iomanip>     // Para formateo de salida

using namespace std; 

// Función para ejecutar consulta y mostrar resultados
// Parámetros:
// - hdbc: Manejador de la conexión a la base de datos
// - consulta: Consulta SQL a ejecutar en formato wide char
// - nombreTabla: Nombre de la tabla para mostrar en los resultados
static void ejecutarConsulta(SQLHDBC hdbc, const wchar_t* consulta, const char* nombreTabla) {
    SQLHSTMT hstmt;    // Manejador para la sentencia SQL
    SQLRETURN ret;     // Variable para almacenar resultados de operaciones ODBC

    // Crear manejador para la consulta - necesario para ejecutar sentencias SQL
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        cout << "Error al crear el handle de la consulta" << endl;
        return;
    }

    // Ejecutar la consulta SQL directamente usando SQLExecDirectW para soporte Unicode
    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)consulta, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        cout << "Error al ejecutar la consulta para " << nombreTabla << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nDatos de la tabla " << nombreTabla << ":\n" << endl;

    // Obtener el número total de columnas en el resultado de la consulta
    SQLSMALLINT numCols;
    SQLNumResultCols(hstmt, &numCols);

    // Crear vectores para almacenar información de las columnas
    vector<wstring> columnNames(numCols);      // Almacena nombres de columnas
	vector<SQLLEN> columnSizes(numCols);       // Almacena anchos de columnas

    // Obtener metadatos de cada columna (nombre y tamaño para formato)
    for (SQLSMALLINT i = 1; i <= numCols; i++) {
        SQLWCHAR columnName[128];              // Buffer para nombre de columna
        SQLSMALLINT columnNameLength;          // Longitud del nombre
        SQLLEN columnSize;                     // Tamaño de la columna 

        // Obtener nombre de la columna
        SQLColAttributeW(hstmt, i, SQL_DESC_NAME, columnName, sizeof(columnName), &columnNameLength, NULL);
        // Obtener ancho de visualización de la columna
        SQLColAttributeW(hstmt, i, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &columnSize);

        // Guardar nombre y calcular ancho máximo entre nombre y contenido
        columnNames[i-1] = columnName;
        columnSizes[i-1] = max((SQLLEN)columnNames[i-1].length(), columnSize);
    }

    // Imprimir línea superior de la tabla
    for (SQLSMALLINT i = 0; i < numCols; i++) {
        wcout << wstring(columnSizes[i] + 2, L'-') << L"+";
    }
    wcout << endl;

    // Imprimir encabezados de columnas
    for (SQLSMALLINT i = 0; i < numCols; i++) {
        wcout << L" " << left << setw(columnSizes[i]) << columnNames[i] << L" |";
    }
    wcout << endl;

    // Imprimir línea separadora después de encabezados
    for (SQLSMALLINT i = 0; i < numCols; i++) {
        wcout << wstring(columnSizes[i] + 2, L'-') << L"+";
    }
    wcout << endl;

    // Recuperar y mostrar cada fila de resultados
    while (SQL_SUCCESS == SQLFetch(hstmt)) {
        for (SQLSMALLINT i = 1; i <= numCols; i++) {
            SQLWCHAR buffer[512];              // Buffer para datos de columna
            SQLLEN indicator;                  // Indicador de nulo/longitud
            
            // Obtener dato de la columna actual
            ret = SQLGetData(hstmt, i, SQL_C_WCHAR, buffer, sizeof(buffer), &indicator);
            if (SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret) {
                wcout << L" ";
                // Manejar valores NULL y mostrar datos con formato
                if (indicator == SQL_NULL_DATA)
                    wcout << left << setw(columnSizes[i-1]) << L"NULL";
                else
                    wcout << left << setw(columnSizes[i-1]) << buffer;
                wcout << L" |";
            }
        }
        wcout << endl;
    }

    // Imprimir línea inferior de la tabla
    for (SQLSMALLINT i = 0; i < numCols; i++) {
        wcout << wstring(columnSizes[i] + 2, L'-') << L"+";
    }
    wcout << endl;

    // Liberar el manejador de la sentencia SQL
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}


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

   // Consulta para el reporte personalizado
   const wchar_t* consultaReporte = L"SELECT "
       L"E.Empleado_id, "
       L"E.Nombre + ' ' + E.Apellido_paterno + ' ' + E.Apellido_materno as NombreCompleto, "
       L"E.Fecha_nacimiento, "
       L"E.RFC, "
       L"CC.Nombre_centro, "
       L"CP.Nombre_puesto, "
       L"CASE WHEN D.Empleado_id IS NOT NULL THEN 'SI' ELSE 'NO' END as EsDirectivo "
       L"FROM Empleado E "
       L"LEFT JOIN Catalogo_Centro CC ON E.Centro_id = CC.Centro_id "
       L"LEFT JOIN Catalogo_Puesto CP ON E.Puesto_id = CP.Puesto_id "
       L"LEFT JOIN Directivo D ON E.Empleado_id = D.Empleado_id;";

   // Ejecutar consulta del reporte
   ejecutarConsulta(hdbc, consultaReporte, "Reporte de Empleados");

   // Paso 6: Cerrar la conexión y liberar recursos
   SQLDisconnect(hdbc);                        // Desconectar de la base de datos
   SQLFreeHandle(SQL_HANDLE_DBC, hdbc);        // Liberar el manejador de conexión
   SQLFreeHandle(SQL_HANDLE_ENV, henv);        // Liberar el manejador del ambiente

   return 0;
}