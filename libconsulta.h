#ifndef LIBCONSULTA_H
#define LIBCONSULTA_H

void conectar_bd(const char *host, const char *user, const char *password, const char *database);
void escribir_bd(const char *json);

#endif // LIBCONSULTA_H
