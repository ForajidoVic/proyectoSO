#include <mariadb/mysql.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>

// Conexión a la base de datos
MYSQL *conectar_bd(const char *host, const char *user, const char *password, const char *database) {
    MYSQL *conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "Error inicializando MYSQL: %s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, host, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "Error conectando a la base de datos: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }

    printf("Conexión exitosa a la base de datos '%s'\n", database);
    return conn;
}

// Escribir datos en la base de datos a partir de un JSON
void escribir_bd(const char *json_str) {
    MYSQL *conn = conectar_bd("localhost", "root", "tesoem", "escuela");

    // Parsear el JSON recibido
    struct json_object *json_obj = json_tokener_parse(json_str);
    if (!json_obj) {
        fprintf(stderr, "Error parseando el JSON\n");
        mysql_close(conn);
        return;
    }

    // Extraer campos del JSON
    const char *nombre = json_object_get_string(json_object_object_get(json_obj, "nombre"));
    const char *primer_apellido = json_object_get_string(json_object_object_get(json_obj, "primer_apellido"));
    const char *segundo_apellido = json_object_get_string(json_object_object_get(json_obj, "segundo_apellido"));
    const char *matricula = json_object_get_string(json_object_object_get(json_obj, "matricula"));
    const char *carrera = json_object_get_string(json_object_object_get(json_obj, "carrera"));
    int num_materias = json_object_get_int(json_object_object_get(json_obj, "num_materias"));
    struct json_object *materias = json_object_object_get(json_obj, "materias");
    struct json_object *calificaciones = json_object_object_get(json_obj, "calificaciones");

    if (!nombre || !primer_apellido || !segundo_apellido || !matricula || !carrera || !materias || !calificaciones) {
        fprintf(stderr, "Datos incompletos en el JSON\n");
        json_object_put(json_obj);
        mysql_close(conn);
        return;
    }

    // Crear la consulta SQL
    char query[4096];
    snprintf(query, sizeof(query),
             "INSERT INTO estudiantes (nombre, primer_apellido, segundo_apellido, matricula, carrera, num_materias, materias, calificaciones) "
             "VALUES ('%s', '%s', '%s', '%s', '%s', %d, '%s', '%s')",
             nombre, primer_apellido, segundo_apellido, matricula, carrera, num_materias,
             json_object_to_json_string(materias),
             json_object_to_json_string(calificaciones));

    // Ejecutar la consulta
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error ejecutando consulta: %s\n", mysql_error(conn));
    } else {
        printf("Datos insertados correctamente\n");
    }

    // Liberar memoria
    json_object_put(json_obj);
    mysql_close(conn);
}
