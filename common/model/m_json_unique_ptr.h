#ifndef CLT_OPTIC_M_JSON_UNIQUE_PTR_H
#define CLT_OPTIC_M_JSON_UNIQUE_PTR_H
#include <memory>
#include <jansson.h>

template <typename T> using deleted_unique_ptr = std::unique_ptr<T, void (*)(T*)>;
using json_unique_ptr = deleted_unique_ptr<json_t>;
inline json_unique_ptr json_unique_ptr_create(json_t* value)
{
    return {value, json_decref};
}
#endif //CLT_OPTIC_M_JSON_UNIQUE_PTR_H
