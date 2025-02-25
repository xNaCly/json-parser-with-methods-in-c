#include "json.h"
#include <stdio.h>
#include <stdlib.h>

void print_json_value(struct json_value *json_value) {
  printf("jsoninc: ");
  switch (json_value->type) {
  case json_null:
    puts("null");
    break;
  case json_number:
    printf("%f\n", json_value->atom_value.number);
    break;
  case json_string:
    puts(json_value->atom_value.string);
    break;
  case json_boolean:
    puts(json_value->atom_value.boolean ? "true" : "false");
    break;
  case json_object:
  case json_array:
  default:
    ASSERT(0, "Unimplemented json_value case");
    break;
  }
}

#define R(string) #string

int main(void) {
  struct json json = json_new(R("helloWorldIAmAString"));
  struct json_value json_value = json.next(&json);
  print_json_value(&json_value);
  json_free_value(&json_value);
  return EXIT_SUCCESS;
}
