#include <raylib.h>
#include <raymath.h>
#include "lib/C_vector/C_vector.h"

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 800
#define PARTICLE_RADIUS 8

typedef struct {
    Vector2 position;
    float radius;
    float mass;
    Color color;
    Vector2 velocity;
} Particle;

#define Particle()                                    \
    (Particle){                                       \
        .position = {                                 \
            .x = GetRandomValue(0, WINDOW_WIDTH),     \
            .y = GetRandomValue(0, WINDOW_HEIGHT)     \
        },                                            \
        .radius = 8,                                  \
        .mass = 2,                                    \
        .color = (Color){                             \
            .r = GetRandomValue(0, 255),              \
            .g = GetRandomValue(0, 255),              \
            .b = GetRandomValue(0, 255), .a = 255},   \
        .velocity = (Vector2){                        \
            .x = GetRandomValue(-500, 500) / 15.0f,   \
            .y = 0                                    \
        }                                             \
    }

Particle* particles;
uint32_t to_add = 0;

Vector2 compute_gravity(Particle* particle){
    return (Vector2){0, particle->mass * -9.81f};
}

void apply_force(Particle* particle, float delta_time){
    Vector2 force = compute_gravity(particle);
    Vector2 acceleration = (Vector2){ force.x / particle->mass, force.y / particle->mass };
    
    particle->velocity.x += acceleration.x * delta_time;
    particle->velocity.y += acceleration.y * delta_time;

    particle->position.x += particle->velocity.x * delta_time;
    /* subtract y since low = up */
    particle->position.y -= particle->velocity.y * delta_time;    
}

void apply_collision(Particle* particles){
    for (size_t i = 0; i < get_count_array(particles); ++i){
        Particle* particle_1 = particles + i;

        if (particle_1->position.x >= WINDOW_WIDTH  - particle_1->radius) {
            particle_1->velocity.x = -particle_1->velocity.x;
            particle_1->position.x = WINDOW_WIDTH - particle_1->radius;
        } else
        if (particle_1->position.x <= particle_1->radius) {
            particle_1->velocity.x = -particle_1->velocity.x;
            particle_1->position.x = particle_1->radius;
        }
        
        if (particle_1->position.y >= WINDOW_HEIGHT - particle_1->radius) {
            particle_1->velocity.y = -particle_1->velocity.y;
            particle_1->position.y = WINDOW_HEIGHT - particle_1->radius;
        } else
        if (particle_1->position.y <= particle_1->radius) {
            particle_1->velocity.y = -particle_1->velocity.y;
            particle_1->position.y = particle_1->radius;
        }

    for (size_t j = i + 1; j < get_count_array(particles); ++j){
            Particle* particle_2 = particles + j;
            float distance = Vector2Distance(particle_1->position, particle_2->position);
            float sum_radii = particle_1->radius + particle_2->radius;

            if (distance <= sum_radii){
                ++to_add;

                Vector2 normal = Vector2Scale(Vector2Subtract(particle_2->position, particle_1->position), 1.0f / distance);

                float overlap = (sum_radii - distance) / 2.0f;

                Vector2 displacement = Vector2Scale(normal, overlap);
                particle_1->position = Vector2Subtract(particle_1->position, displacement);
                particle_2->position = Vector2Add(particle_2->position, displacement);

                Vector2 tangent = (Vector2){.x = -normal.y, .y = normal.x};

                float dp_tan_1 = Vector2DotProduct(particle_1->velocity, tangent);
                float dp_tan_2 = Vector2DotProduct(particle_2->velocity, tangent);

                float dp_norm_1 = Vector2DotProduct(particle_1->velocity, normal);
                float dp_norm_2 = Vector2DotProduct(particle_2->velocity, normal);

                float m1 = (dp_norm_1 * (particle_1->mass - particle_2->mass) + 2.0f * particle_2->mass * dp_norm_2) / (particle_1->mass + particle_2->mass);
                float m2 = (dp_norm_2 * (particle_2->mass - particle_1->mass) + 2.0f * particle_1->mass * dp_norm_1) / (particle_2->mass + particle_1->mass);

                particle_1->velocity = (Vector2){
                    .x = tangent.x * dp_tan_1 + normal.x * m1,
                    .y = tangent.y * dp_tan_1 + normal.y * m1
                };

                particle_2->velocity = (Vector2){
                    .x = tangent.x * dp_tan_2 + normal.x * m2,
                    .y = tangent.y * dp_tan_2 + normal.y * m2
                };
            }
        }
    }
}

void step_simulation(Particle* particles, float delta_time){
    for (uint32_t i = 0; i < get_count_array(particles); ++i)
        apply_force(particles + i, delta_time);
    
    apply_collision(particles);
}

void draw_particles(Particle* particles){
    for (size_t i = 0; i < get_count_array(particles); ++i)
        DrawCircleV(particles[i].position, particles[i].radius, particles[i].color);
}

int main(){
    SetRandomSeed(time(NULL));

    particles = alloc_array(sizeof(Particle), 0);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Demo");

    Particle new_particle = Particle();
    pback_array(&particles, &new_particle);

    new_particle = Particle();
    pback_array(&particles, &new_particle);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        draw_particles(particles);
        EndDrawing();

        for (; to_add; --to_add){
            new_particle = Particle();
            pback_array(&particles, &new_particle);
        }

        step_simulation(particles, 0.01f);
        usleep(1000);
    }

    free_array(particles, NULL);

    CloseWindow();
}