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