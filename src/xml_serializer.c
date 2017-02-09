#include "xml_parser.h"

int level = 0;
int tab_size = 4;

int isWhiteSpace(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

char* trim(char* str) {
    if (!str) return NULL;

    int i = 0;
    while (str[i] && isWhiteSpace(str[i])) i++;
    if (!str[i]) return NULL;

    char* new_str = &str[i];

    int last_letter = i;
    while (str[i]) {
        if (!isWhiteSpace(str[i])) last_letter = i;
        i++;
    }

    str[last_letter + 1] = '\0';
    return new_str;
}

void print_attributes(attribute_t* a, char* xml, int* i) {
    if (!a) return;

    xml[*i] = ' ';
    (*i)++;

    if (a -> key) {
        strncpy(&xml[*i], a -> key, strlen(a -> key));
        *i += strlen(a -> key);
    }

    xml[*i] = '=';
    (*i)++;

    xml[*i] = '"';
    (*i)++;   

    if (a -> value) {
        strncpy(&xml[*i], a -> value, strlen(a -> value));
        *i += strlen(a -> value);
    }

    xml[*i] = '"';
    (*i)++;   
}

char* serialized(element_t* root, char* xml, int* i, int size) {

    int spaces;

    if (!xml) {
        size = 512;
        xml = malloc(size);
    }

    if (*i >= size/2) {
        size *= 2;
        xml = realloc(xml, size);
    }

    spaces = 0;
    if (level) {
        xml[*i] = '\n';
        (*i)++;
    }
    while (spaces < level * tab_size) {
        xml[*i] = ' ';
        (*i)++;

        spaces++;
    }

    xml[*i] = '<';
    (*i)++;

    if (root -> tag) {
        strncpy(&xml[*i], root -> tag, strlen(root -> tag));
        *i += strlen(root -> tag);
    }

    print_attributes(root -> attributes, xml, i);

    xml[*i] = '>';
    (*i)++;

    level++;
    node_t* n = root -> nodes;

    while (n) {
        xml = serialized(n -> element, xml, i, size);
        n = n -> sibling;
    }

    if (root -> text) {
        root -> text = trim(root -> text);
    }

    level--;

    if (root -> text) {
        strncpy(&xml[*i], root -> text, strlen(root -> text));
        *i += strlen(root -> text);
    } else {
        spaces = 0;
        xml[*i] = '\n';
        (*i)++;
        while (spaces < level * tab_size) {
            xml[*i] = ' ';
            (*i)++;

            spaces++;
        }
    }

    xml[*i] = '<';
    (*i)++;

    xml[*i] = '/';
    (*i)++;

    if (root -> tag) {
        strncpy(&xml[*i], root -> tag, strlen(root -> tag));
        *i += strlen(root -> tag);
    }

    xml[*i] = '>';
    (*i)++;

    return xml;
}

char* serialize_xml(element_t* root) {
    int i = 0;
    char* xml = serialized(root, 0, &i, 0);
    xml[i] = '\0';
    return xml;
}