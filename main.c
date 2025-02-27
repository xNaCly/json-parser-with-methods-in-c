#include "json.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  struct json json = json_new(JSON({
    "object" : {},
    "array" : [[]],
    "atoms" : [ "string", 0.1, true, false, null ]
  }));
  struct json_value json_value = json.parse(&json);
  json_print_value(&json_value);
  puts("");
  json_free_value(&json_value);
  return EXIT_SUCCESS;
}
