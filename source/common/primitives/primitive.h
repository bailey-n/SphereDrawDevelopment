//
// Created by Nathan on 2/5/2026.
//

#ifndef SPHEREDRAW_PRIMITIVE_H
#define SPHEREDRAW_PRIMITIVE_H

#include <string>

class Primitive {
    unsigned int id;
public:
    [[nodiscard]] unsigned int get_id() const { return id; };
    [[nodiscard]] virtual unsigned int get_type() const = 0;
    [[nodiscard]] virtual std::string get_type_string() const = 0;
};


#endif //SPHEREDRAW_PRIMITIVE_H
