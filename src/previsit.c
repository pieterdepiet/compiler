#include "include/previsit.h"
#include "include/errors.h"

AST_T* previsit_compound(AST_T* compound);
AST_T* previsit_binop(AST_T* binop);

AST_T* previsit(AST_T* node) {
    if (node == (void*) 0) {
        return node;
    }
    printf("Previsit %s\n", ast_node_type_string(node));
    switch (node->type) {
        case AST_COMPOUND: return previsit_compound(node); break;
        case AST_BINOP: return previsit_binop(node); break;
        case AST_VARIABLE_DEFINITION: {
            node->variable_definition_value = previsit(node->variable_definition_value);
        }; break;
        case AST_FUNCTION_DEFINITION: {
            node->function_definition_body = previsit(node->function_definition_body);
        }; break;
        default: break;
    }
    return node;
}
AST_T* previsit_compound(AST_T* compound) {
    for (size_t i = 0; i < compound->compound_size; i++) {
        compound->compound_value[i] = previsit(compound->compound_value[i]);
    }
    return compound;
}

AST_T* previsit_binop(AST_T* binop) {
    binop->left_hand = previsit(binop->left_hand);
    binop->right_hand = previsit(binop->right_hand);
    if (binop->left_hand->type == AST_INT) {
        if (binop->right_hand->type == AST_INT) {
            binop->type = AST_INT;
            if (binop->binop_type == BINOP_PLUS) {
                binop->int_value = binop->left_hand->int_value + binop->right_hand->int_value;
            }
        }
    }
    return binop;
}