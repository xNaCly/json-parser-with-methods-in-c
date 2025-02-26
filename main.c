#include "json.h"
#include <stdio.h>
#include <stdlib.h>

void print_json_value(struct json_value *json_value) {
  switch (json_value->type) {
  case json_null:
    printf("null");
    break;
  case json_number:
    printf("%f", json_value->value.number);
    break;
  case json_string:
    printf("\"%s\"", json_value->value.string);
    break;
  case json_boolean:
    printf(json_value->value.boolean ? "true" : "false");
    break;
  case json_object:
    printf("{");
    for (size_t i = 0; i < json_value->length; i++) {
      printf("\"%s\": ", json_value->object_keys[i]);
      print_json_value(&json_value->values[i]);
      if (i < json_value->length - 1) {
        printf(", ");
      }
    }
    printf("}");
    break;
  case json_array:
    printf("[");
    for (size_t i = 0; i < json_value->length; i++) {
      print_json_value(&json_value->values[i]);
      if (i < json_value->length - 1) {
        printf(", ");
      }
    }
    printf("]");
    break;
  default:
    ASSERT(0, "Unimplemented json_value case");
    break;
  }
}

int main(void) {
  // TODO:
  // struct json json = json_new(JSON({
  //   "object" : {},
  //   "array" : [[]],
  //   "atoms" : [ "string", 0.1, true, false, null ],
  // }));
  struct json json = json_new(JSON("i am a string"));
  struct json_value json_value = json.parse(&json);
  print_json_value(&json_value);
  puts("");
  json_free_value(&json_value);
  return EXIT_SUCCESS;
}
