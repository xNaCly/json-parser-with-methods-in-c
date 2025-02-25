#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(EXP, context)                                                   \
  if (!(EXP)) {                                                                \
    fprintf(stderr,                                                            \
            "jsoninc: ASSERT(" #EXP "): `" context                             \
            "` failed at %s, line %d\n",                                       \
            __FILE__, __LINE__);                                               \
    exit(EXIT_FAILURE);                                                        \
  }

enum json_type {
  json_number,
  json_string,
  json_boolean,
  json_null,
  json_object,
  json_array,
};

struct json_value {
  enum json_type type;
  union {
    bool boolean;
    char *string;
    double number;
  } atom_value;
  struct json_value *array_childs;
  char *object_keys;
  struct json_value *object_values;
  // length is filled for json_type=json_array|json_object
  size_t length;
};

struct json {
  char *input;
  size_t pos;
  char (*cur)(struct json *json);
  bool (*is_eof)(struct json *json);
  void (*advance)(struct json *json);
  struct json_value (*atom)(struct json *json);
  struct json_value (*array)(struct json *json);
  struct json_value (*object)(struct json *json);
  struct json_value (*next)(struct json *json);
};

struct json json_new(char *input);
void json_free_value(struct json_value *json_value);

#endif
