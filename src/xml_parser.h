#ifndef XML_PARSER
#define XML_PARSER

typedef struct e element_t;
typedef struct n node_t;
typedef struct a attribute_t;

element_t* get_element(char*, int*);
char* get_tag(char*, int*);
attribute_t* get_attribute(char*, int*);

struct n {
    element_t* element;
    node_t* sibling;
};

struct a {
    char* key;
    char* value;
    attribute_t* sibling;
};

struct e {
    char* tag;
    attribute_t* attributes;
    node_t* nodes;
    char* text;
};

#endif