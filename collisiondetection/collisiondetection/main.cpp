//
//  main.cpp
//  collisiondetection
//
//  Created by Raymond Kim on 10/24/16.
//  Copyright © 2016 raymnd. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <vector>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

#include "shader.h"
#include "Camera.h"
#include "Polytope.h"
#include "vcollide/src/VCollide.H"

const GLint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

void KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mode );
//void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void MouseCallback( GLFWwindow* window, double xPos, double yPos);
void DoCameraMovement();
void DoObjectMovement();
void DoCollisionCheckOne();
void DoCollisionCheckTwo(Polytope a, Polytope b);
void ComputeResponse(Polytope a, Polytope b);
void TestOverhead();

Camera camera( glm::vec3(0.0f, 0.0f, 3.0f) );
GLfloat lastX = WIDTH/2.0f;
GLfloat lastY = WIDTH/2.0f;
bool keys[1024];
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

glm::vec3 lightPos(10.0f, 10.0f, 10.0f);

std::vector<Polytope> objects;
std::vector<Polytope> boundary;

//*********//
// INPUTS:
 
GLuint numberofObjects = 15;
GLboolean VARYING_SIZE = GL_TRUE;

//*********//

int main() {

    srand(4);
    glfwInit(  );
    
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Collision Detection", nullptr, nullptr);
    
    
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    
    if( nullptr == window ) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
//    glfwSetScrollCallback( window, ScrollCallback );
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glewExperimental = GL_TRUE;
    
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initalize GLEW" << std::endl;
        return -1;
    }
    
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Shader coreShader("resources/shaders/core.vs", "resources/shaders/core.frag");
    Shader sourceShader("resources/shaders/source.vs", "resources/shaders/source.frag");
    
//    GLfloat vertices[] =
//    {
//        //position              // color
//        -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f, //bottom left
//         0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f, //bottom right
//         0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f  //middle
//    };

    // Set up vertex data (and buffer(s)) and attribute pointers
    
    GLfloat vertices[] = {
        //position          //normal
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    
//    glm::vec3 cubePositions[] = {
//        glm::vec3(0.0f, 0.0f, 0.0f),
//        glm::vec3(2.0f, 5.0f, -15.0f),
//        glm::vec3(-1.5f, -2.2f, -2.5f),
//        glm::vec3(-3.8f, -2.0f, -12.3f),
//        glm::vec3(2.4f, -0.4f, -3.5f),
//        glm::vec3(-1.7f, 3.0f, -7.5f),
//        glm::vec3(1.3f, -2.0f, -2.5f),
//        glm::vec3(1.5f, 2.0f, -2.5f),
//        glm::vec3(1.5f, 0.2f, -1.5f),
//        glm::vec3(-1.3f, 1.0f, -1.5f),
//    };
    
    
    /* VCOLLID STUFF HERE*/
    
    // i think vcollide library is too old
    
    //VCollide vc;
    
    /*********************/
    

    
    //initialize objects
    for (GLuint i = 0; i<numberofObjects; i++) {
        Polytope obj = Polytope((GLfloat)rand()/(GLfloat)(RAND_MAX/4) + 3.0f);
        
        bool flag = false;
        
        GLfloat px =(GLfloat)rand()/(GLfloat)(RAND_MAX/28) - 14.0f;
        GLfloat py =(GLfloat)rand()/(GLfloat)(RAND_MAX/28) - 14.0f;
        GLfloat pz =(GLfloat)rand()/(GLfloat)(RAND_MAX/28) - 14.0f;
        
        obj.setPosition(glm::vec3(px, py, pz));
        
        for (GLuint j = 0; j < i; j++) {
            GLfloat dis = glm::length(obj.getPosition()-objects[j].getPosition());
            GLfloat sum =  obj.getRadius()+objects[j].getRadius();
            if( dis < sum) {
                std::cout << i<<":["<<obj.getPosition().x<<", "<<obj.getPosition().y<<", "<<obj.getPosition().z<<"]"<<" is too close to "<<j<<":["<<objects[j].getPosition().x<<", "<<objects[j].getPosition().y<<", "<<objects[j].getPosition().z<<"]"<<"   "<< dis << std::endl;
                i--;
                flag = true;
            }
        }
        if (flag) {
            continue;
        }
        
        GLfloat pVx =(GLfloat)rand()/(GLfloat)(RAND_MAX/2) - 1.0f;
        GLfloat pVy =(GLfloat)rand()/(GLfloat)(RAND_MAX/2) - 1.0f;
        GLfloat pVz =(GLfloat)rand()/(GLfloat)(RAND_MAX/2) - 1.0f;
        
        obj.setposVel(glm::vec3(pVx,pVy,pVz));
        obj.setRot(glm::vec3((GLfloat)rand()/(GLfloat)RAND_MAX, (GLfloat)rand()/(GLfloat)RAND_MAX, (GLfloat)rand()/(GLfloat)RAND_MAX));
        //angular spped from -5.0f to 5.0f
        obj.setangVel((GLfloat)rand()/(GLfloat)(RAND_MAX/5) - 2.5f);
        
        objects.push_back(obj);
        std::cout <<"got pushed: "<< i<<": "<< obj.getSize() <<" "<<std::endl;
  
    }
    
    
    //initaize boundary
    
    Polytope surface1 = Polytope(30-0.01);
    surface1.setPosition(glm::vec3(-30.0f, 0.0f, 0.0f));
    Polytope surface2 = Polytope(30-0.01);
    surface1.setPosition(glm::vec3(30.0f, 0.0f, 0.0f));
    Polytope surface3 = Polytope(30-0.01);
    surface1.setPosition(glm::vec3(0.0f, 30.0f, 0.0f));
    Polytope surface4 = Polytope(30-0.01);
    surface1.setPosition(glm::vec3(0.0f, -30.0f, 0.0f));
    Polytope surface5 = Polytope(30-0.01);
    surface1.setPosition(glm::vec3(0.0f, 0.0f, 30.0f));
    Polytope surface6 = Polytope(30-0.01);
    surface1.setPosition(glm::vec3(0.0f, 0.0f, -30.0f));
    
    boundary.push_back(surface1);
    boundary.push_back(surface2);
    boundary.push_back(surface3);
    boundary.push_back(surface4);
    boundary.push_back(surface5);
    boundary.push_back(surface6);
    
    objects.push_back(surface1);
    objects.push_back(surface2);
    objects.push_back(surface3);
    objects.push_back(surface4);
    objects.push_back(surface5);
    objects.push_back(surface6);
    
    GLuint VBO, boxVAO;
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) 0 );
    glEnableVertexAttribArray(0);
    
    //Normal Attribute
    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray(1);
    
//    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)) );
//    glEnableVertexAttribArray(1);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    
    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (GLvoid*) 0 );
    glEnableVertexAttribArray(0);
    
    //    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)) );
    //    glEnableVertexAttribArray(1);
    
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glm::mat4 projection = glm::perspective( camera.getZoom( ), ( GLfloat )SCREEN_WIDTH / ( GLfloat )SCREEN_HEIGHT, 0.1f, 100.0f );
    
    float sum = 0.0f;
    
    while (!glfwWindowShouldClose(window)) {
        
        
        
        
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents();
        
        DoCameraMovement();
        DoObjectMovement();
        
        //glClearColor(1.0f, 0.8f, 0.6f, 1.0f);
        glClearColor(0.96f, 0.94f, 0.87f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //******************************************************
        
        coreShader.Use();
        
        GLint objectColorLoc = glGetUniformLocation( coreShader.Program, "objectColor" );
        GLint lightColorLoc = glGetUniformLocation( coreShader.Program, "lightColor" );
        GLint lightPosLoc = glGetUniformLocation( coreShader.Program, "lightPos" );
        GLint viewPosLoc = glGetUniformLocation( coreShader.Program, "viewPos" );
        glUniform4f( objectColorLoc, 0.4f, 0.8f, 1.0f, 0.1f);
        glUniform4f( lightColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform3f( lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f( viewPosLoc, camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        
        glm::mat4 view;
        view = camera.getViewMatrix();
        
        GLint modelLoc = glGetUniformLocation(coreShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(coreShader.Program, "view");
        GLint projLoc = glGetUniformLocation(coreShader.Program, "projection");
        
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        glBindVertexArray(boxVAO);
        
        glUniform4f( objectColorLoc, 0.4f, 0.8f, 1.0f, 0.1f);

//draws oustide box
        glm::mat4 model;
//        model = glm::scale(model, glm::vec3(30.0f));
//        //model = glm::rotate(model, (GLfloat) glfwGetTime( ) * -1.0f, glm::vec3( 1.0f, 0.3f, 0.5f ) );
//        //model = glm::rotate(model, 20.0f * i, glm::vec3( 1.0f, 0.3f, 0.5f ) );
//        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        
        glBindVertexArray(0);
        glUniform4f( objectColorLoc, 0.4f, 0.8f, 1.0f, 1.0f);
        glBindVertexArray(boxVAO);
        
        for (GLuint i=0; i < numberofObjects; i++) {
            glm::mat4 model;
            model = glm::translate(model, objects[i].getPosition());
            model = glm::scale(model, glm::vec3(objects[i].getSize()));
            model = glm::rotate(model, (GLfloat) glfwGetTime( ) * objects[i].getangularVel(), objects[i].getrotAxis());
            //model = glm::rotate(model, 20.0f * i, glm::vec3( 1.0f, 0.3f, 0.5f ) );
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        float a = glfwGetTime();
        DoCollisionCheckOne();
        float elapsedTime = glfwGetTime() - a;
        sum += elapsedTime;
        
        std::cout << sum << std::endl;
        
        
        glBindVertexArray(0);
        
        //******************************************************
        
//        sourceShader.Use();
//        
//        modelLoc = glGetUniformLocation(sourceShader.Program, "model");
//        viewLoc = glGetUniformLocation(sourceShader.Program, "view");
//        projLoc = glGetUniformLocation(sourceShader.Program, "projection");
//        
//        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//        
//        // model;
//        model = glm::mat4();
//        model = glm::translate(model, lightPos);
//        model = glm::scale(model, glm::vec3(0.2f));
//        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//        
//        glBindVertexArray(lightVAO);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//        glBindVertexArray(0);
        
        // 15:30
//        glm::mat4 transform;
//        transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
//        transform = glm::rotate(transform, (GLfloat) glfwGetTime() * -5.0f, glm::vec3(0.0f,0.0f,1.0f));
//        
//        GLint transformLocation = glGetUniformLocation(ourShader.Program,"transform");
//        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));
        
        
        glfwSwapBuffers(window);
        
    }
    
    glDeleteVertexArrays(1, &boxVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    
    glfwTerminate();
    return 0;
}

void DoCameraMovement() {
    
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
        
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
        
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
        
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
        
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (keys[GLFW_KEY_SPACE]) {
        
        camera.ProcessKeyboard(UP, deltaTime);
    }
//    if (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT]) {
//        
//        camera.ProcessKeyboard(DOWN, deltaTime);
//    }
}

void DoObjectMovement() {
    
    for (GLuint i = 0; i<numberofObjects; i++) {
        glm::vec3 curPos = objects[i].getPosition();
        glm::vec3 curVel = objects[i].getposVel();
        objects[i].setPosition(curPos + 0.01f * curVel);
    }
    
}

void DoCollisionCheckOne() {
    
    for (GLuint i = 0; i<numberofObjects-1+6; i++) {
        for (GLuint j = i+1; j<numberofObjects+6; j++) {
            GLfloat dis = glm::length(objects[i].getPosition()-objects[j].getPosition());
            GLfloat siz = objects[i].getRadius()+objects[j].getRadius();
            if (dis <= siz) {
                //std::cout << glfwGetTime() <<" - Collision Detected:" << i << " and " << j << std::endl;
                //std::cout << "computing response between "<<i<<", "<<j<<" :::" << glm::to_string(objects[i].getPosition())<<" and "<< glm::to_string(objects[j].getPosition()) << std::endl;
                DoCollisionCheckTwo(objects[i], objects[j]);
                
                
                
//                glm::vec3 avel = objects[i].getposVel();
//                glm::vec3 bvel = objects[j].getposVel();
//                objects[i].setposVel(-avel);
//                objects[j].setposVel(-bvel);

                
            }
        }
    }
}

void DoCollisionCheckTwo(Polytope a, Polytope b) {
    bool collide = false;
    for (GLuint i = 0 ; i < 6; i++) {
        for (GLuint j = 0; j < 9; j++) {
            //Do More Exact Computations; will take constant time for its current scope
            TestOverhead();
            collide = true;
        }
    }
    if (collide) {
        ComputeResponse(a, b);
    }
}

void ComputeResponse(Polytope a, Polytope b) {
    //std::cout << "ComputeResponse triggered" << std::endl;
    glm::vec3 avel = a.getposVel();
    float avvel = a.getangularVel();
    glm::vec3 bvel = b.getposVel();
    float bvvel = b.getangularVel();
    a.setposVel(-avel);
    a.setangVel(-avvel);
    b.setposVel(-bvel);
    b.setangVel(-bvvel);
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    
    if (GLFW_KEY_ESCAPE==key && GLFW_PRESS==action) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if (key >= 0 && key < 1024) {
        if (GLFW_PRESS == action) {
            keys[key] = true;
        }
        else if (GLFW_RELEASE == action) {
            keys[key] = false;
        }
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    
    lastX = xPos;
    lastY = yPos;
    
    camera.ProcessMouseMovement(xOffset, yOffset);
}

//void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
//    
//    camera.ProcessMouseScroll(yOffset);
//}

void TestOverhead() {
    glm::vec3 vector(1.0f, 42.5f, 294.f);
    for (int i=0; i < 10; i++) {
        vector += vector;
    }
}











































