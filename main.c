#include <stdbool.h>
#include <stdlib.h>

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
    long double number;
  } atom_value;
  struct json_value *array_childs;
  struct json_value *object_keys;
  struct json_value *object_values;
  // length is filled for json_type=json_array|json_object
  long length;
};

struct json {
  char (*cur)(struct json *json);
  bool (*is_eof)(struct json *json);
  struct json_value (*atom)(struct json *json);
  struct json_value (*object)(struct json *json);
  struct json_value (*array)(struct json *json);
  struct json_value (*next)(struct json *json);
};

int main(void) {
  struct json_value val =
      (struct json_value){.type = json_number, .atom_value.number = 1024.0};
  return EXIT_SUCCESS;
}
