struct ConvertBlock {

    Boolean (*ConvertRtn)();
    MidasType FromType;
    MidasType ToType;
};
                    
typedef struct ConvertBlock ConvertBlock;
