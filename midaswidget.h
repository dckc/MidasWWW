
struct MidasWidget {
    Widget               Widget;        /* This widget */
    Widget               Map;           /* Widget in map */
    struct MidasWidget  *Parent; 
    struct MidasWidget  *HashChain;
    Boolean              ShowChildren;  /* True if children in map */
    List                *IngotList;
    ListItem            *NameEntry; 
    Boolean              Inited;
};

typedef struct MidasWidget MidasWidget;
