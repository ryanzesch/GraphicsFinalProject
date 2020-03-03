#include "Multishape.h"
#include <iostream>
#include <assert.h>
#include <memory>
#include <limits.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Multishape::Multishape()
{
	min = glm::vec3(INT_MAX);
	max = glm::vec3(INT_MIN);
}

glm::vec3 Multishape::getCenter() {
	return glm::vec3((max.x+min.x)/2, (max.y+min.y)/2, (max.z+min.z)/2);
}

void Multishape::addShape(shared_ptr<Shape> s) {
    shapes.push_back(s);
    if (s->max.x > max.x) max.x = s->max.x;
    if (s->max.y > max.y) max.y = s->max.y;
    if (s->max.z > max.z) max.z = s->max.z;
    if (s->min.x < min.x) min.x = s->min.x;
    if (s->min.y < min.y) min.y = s->min.y;
    if (s->min.z < min.z) min.z = s->min.z;
}

Multishape::~Multishape()
{
}
