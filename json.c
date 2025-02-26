#include "json.h"

#include <stdlib.h>
#include <string.h>

char *json_type_map[] = {
    [json_number] = "json_number",   [json_string] = "json_string",
    [json_boolean] = "json_boolean", [json_null] = "json_null",
    [json_object] = "json_object",   [json_array] = "json_array",
};

void json_free_value(struct json_value *json_value) {
  switch (json_value->type) {
  case json_string:
    free(json_value->value.string);
    break;
  case json_object:
    for (size_t i = 0; i < json_value->length; i++) {
      free(&json_value->object_keys[i]);
      json_free_value(&json_value->values[i]);
    }
    break;
  case json_array:
    for (size_t i = 0; i < json_value->length; i++) {
      json_free_value(&json_value->values[i]);
    }
    break;
  case json_number:
  case json_boolean:
  case json_null:
  default:
    break;
  }
  json_value->type = json_null;
}

static char cur(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  char cc = json->input[json->pos];
  return cc ? cc : -1;
}

static bool is_eof(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  return json->input[json->pos] == 0;
}

static void advance(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  if (!json->is_eof(json)) {
    json->pos++;
  }
}

static struct json_value number(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  size_t start = json->pos;
  // i dont give a fuck about scientific notation <3
  for (char cc = json->cur(json);
       ((cc >= '0' && cc <= '9') || cc == '_' || cc == '.');
       json->advance(json), cc = json->cur(json))
    ;

  if (start == json->pos) {
    return (struct json_value){.type = json_number, .value = {.number = 0}};
  }

  char *slice = malloc(sizeof(char) * json->pos - start + 1);
  ASSERT(slice != NULL, "failed to allocate slice for number parsing")
  memcpy(slice, json->input + start, json->pos - start);
  slice[json->pos - start] = 0;
  double number = strtod(slice, NULL);
  free(slice);

  return (struct json_value){.type = json_number, .value = {.number = number}};
}

static struct json_value atom(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  char cc = json->cur(json);
  if ((cc >= '0' && cc <= '9') || cc == '.') {
    return number(json);
  }

  switch (cc) {
  case 'n': // null
    json->advance(json);
    ASSERT(json->cur(json) == 'u', "invalid null")
    json->advance(json);
    ASSERT(json->cur(json) == 'l', "invalid null")
    json->advance(json);
    ASSERT(json->cur(json) == 'l', "invalid null")
    return (struct json_value){.type = json_null};
  case 't': // true
    json->advance(json);
    ASSERT(json->cur(json) == 'r', "unknown atom 't', wanted 'true'")
    json->advance(json);
    ASSERT(json->cur(json) == 'u', "unknown atom 'tr', wanted 'true'")
    json->advance(json);
    ASSERT(json->cur(json) == 'e', "unknown atom 'tru', wanted 'true'")
    return (struct json_value){.type = json_boolean,
                               .value = {.boolean = true}};
  case 'f': // false
    json->advance(json);
    ASSERT(json->cur(json) == 'a', "invalid atom 'f', wanted 'false'")
    json->advance(json);
    ASSERT(json->cur(json) == 'l', "invalid atom 'fa', wanted 'false'")
    json->advance(json);
    ASSERT(json->cur(json) == 's', "invalid atom 'fal', wanted 'false'")
    json->advance(json);
    ASSERT(json->cur(json) == 'e', "invalid atom 'fals', wanted 'false'")
    return (struct json_value){.type = json_boolean,
                               .value = {.boolean = false}};
  case '"':
    json->advance(json);
    size_t start = json->pos;
    for (cc = json->cur(json); cc != '\n' && cc != '"';
         json->advance(json), cc = json->cur(json))
      ;
    char *slice = malloc(sizeof(char) * json->pos - start + 1);
    ASSERT(slice != NULL, "failed to allocate slice for a string")
    memcpy(slice, json->input + start, json->pos - start);
    slice[json->pos - start] = 0;
    ASSERT(json->cur(json) == '"', "unterminated string");
    return (struct json_value){.type = json_string, .value = {.string = slice}};
  default:
    ASSERT(false, "unknown character at this point");
    return (struct json_value){.type = json_null};
  }
}

struct json_value array(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  ASSERT(json->cur(json) == '[', "invalid array start");
  // TODO: implement this, i dont know how rn, i am toooo exhausted :(
  ASSERT(json->cur(json) == ']', "missing array end");
  return (struct json_value){.type = json_array};
}

struct json_value parse(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  ASSERT(!json->is_eof(json), "unexpected empty input");

  char cc = json->cur(json);
  while (!json->is_eof(json) && (cc == ' ' || cc == '\t' || cc == '\n')) {
    json->advance(json);
    cc = json->cur(json);
  }

  switch (cc = json->cur(json)) {
  // case '[':
  //   return json->array(json);
  // case '{':
  //   return json->object(json);
  default:
    return json->atom(json);
  }
}

struct json json_new(char *input) {
  ASSERT(input != NULL, "corrupted input");
  struct json j = (struct json){
      .input = input,
  };

  j.cur = cur;
  j.is_eof = is_eof;
  j.advance = advance;
  j.parse = parse;
  j.atom = atom;

  return j;
}
