#include "Sentry.h"
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

Sentry::Sentry()
{
    rotateOffset = (rand() % 6280) / 1000.0f; 
}

void setSentryModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
}

void SetSentryMaterial(int i, std::shared_ptr<Program>  prog) {
    switch (i) {
    case 5: //sandstone for sentry
    glUniform3f(prog->getUniform("MatAmb"), 211.0/255 * .4, 191.0/255 * .4, 145.0/255 * .4);
    glUniform3f(prog->getUniform("MatDif"), 0.3038, 0.27048, 0.0828);
    glUniform3f(prog->getUniform("MatSpec"), 0.126777, 0.137622, 0.086014);
    glUniform1f(prog->getUniform("shine"), 12.8);
    break;
    case 6: //blue for sentry
    glUniform3f(prog->getUniform("MatAmb"), 65.0/255 * .4, 154.0/255 * .4, 211.0/255 * .4);
    glUniform3f(prog->getUniform("MatDif"), 0.2f,0.3f,0.5f);
    glUniform3f(prog->getUniform("MatSpec"),0.02f,0.3f,0.7f);
    glUniform1f(prog->getUniform("shine"), 200);
    break;
    }
}

void Sentry::drawSentry(std::shared_ptr<Program>  prog, std::vector<std::shared_ptr<Multishape>> meshes) {

    // Temp "dying"
    if (health <= 0 && !charging) {
        pos = glm::vec3(0,-20, 0);
        state = SENTRY_DEAD;
        return;
    }

    std::shared_ptr<MatrixStack> Model = make_shared<MatrixStack>();
    Model->loadIdentity();
    float curTime = glfwGetTime();
    
    // Enum stuff
    int sentry = 3;
    int laser = 4;
    int sen_bot = 0;
    int sen_top = 1;
    int sen_mid = 2;
    
    charging = false;
    firing = false;

    double amount = 0;
    if (canFire) {
        if (curTime > lastRotate + rotateDelay) {
            lastRotate = curTime;
        }
        //Currently firing
        if (curTime > lastRotate && lastRotate + rotateDuration > curTime) {
            charging = true;
            // Value between 0 and 1
            amount = pow(3,(-3.215*pow(curTime-lastRotate-.5*rotateDuration,2)));
            if (amount > .8) {
                firing = true;
            }
        }
    }

    // Update the pos
    pos.y = .1*sin(curTime*.6 + rotateOffset) + .5;
    top = pos.y + .35;
    bottom = pos.y - .35;

    
    Model->pushMatrix();
        SetSentryMaterial(5, prog);
        
        // Place full obj
        Model->translate(pos);
        Model->scale(glm::vec3(0.7/(meshes[sentry]->max.y - meshes[sentry]->min.y)));

        // Center full obj
        Model->translate(-meshes[sentry]->getCenter());		

        // Bottom
        Model->pushMatrix();
            Model->translate(meshes[sentry]->shapes[sen_bot]->getCenter());	
            Model->rotate(curTime*.6 + rotateOffset,glm::vec3(0,1,0));
            if (charging) {
                Model->scale(glm::vec3(1, 1+amount/6, 1));
                Model->translate(glm::vec3(0,amount/30,0));
                Model->rotate(5*amount,glm::vec3(0,-1,0));
            }
            Model->translate(-meshes[sentry]->shapes[sen_bot]->getCenter());	
            setSentryModel(prog, Model);
            meshes[sentry]->shapes[sen_bot]->draw(prog);
        Model->popMatrix();
        // Top
        Model->pushMatrix();
            Model->translate(meshes[sentry]->shapes[sen_top]->getCenter());	
            Model->rotate(curTime*.6 + rotateOffset,glm::vec3(0,-1,0));
            if (charging) {
                Model->scale(glm::vec3(1, 1+amount/6, 1));
                Model->translate(glm::vec3(0,-amount/30,0));
                Model->rotate(5*amount,glm::vec3(0,1,0));
            }
            Model->translate(-meshes[sentry]->shapes[sen_top]->getCenter());
            setSentryModel(prog, Model);
            meshes[sentry]->shapes[sen_top]->draw(prog);
        Model->popMatrix();
        // Middle
        SetSentryMaterial(6, prog);
        Model->pushMatrix();
            Model->translate(meshes[sentry]->shapes[sen_mid]->getCenter());	
            if(firing) {
                Model->pushMatrix();
                    Model->rotate(2,glm::vec3(0,-1,0));
                    Model->translate(glm::vec3(200,0,0));
                    Model->scale(glm::vec3(20,(amount-.8)*5,(amount-.8)*5));			
                    setSentryModel(prog, Model);
                    meshes[laser]->shapes[0]->draw(prog);
                Model->popMatrix();
            }
            Model->translate(-meshes[sentry]->shapes[sen_mid]->getCenter());	
            setSentryModel(prog, Model);
            meshes[sentry]->shapes[sen_mid]->draw(prog);
        Model->popMatrix();
    Model->popMatrix();


}

Sentry::~Sentry()
{
}
