#pragma once
#ifndef _SENTRY_H_
#define _SENTRY_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "MatrixStack.h"
#include "Shape.h"
#include "Multishape.h"

enum sentry_state
{
    SENTRY_ALIVE,
	SENTRY_DYING,
	SENTRY_DEAD
};

class Program;

class Sentry
{
public:
	Sentry();
	virtual ~Sentry();
	glm::vec3 pos;
	glm::vec3 hitboxpos;
    void drawSentry(std::shared_ptr<Program> prog, std::vector<std::shared_ptr<Multishape>> meshes);
	const float rotateDuration = 3;
	float rotateDelay = 10;
	float lastRotate = 3;
	float rotateOffset = 0;
	bool canFire = true;
	float top = 1;
	float bottom = 0;
	int health = 40;
	int state = SENTRY_ALIVE;
	bool charging = false;
    bool firing = false;
	float accel = 0;
	glm::vec3 top_vel = glm::vec3(0);
	glm::vec3 mid_vel = glm::vec3(0);
	glm::vec3 bot_vel = glm::vec3(0);
	glm::vec3 top_pos_offset = glm::vec3(0);
	glm::vec3 mid_pos_offset = glm::vec3(0);
	glm::vec3 bot_pos_offset = glm::vec3(0);
	float timeofdeath = -1;
	float timesincedeath = 0;

};


#endif
