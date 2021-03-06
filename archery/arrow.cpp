#include "arrow.h"
#include "util.h"
#include "player.h"

#include <GL/glut.h>
#include <iostream>
#include <ctime>
#include <cstdio>

int prev_tick, curr_tick;
vec3 gravity = {0, -9.81, 0};
vec3 wind;

enum ArrowParts {
    SHAFT = 0,
    HEAD = 1,
    FLETCHING = 2,
    ARR_PARTS = 3
};

Arrow::Arrow() {
    pos = {0, 0, 0};
    thickness = length = 0.0f;
    pitch = yaw = roll = 0.0f;
    state = STASHED;
}

Arrow::Arrow(float t, float l) {
    thickness = t;
    length = l;

    pos = {0, 3, 0};
    vel = {0, 0, 0};
    state = STASHED;
}

void Arrow::make_handle() {
    handle = glGenLists(ARR_PARTS);
    // texture = load_and_bind_tex("images/arrow.png");

    glNewList(handle + SHAFT, GL_COMPILE);
        glPushMatrix();
            draw_capped_cylinder(thickness, length);
        glPopMatrix();
    glEndList();

    glNewList(handle + HEAD, GL_COMPILE);
        glPushMatrix();
            glRotatef(180, 0, 1, 0);
            draw_cone(thickness, 0.2);
        glPopMatrix();
    glEndList();

    glNewList(handle + FLETCHING, GL_COMPILE);
        glPushMatrix();
            glPushMatrix();
                glTranslatef(0, 0, length);
                glScalef(0.1, 0.01, 0.1);
                glutSolidCube(1);
            glPopMatrix();

            glRotatef(90, 0, 0, 1);

            glPushMatrix();
                glTranslatef(0, 0, length);
                glScalef(0.1, 0.01, 0.1);
                glutSolidCube(1);
            glPopMatrix();
        glPopMatrix();
    glEndList();
}

    // glNewList(handle, GL_COMPILE);
    //     glPushMatrix();

    //     /*
    //         glBindTexture(GL_TEXTURE_2D, texture);

    //         glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    //         glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    //         glEnable(GL_TEXTURE_GEN_S);
    //         glEnable(GL_TEXTURE_GEN_T);
    //         glEnable(GL_TEXTURE_2D);

    //         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    //     */

    //         // shaft
    //         draw_capped_cylinder(thickness, length);

    //         glDisable(GL_TEXTURE_GEN_S);
    //         glDisable(GL_TEXTURE_GEN_T);
    //         glDisable(GL_TEXTURE_2D);

    //         // head
    //         glPushMatrix();
    //             glRotatef(180, 0, 1, 0);
    //             draw_cone(thickness, 0.2);
    //         glPopMatrix();

    //         // fletchings
    //         glPushMatrix();
    //             glTranslatef(0, 0, length);
    //             glRotatef(90, 0, 0, 1);
    //             glScalef(0.1, 0.01, 0.1);
    //             glutSolidCube(1);
    //         glPopMatrix();

    //         glPushMatrix();
    //             glTranslatef(0, 0, length);
    //             glScalef(0.1, 0.01, 0.1);
    //             glutSolidCube(1);
    //         glPopMatrix();

    //         glPushMatrix();
    //         glPopMatrix();
    //     glPopMatrix();
    // glEndList();

void Arrow::point() {
    if (vel.z < 0) {
        yaw = atan2(vel.y, vel.z) * 180.f / M_PI;
        pitch = atan2(vel.x, vel.z) * 180.f / M_PI;
        glRotatef(180 - yaw, 1, 0, 0);
        glRotatef(180 + pitch, 0, 1, 0);
    } else {
        // TODO: get this right
        yaw = atan2(vel.z, vel.y) * 180.f / M_PI;
        pitch = atan2(vel.x, vel.z) * 180.f / M_PI;
        glRotatef(180 + yaw, 1, 0, 0);
        glRotatef(180 + pitch, 0, 1, 0);
    }

    if (state == DEAD)
        glRotatef(90, 1, 0, 0);
}

void Arrow::simulate() {

    if (vel.len() < 0.01f) {
        vel = {0, -1, 0};
        state = DEAD;
    }

    // prev_tick = curr_tick;
    // curr_tick = glutGet(GLUT_ELAPSED_TIME);
    
    prev_tick = curr_tick;
    curr_tick = clock();

    float dt = ((float) (curr_tick - prev_tick)) / CLOCKS_PER_SEC;
    dt = dt > 0.1 ? 0.016 : dt;

    // float dt = glutGet(GLUT_ELAPSED_TIME) / 10000;
    // float dt = ((float) (curr_tick - prev_tick)) / 1000.f;

    if (curr_tick < prev_tick)
        dt = 0.016;

    vec3 dv = gravity * dt;

    vel += dv;
    pos += vel;

    if (pos.y <= GROUND_LEVEL) {
        pos.y = 0;
        vel.y *= -0.5;

        vel.x *= 0.5;
        vel.z *= 0.5;
    }

    roll += 15;

    // glutPostRedisplay();
}

void Arrow::draw_nocked() {
    glPushMatrix();
        glTranslatef(0.4, -0.2, -1.8 + pulled);
        glRotatef(15, 0, 1, 0);
        glCallList(handle + SHAFT);
        glCallList(handle + HEAD);
        glCallList(handle + FLETCHING);
    glPopMatrix();
    glutPostRedisplay();
}

void Arrow::draw_flight() {

    glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        point();
        glRotatef(roll, 0, 0, 1);
        glCallList(handle + SHAFT);
        glCallList(handle + HEAD);
        glCallList(handle + FLETCHING);
    glPopMatrix();

    glutPostRedisplay();
}

/* use simple axis aligned bounding box to detect collisions */
bool Arrow::colliding_with(Target& t) {
    vec3 tail = normalize(vel * -1) * length;
    if (
        ((pos.x >= t.pos.x - t.radius) && (pos.x <= t.pos.x + t.radius) &&
        (pos.y >= t.pos.y - t.radius) && (pos.y <= t.pos.y + t.radius) &&
        (pos.z <= t.pos.z + t.thickness + 4*t.margin) && (pos.z >= t.pos.z - 8*t.margin)) /*||

        ((tail.x >= t.pos.x - t.radius) && (tail.x <= t.pos.x + t.radius) &&
        (tail.y >= t.pos.y - t.radius) && (tail.y <= t.pos.y + t.radius) &&
        (tail.z <= t.pos.z + t.thickness + 4*t.margin) && (tail.z >= t.pos.z - 8*t.margin))*/
    ) {
            offset = {
                pos.x - t.pos.x,
                pos.y - t.pos.y,
                pos.z - t.pos.z
            };

            state = STUCK;
            stuck_in = &t;

        return true;
    } else {
        return false;
    }
}

void Arrow::stick_in() {
    pos = {
        stuck_in->pos.x + offset.x,
        stuck_in->pos.y + offset.y,
        stuck_in->pos.z + offset.z
    };
}

void Arrow::draw_stuck() {
    glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        point();
        glRotatef(roll, 0, 0, 1);
        glCallList(handle + SHAFT);
        glCallList(handle + HEAD);
        glCallList(handle + FLETCHING);
    glPopMatrix();

    glutPostRedisplay();
}

int Arrow::get_score(Target& t) {
    float seg_width = t.radius / t.segments;
    float dist2d = sqrt(
            pow(pos.x - t.pos.x, 2) + pow(pos.y - t.pos.y, 2)
    );
    return t.segments + 1 - ceil(dist2d / seg_width);
}
