#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml_parser.h"

int is_whitespace(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

char* string_concat(char* str, char* start, char* stop) {
    if (start > stop) return NULL;

    int length = (int) ((long unsigned int) stop - (long unsigned int) start);

    int i = 0;
    if (str) {
        while (str[i]) i++;
    }
    char* string = malloc(i + length + 1);

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

element_t* parse_xml(char* xml_string) {

    if (!xml_string) return NULL;

    int i = 0;
    while (xml_string[i]) {
        if (!(xml_string[i+1])) return NULL;
        if (xml_string[i] == '<') {
            if (xml_string[i+1] == '!' || xml_string[i+1] == '?') {
                i++;
            } else {
                return get_element(&xml_string[i], 0);
            }
        }
    }
    return NULL;
}

element_t* get_element(char* xml, int* last_index) {

    if (xml[0] != '<') {
        printf("Get element needs to start at opening tag\n");
        exit(1);
    }

    element_t* e = malloc(sizeof(element_t));
    e -> text = NULL;
    e -> nodes = NULL;

    int inside_tag = 1;
    attribute_t* at_p = NULL;
    node_t* n_p = NULL;

    int j, i = 0;
    while (xml[i]) {
        if (inside_tag) {
            if (is_whitespace(xml[i])) {
                i++;
                continue;
            } else

            if (xml[i] == '<') {
                i++;
                e -> tag = get_tag(&xml[i], &i);
            } else

            if (xml[i] == '>') {
                i++;
                inside_tag = 0;
            } else

            if (xml[i] == '/') {
                e -> text = NULL;
                e -> nodes = NULL;
                if (last_index) *last_index += i + 2;
                return e;
            } else {
                at_p = e -> attributes;
                e -> attributes = get_attribute(&xml[i], &i);
                e -> attributes -> sibling = at_p;
            }

        } else {
            
            if (xml[i] == '<') {
                if (xml[i+1] == '/') {
                    j = 0;
                    while (e -> tag[j]) j++;
                    if (!strncmp(e -> tag, &xml[i+2], j)) {
                        i += j + 3;
                        if (last_index) *last_index += i;
                        return e;
                    } else {
                        return NULL;
                        printf("Illegal overlap of tags\nOpen tag: %s\nUnexpected closing tag: %s\n", e->tag, string_concat(NULL, &xml[i+2], &xml[i+2+j]));
                        exit(2);
                    }
                } else {
                    n_p = e -> nodes;
                    e -> nodes = malloc(sizeof(node_t));
                    e -> nodes -> element = get_element(&xml[i], &i);
                    e -> nodes -> sibling = n_p;

                    if (e -> nodes -> element == NULL) return NULL;
                }
            } else {
                j = i;
                while (xml[j] && xml[j] != '<') j++;
                e -> text = string_concat(e -> text, &xml[i], &xml[j]);
                i = j;
            }
        }
    }
    return NULL;
}

char* get_tag(char* xml, int* j) {
    int i = 0;
    while (xml[i] != ' ' && xml[i] != '>' && xml[i] != '/') i++;

    char* tag = malloc(i+1);
    memcpy(tag, xml, i);
    tag[i] = '\0';

    *j += i;
    return tag;
}

attribute_t* get_attribute(char* xml, int* j) {

    attribute_t* at = malloc(sizeof(attribute_t));

    int i = 0;
    while (xml[i] != '=') i++;

    at -> key = string_concat(NULL, xml, &xml[i]);

    while (xml[i] != '"' && xml[i] != '\'') i++;
    char quote = xml[i];

    i++;

    int k = i;
    while (xml[k] != quote) k++;

    at -> value = string_concat(NULL, &xml[i], &xml[k]);

    *j += k + 1;
    return at;
}