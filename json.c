#include "json.h"
#include <stdlib.h>
#include <string.h>

char *json_type_map[] = {
    [json_number] = "json_number",   [json_string] = "json_string",
    [json_boolean] = "json_boolean", [json_null] = "json_null",
    [json_object] = "json_object",   [json_array] = "json_array",
};

void json_print_value(struct json_value *json_value) {
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
      json_print_value(&json_value->values[i]);
      if (i < json_value->length - 1) {
        printf(", ");
      }
    }
    printf("}");
    break;
  case json_array:
    printf("[");
    for (size_t i = 0; i < json_value->length; i++) {
      json_print_value(&json_value->values[i]);
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

void json_free_value(struct json_value *json_value) {
  switch (json_value->type) {
  case json_string:
    free(json_value->value.string);
    break;
  case json_object:
    for (size_t i = 0; i < json_value->length; i++) {
      free(json_value->object_keys[i]);
      json_free_value(&json_value->values[i]);
    }
    if (json_value->object_keys != NULL) {
      free(json_value->object_keys);
      json_value->object_keys = NULL;
    }
    if (json_value->values != NULL) {
      free(json_value->values);
      json_value->values = NULL;
    }
    break;
  case json_array:
    for (size_t i = 0; i < json_value->length; i++) {
      json_free_value(&json_value->values[i]);
    }
    if (json_value->values != NULL) {
      free(json_value->values);
      json_value->values = NULL;
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

static void skip_whitespace(struct json *json) {
  while (!json->is_eof(json) &&
         (json->cur(json) == ' ' || json->cur(json) == '\t' ||
          json->cur(json) == '\n')) {
    json->pos++;
  }
}

static char cur(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  return json->is_eof(json) ? -1 : json->input[json->pos];
}

static bool is_eof(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  return json->pos > json->length;
}

static void advance(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  json->pos++;
  skip_whitespace(json);
}

static struct json_value number(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  size_t start = json->pos;
  // i dont give a fuck about scientific notation <3
  for (char cc = json->cur(json);
       ((cc >= '0' && cc <= '9') || cc == '_' || cc == '.' || cc == '-');
       json->advance(json), cc = json->cur(json))
    ;

  char *slice = malloc(sizeof(char) * json->pos - start + 1);
  ASSERT(slice != NULL, "failed to allocate slice for number parsing")
  memcpy(slice, json->input + start, json->pos - start);
  slice[json->pos - start] = 0;
  double number = strtod(slice, NULL);
  free(slice);

  return (struct json_value){.type = json_number, .value = {.number = number}};
}

static char *string(struct json *json) {
  json->advance(json);
  size_t start = json->pos;
  for (char cc = json->cur(json); cc != '\n' && cc != '"';
       json->advance(json), cc = json->cur(json))
    ;

  char *slice = malloc(sizeof(char) * json->pos - start + 1);
  ASSERT(slice != NULL, "failed to allocate slice for a string")

  memcpy(slice, json->input + start, json->pos - start);
  slice[json->pos - start] = 0;

  ASSERT(json->cur(json) == '"', "unterminated string");
  json->advance(json);
  return slice;
}

static struct json_value atom(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");

  skip_whitespace(json);

  char cc = json->cur(json);
  if ((cc >= '0' && cc <= '9') || cc == '.' || cc == '-') {
    return number(json);
  }

  switch (cc) {
  case 'n': // null
    json->pos++;
    ASSERT(json->cur(json) == 'u', "unknown atom 'n', wanted 'null'")
    json->pos++;
    ASSERT(json->cur(json) == 'l', "unknown atom 'nu', wanted 'null'")
    json->pos++;
    ASSERT(json->cur(json) == 'l', "unknown atom 'nul', wanted 'null'")
    json->advance(json);
    return (struct json_value){.type = json_null};
  case 't': // true
    json->pos++;
    ASSERT(json->cur(json) == 'r', "unknown atom 't', wanted 'true'")
    json->pos++;
    ASSERT(json->cur(json) == 'u', "unknown atom 'tr', wanted 'true'")
    json->pos++;
    ASSERT(json->cur(json) == 'e', "unknown atom 'tru', wanted 'true'")
    json->advance(json);
    return (struct json_value){.type = json_boolean,
                               .value = {.boolean = true}};
  case 'f': // false
    json->pos++;
    ASSERT(json->cur(json) == 'a', "invalid atom 'f', wanted 'false'")
    json->pos++;
    ASSERT(json->cur(json) == 'l', "invalid atom 'fa', wanted 'false'")
    json->pos++;
    ASSERT(json->cur(json) == 's', "invalid atom 'fal', wanted 'false'")
    json->pos++;
    ASSERT(json->cur(json) == 'e', "invalid atom 'fals', wanted 'false'")
    json->advance(json);
    return (struct json_value){.type = json_boolean,
                               .value = {.boolean = false}};
  case '"':
    return (struct json_value){.type = json_string,
                               .value = {.string = string(json)}};
  default:
    printf("unknown character '%c' at pos %zu\n", json->cur(json), json->pos);
    ASSERT(false, "unknown character");
    return (struct json_value){.type = json_null};
  }
}

struct json_value array(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  ASSERT(json->cur(json) == '[', "invalid array start");
  json->advance(json);

  struct json_value json_value = {.type = json_array};
  json_value.values = malloc(sizeof(struct json_value));

  while (!json->is_eof(json) && json->cur(json) != ']') {
    if (json_value.length > 0) {
      if (json->cur(json) != ',') {
        json_free_value(&json_value);
      }
      ASSERT(json->cur(json) == ',',
             "expected , as the separator between array members");
      json->advance(json);
    }
    struct json_value member = json->parse(json);
    json_value.values = realloc(json_value.values,
                                sizeof(json_value) * (json_value.length + 1));
    json_value.values[json_value.length++] = member;
  }

  ASSERT(json->cur(json) == ']', "missing array end");
  json->advance(json);
  return json_value;
}

struct json_value object(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  ASSERT(json->cur(json) == '{', "invalid object start");
  json->advance(json);

  struct json_value json_value = {.type = json_object};
  json_value.object_keys = malloc(sizeof(char *));
  json_value.values = malloc(sizeof(struct json_value));

  while (!json->is_eof(json) && json->cur(json) != '}') {
    if (json_value.length > 0) {
      if (json->cur(json) != ',') {
        json_free_value(&json_value);
      }
      ASSERT(json->cur(json) == ',',
             "expected , as separator between object key value pairs");
      json->advance(json);
    }
    ASSERT(json->cur(json) == '"',
           "expected a string as the object key, did not get that")
    char *key = string(json);
    ASSERT(json->cur(json) == ':', "expected object key and value separator");
    json->advance(json);

    struct json_value member = json->parse(json);
    json_value.values = realloc(json_value.values, sizeof(struct json_value) *
                                                       (json_value.length + 1));
    json_value.values[json_value.length] = member;
    json_value.object_keys = realloc(json_value.object_keys,
                                     sizeof(char **) * (json_value.length + 1));
    json_value.object_keys[json_value.length] = key;
    json_value.length++;
  }

  ASSERT(json->cur(json) == '}', "missing object end");
  json->advance(json);
  return json_value;
}

struct json_value parse(struct json *json) {
  ASSERT(json != NULL, "corrupted internal state");
  skip_whitespace(json);
  ASSERT(!json->is_eof(json), "unexpected end of input");

  struct json_value res;
  switch (json->cur(json)) {
  case '{':
    res = json->object(json);
    break;
  case '[':
    res = json->array(json);
    break;
  default:
    res = json->atom(json);
    break;
  }

  return res;
}

struct json json_new(char *input) {
  ASSERT(input != NULL, "corrupted input");
  struct json j = (struct json){
      .input = input,
      .length = strlen(input) - 1,
  };

  j.cur = cur;
  j.is_eof = is_eof;
  j.advance = advance;
  j.parse = parse;
  j.object = object;
  j.array = array;
  j.atom = atom;

  return j;
}
