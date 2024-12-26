#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <json-c/json.h>

// Definición de la estructura del estudiante
typedef struct {
    char nombre[50];
    char primer_apellido[50];
    char segundo_apellido[50];
    char matricula[20];
    char carrera[50];
    int num_materias;
    char materias[10][50];
    float calificaciones[10];
} Estudiante;

// Función para crear un mensaje JSON desde la estructura
char *crearMensaje(Estudiante *estudiante) {
    struct json_object *json = json_object_new_object();

    json_object_object_add(json, "nombre", json_object_new_string(estudiante->nombre));
    json_object_object_add(json, "primer_apellido", json_object_new_string(estudiante->primer_apellido));
    json_object_object_add(json, "segundo_apellido", json_object_new_string(estudiante->segundo_apellido));
    json_object_object_add(json, "matricula", json_object_new_string(estudiante->matricula));
    json_object_object_add(json, "carrera", json_object_new_string(estudiante->carrera));
    json_object_object_add(json, "num_materias", json_object_new_int(estudiante->num_materias));


    struct json_object *materias_array = json_object_new_array();
    struct json_object *calificaciones_array = json_object_new_array();

    for (int i = 0; i < estudiante->num_materias; i++) {
        json_object_array_add(materias_array, json_object_new_string(estudiante->materias[i]));
        json_object_array_add(calificaciones_array, json_object_new_double(estudiante->calificaciones[i]));
    }

    json_object_object_add(json, "materias", materias_array);
    json_object_object_add(json, "calificaciones", calificaciones_array);

    const char *json_str = json_object_to_json_string(json);
    char *resultado = strdup(json_str); // Copia el string para retornarlo
    json_object_put(json);             // Libera el objeto JSON
    return resultado;
}

// Publicar el mensaje en Mosquitto
void publicarMensaje(struct mosquitto *mosq, const char *topic, const char *mensaje) {
    int ret = mosquitto_publish(mosq, NULL, topic, strlen(mensaje), mensaje, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error publicando mensaje: %s\n", mosquitto_strerror(ret));
    }
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        fprintf(stderr, "Uso: %s <nombre> <primer_apellido> <segundo_apellido> <matricula> <carrera> <num_materias> [<materia1> <calificacion1> ...]\n", argv[0]);
        return 1;
    }

    Estudiante estudiante;
    strncpy(estudiante.nombre, argv[1], sizeof(estudiante.nombre) - 1);
    strncpy(estudiante.primer_apellido, argv[2], sizeof(estudiante.primer_apellido) - 1);
    strncpy(estudiante.segundo_apellido, argv[3], sizeof(estudiante.segundo_apellido) - 1);
    strncpy(estudiante.matricula, argv[4], sizeof(estudiante.matricula) - 1);
    strncpy(estudiante.carrera, argv[5], sizeof(estudiante.carrera) - 1);
    estudiante.num_materias = atoi(argv[6]);

    if (argc != 7 + 2 * estudiante.num_materias) {
        fprintf(stderr, "Número incorrecto de argumentos para las materias y calificaciones\n");
        return 1;
    }

    for (int i = 0; i < estudiante.num_materias; i++) {
        strncpy(estudiante.materias[i], argv[7 + 2 * i], sizeof(estudiante.materias[i]) - 1);
        estudiante.calificaciones[i] = atof(argv[8 + 2 * i]);
    }

    char *mensaje = crearMensaje(&estudiante);

    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error inicializando Mosquitto\n");
        free(mensaje);
        return 1;
    }

    if (mosquitto_connect(mosq, "localhost", 1883, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error conectando al broker Mosquitto\n");
        free(mensaje);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    publicarMensaje(mosq, "estudiantes", mensaje);

    free(mensaje);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
