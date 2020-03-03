#pragma once
#ifndef _MSHAPE_H_
#define _MSHAPE_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>
#include "Shape.h"

class Program;

class Multishape
{
public:
	Multishape();
	virtual ~Multishape();
    void addShape(std::shared_ptr<Shape> s);
	glm::vec3 getCenter();
	glm::vec3 min;
	glm::vec3 max;
    std::vector<std::shared_ptr<Shape>> shapes;
};


#endif
