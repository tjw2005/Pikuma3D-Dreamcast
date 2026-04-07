#include "vector.h"
#include <math.h>

vec3_t vec3_rotate_x(vec3_t v, float angle) {
    vec3_t rotated_vector;
    rotated_vector.x = v.x;
    rotated_vector.y = v.y * cos(angle) - v.z * sin(angle);
    rotated_vector.z = v.y * sin(angle) + v.z * cos(angle);
    return rotated_vector;
}

vec3_t vec3_rotate_y(vec3_t v, float angle) {
    vec3_t rotated_vector;
    rotated_vector.x = v.x * cos(angle) + v.z * sin(angle);
    rotated_vector.y = v.y;
    rotated_vector.z = -v.x * sin(angle) + v.z * cos(angle);
    return rotated_vector;
}

vec3_t vec3_rotate_z(vec3_t v, float angle) {
    vec3_t rotated_vector;
    rotated_vector.x = v.x * cos(angle) - v.y * sin(angle);
    rotated_vector.y = v.x * sin(angle) + v.y * cos(angle);
    rotated_vector.z = v.z;
    return rotated_vector;
}
