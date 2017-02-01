#include <stdio.h>
#include <stdlib.h>

element_t* get_element(char*, int*);


typedef struct {
    element_t element;
    node_t* sibling;
} node_t;

typedef struct {
    char* key;
    char* value;
    attribute_t* sibling;
} attribute_t;

typedef struct {
    char* tag;
    attribute_t* attributes;
    node_t* nodes;
    char* text;
} element_t;


int is_whitespace(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

char* string_concat(char* str, char* start, char* stop) {
    if (start > stop) return NULL;

    int i = 0;
    while (str[i]) i++;
    char* string = malloc(i + stop - start + 1);

    int j = 0;
    while (j < i) {
        string[j] = str[j];
        j++;
    }

    while (j - i < stop - start) {
        string[j] = start[j - i];
        j++;
    }

    string[j] = '\0';

    free(str);
    return string;
}

element_t* parse_xml(char* xml-string) {

    if (!xml-string) return NULL;

    int i = 0;
    while (xml-string[i]) {
        if (!(xml-string[i+1])) return NULL;
        if (xml-string[i] == '<') {
            if (xml-string[i+1] == '!' || xml-string[i+1] == '?') {
                i++;
            } else {
                return get_element(&xml-string[i], 0);
            }
        }
    }
}

element_t* get_element(char* xml, int* last_index) {

    if (xml[0] != '<') {
        printf("Get element needs to start at opening tag\n")
        exit(1);
    }

    element_t* e = malloc(sizeof(element_t));

    int inside_tag = 1;
    attribute_t* at_p = NULL;
    element_t* e_p = NULL;

    int j, i = 0;
    while (xml[i]) {
        if (inside_tag) {
            if (is_whitespace(xml[i])) {
                i++;
                continue;
            } else

            if (xml[i] == '<') {
                e -> tag = get_tag(&xml[i], &i);
            } else

            if (xml[i] == '>') {
                i++;
                inside_tag = 0;
            } else

            if (xml[i] == '/') {
                e -> text = NULL;
                e -> nodes = NULL;
                if (last_index) *last_index = i + 2;
                return e;
            } else {
                at_p = e -> attributes;
                e -> attributes = get_attribute(&xml[i], &i);
                e -> attributes -> sibling = at_p;
            }

        } else {
            
            if (xml[i] == '<') {
                if (xml[i+1] == '/') {
                    j = i;
                    while (e -> tag[j]) j++;
                    if (!strncmp(e.tag, &xml[i+2], j)) {
                        i += j + 2;
                        if (last_index) *last_index = i;
                        return e;
                    } else {
                        printf("Illegal overlap of tags\nOpen tag: %s\nUnexpected closing tag: %s\n", e.tag, string_concat(NULL, &xml[i+2], &xml[i+2+j]));
                        exit(2);
                    }
                } else {
                    e_p = e -> nodes;
                    e -> nodes = get_element(&xml[i], &i);
                    e -> nodes -> sibling = e_p;
                }
            } else {
                j = i;
                while (xml[j] != '<') j++;
                e -> text = string_concat(e -> text, &xml[i], &xml[j]);
            }
        }
    }
}

char* get_tag(char* xml, int* j) {
    int i = 0;
    while (xml[i] != ' ' && xml[i] != '>') i++;

    char* tag = malloc(i+1);
    memcpy(tag, xml, i);
    tag[i] = 0;

    *j = &xml[i];
    return tag;
}

char* get_attribute(char* xml, int* j) {

    attribute_t at = malloc(sizeof(attribute_t));

    int i = 0;
    while (xml[i] != '=') i++;

    at -> key = string_concat(NULL, xml, &xml[i]);

    while (xml[i] != '"') i++;
    i++;

    int k = i;
    while (xml[k] != '"') k++;

    at -> value = string_concat(NULL, &xml[i], &xml[k])

    *j = &xml[k + 1];
    return at;
}