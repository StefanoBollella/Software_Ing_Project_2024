#include "utils.h"

//verifica se una stringa è un intero positivo
bool isPositiveInteger(const char value[]){
    int length = std::strlen(value);

    //Verifica che il valore non sia vuoto
    if(length == 0){
        return false;
    }
    //Controlla che ogni carattere sia una cifra
    for(int i = 0; i < length; ++i){
        if(!std::isdigit(value[i])){
            return false;
        }
    }
 return true;
}