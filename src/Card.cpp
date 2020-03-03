#include "Card.h"
#include <iostream>
#include <assert.h>
#include <memory>
#include <limits.h>
#include <vector>

#include "GLSL.h"
#include "Program.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

using namespace std;

Card::Card(int id)
{
    card_id = id;
    switch (id) {
        case STRIKE_ID:
            damage = 6;
            block = 0;
            draw = false;
            throwable = true;
            cost = 1;
        break;
        case POMMEL_ID:
            damage = 9;
            block = 0;
            draw = true;
            throwable = true;
            cost = 1;
        break;
        case DEFEND_ID:
            damage = 0;
            block = 5;
            draw = false;
            throwable = false;
            cost = 1;
        break;
    }
}

void setCardModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}

void Card::drawHandCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes, int handidx, int handsize, double phi, double theta, glm::vec3 pos, glm::vec3 viewdir, float voffset) {
    if (state != CARD_HAND) {
        cout << "uh oh - card wasn't in hand!" << endl;
        exit(1);
    }
    
    std::shared_ptr<MatrixStack> Model = make_shared<MatrixStack>();;
    Model->loadIdentity();

    // From an enum in main
    int strike = 8;

    Model->pushMatrix();
        // Move down if throwing
        Model->translate(-voffset * glm::normalize(cross(viewdir,cross(viewdir, glm::vec3(0,1,0)))));
        // Move down of center
        Model->translate((.08f + 0.005f * float(pow(abs((handsize-1)/2.0f - float(handidx)),2))) * glm::normalize(cross(viewdir,cross(viewdir, glm::vec3(0,1,0)))));
        // Move left of center
        Model->translate((-.1f + 0.05f * ((handsize-1)/2.0f - float(handidx)) )*glm::normalize(cross(viewdir, glm::vec3(0,1,0))));
        // Move in front of camera
        Model->translate(pos + normalize(viewdir) * (0.2f + float(handidx)/1000));
        // Rotate to face the camera and be stylized
        Model->rotate(phi - .05f, normalize(cross(viewdir, glm::vec3(0,1,0))));
        Model->rotate(-theta, glm::vec3(0,1,0));
        Model->rotate(0.08f * ((handsize-1)/2.0f - float(handidx)), glm::vec3(1,0,0));
        // Scale to size
        Model->scale(glm::vec3(.1/(meshes[strike]->max.y - meshes[strike]->min.y)));
        setCardModel(prog, Model);
        meshes[strike]->shapes[0]->draw(prog);
    Model->popMatrix();

}

void Card::drawThrownCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes) {
    if (state != CARD_THROWN) {
        cout << "uh oh - card wasn't thrown!" << endl;
        exit(1);
    }

    // Update card position
    pos = pos + velocity;
    velocity.y -= .0001;
    velocity -= normalize(cross(throwdir, glm::vec3(0,.1,0))) * .0003f;
    
    std::shared_ptr<MatrixStack> Model = make_shared<MatrixStack>();;
    Model->loadIdentity();

    curtime = glfwGetTime();

    // From an enum in main
    int strike = 8;

    Model->pushMatrix();
        Model->translate(pos);
        // Rotate to be perp-ish to velocity
        Model->rotate(1, velocity);
        // Rotate it to "face" the thrown direction
        Model->rotate(-theta - 3.14/2,glm::vec3(0,1,0));
        // Spin card
        Model->rotate(curtime*10, glm::vec3(1,0,0));
        Model->scale(glm::vec3(.1/(meshes[strike]->max.y - meshes[strike]->min.y)));
        setCardModel(prog, Model);
        meshes[strike]->shapes[0]->draw(prog);
    Model->popMatrix();

}

std::shared_ptr<Card> Card::throwCard(glm::vec3 campos, glm::vec3 camview, double curtheta) {
    if (!throwable) {
        cout << "uh oh - card wasn't throwable!" << endl;
        exit(1);
    }
    float random_offset1 = (rand() % 10000) / 10000000.0f;
    float random_offset2 = (rand() % 10000) / 1000000.0f;
    std::shared_ptr<Card> thrown_card = std::make_shared<Card>(card_id);
    thrown_card->state = CARD_THROWN;
    thrown_card->pos = campos - glm::vec3(0,.1,0) + normalize(cross(camview, glm::vec3(0,.1,0))) * .05f;
    // Forward velocity
    thrown_card->velocity = normalize(camview) * .05f + glm::vec3(0,.005 + random_offset1,0);
    // Horizontal velocity
    thrown_card->velocity += normalize(cross(camview, glm::vec3(0,.1,0))) * (.02f + random_offset2);
    thrown_card->throwdir = camview;
    thrown_card->theta = curtheta;
    return thrown_card;
}

void Card::drawStuckCard(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes) {
    if (state != CARD_STUCK) {
        cout << "uh oh - card wasn't stuck!" << endl;
        exit(1);
    }
    
    std::shared_ptr<MatrixStack> Model = make_shared<MatrixStack>();;
    Model->loadIdentity();

    // From an enum in main
    int strike = 8;

    Model->pushMatrix();
        Model->translate(pos);
        // Rotate to be perp-ish to velocity
        Model->rotate(1, velocity);
        // Rotate it to "face" the thrown direction
        Model->rotate(-theta - 3.14/2,glm::vec3(0,1,0));
        // Spin card
        Model->rotate(curtime*10, glm::vec3(1,0,0));
        Model->scale(glm::vec3(.1/(meshes[strike]->max.y - meshes[strike]->min.y)));
        setCardModel(prog, Model);
        meshes[strike]->shapes[0]->draw(prog);
    Model->popMatrix();

}

void Card::makeStuck() {
    if (state != CARD_THROWN) {
        cout << "uh oh - card cant be made stuck!" << endl;
        exit(1);
    }
    state = CARD_STUCK;
}

Card::~Card()
{
}
