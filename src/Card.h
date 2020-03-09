#pragma once
#ifndef _CARD_H_
#define _CARD_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "MatrixStack.h"
#include "Shape.h"
#include "Multishape.h"

enum card_state
{
    CARD_HAND,
    CARD_THROWN,
    CARD_STUCK,
    CARD_SLASH
};

enum card_img_id
{
    STRIKE_ID,
    POMMEL_ID,
    DEFEND_ID
};

class Program;

class Card
{
public:
	Card(int id);
	virtual ~Card();
    void drawHandCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes, int handidx, int handsize, double phi, double theta, glm::vec3 pos, glm::vec3 viewdir, float voffset);
    void drawThrownCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes);
    void drawSlashingCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes, double curtheta);
    std::shared_ptr<Card> throwCard(glm::vec3 campos, glm::vec3 camview, double curtheta);
    void makeStuck();
    void drawStuckCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes);
    
    int state = CARD_HAND;
    
    // Matter if thrown or in hand
    int card_id = STRIKE_ID;
    int cost = 1;
    int damage = 0;
    int block = 0;
    int draw = 0;
    bool throwable = true;

    // Only matter if thrown
    glm::vec3 pos = glm::vec3(0,0,0);
    glm::vec3 velocity = glm::vec3(0,0,0);
    glm::vec3 throwdir = glm::vec3(0,0,0);
    double theta = 0;
    double curtime = 0;
    int slashFrame = 1;

};


#endif
