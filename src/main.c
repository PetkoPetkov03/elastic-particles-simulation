#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "../includes/raylib/raylib.h"
#include <math.h>
#include <stdbool.h>
#define WIDTH 800
#define HEIGHT 650
#define PARTICLE_NUMS 50
#define MAX_PRAD 10


typedef struct {
    float mass;
    float centerX;
    float centerY;
    float vX;
    float vY;
    float rad;
} Particle;

static Particle** particles;

enum GeneratorType {
    UNIQUE,
    NUNIQUE,
};


void PRandomNumGenImpl(
float out[PARTICLE_NUMS],
float min,
float max,
enum GeneratorType gtype)
{
    float range = (max - min) + 1;

    if(gtype == UNIQUE) {
        if(PARTICLE_NUMS > range) {
            fprintf(stderr, "Number of intances %i larger"
            "than available range %f\n", PARTICLE_NUMS, range);
            exit(1);
        }
    }

    float *pool = (float*)malloc(sizeof(float)*range);

    if(pool == NULL) {
        fprintf(stderr, "malloc error");
        exit(1);
    }

    for(int i = 0; i < range; i++) pool[i] = min + i;

    for(int i = range - 1; i > 0; i--) {
        int j = rand() % (i+1);
        int temp = pool[i];
        pool[i] = pool[j];
        pool[j] = temp;
    }

    for(int i = 0; i < PARTICLE_NUMS; i++) {
        out[i] = pool[i];
    }

    free(pool);
}

void PRandomNumGen(
float out[PARTICLE_NUMS],
float min,
float max
)
{
    PRandomNumGenImpl(out, min, max, UNIQUE);
}

void PRandomNumGenNU(
float out[PARTICLE_NUMS],
float min,
float max
)
{
    PRandomNumGenImpl(out, min, max, NUNIQUE);
}


void ConvertParticleToParticleVector(Vector2* vec, Particle* part)
{
    vec->x = part->centerX;
    vec->y = part->centerY;
}

bool CirclesIntersect(Vector2 a, Vector2 b)
{
    float c1 = pow((a.x - a.y), 2);
    float c2 = pow((b.x - b.y), 2);
    float distance = sqrt(c1 + c2);

    return distance > (c1+c2);
}

bool CirclesNested(Vector2 a, Vector2 b)
{
    float c1 = pow((a.x - a.y), 2);
    float c2 = pow((b.x - b.y), 2);
    float distance = sqrt(c1 + c2);

    return distance > fabs(c1-c2);
}

void CheckInitialCollision(Vector2* position, size_t cSize)
{
    for(size_t i = 0; i < cSize; i++) {
        Vector2 cPartPosition = {0};
        ConvertParticleToParticleVector(&cPartPosition, particles[i]);


    }
}

void ConvertParticleVectorToParticle(Particle **part, Vector2 vector)
{
    (*part)->centerX = vector.x;
    (*part)->centerY = vector.y;
}

void SpawnParticles(
float xpos[PARTICLE_NUMS],
float ypos[PARTICLE_NUMS])
{
    particles = (Particle**)malloc(sizeof(Particle*)*PARTICLE_NUMS);

    if(particles == NULL) {
        fprintf(stderr, "particles list uninitialized");
        exit(1);
    }

    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        Particle* part = (Particle*)malloc(sizeof(Particle));

        if(part == NULL) {
            fprintf(stderr, "particle %ld uninitialized", i);
            exit(1);
        }

        part->centerX = xpos[i];
        part->centerY = ypos[i];
        int dirDeciderX = (rand() % 3) + 1;
        int dirDeciderY = (rand() % 3) + 1;
        bool directionX = dirDeciderX < 2 ? true: false;
        bool directionY = dirDeciderY < 2 ? true: false;

        if(directionX) {
            part->vX = 1;
        }else {
            part->vX = -1;
        }

        if(directionY) {
            part->vY = 1;
        }else {
            part->vY = -1;
        }
        part->rad = (rand() % MAX_PRAD) + 4;
        part->mass = part->rad * PI;

        printf("Particle %i mass %f\n", i, part->mass);

        Vector2 partVec = {0};
        ConvertParticleToParticleVector(&partVec, part);
        CheckInitialCollision(&partVec, i);
        ConvertParticleVectorToParticle(&part, partVec);
        particles[i] = part;
    }
}

void UpdateParticles()
{
    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        if((particles[i]->centerX + particles[i]->rad) > WIDTH) {
            particles[i]->centerX = WIDTH - particles[i]->rad;
            particles[i]->vX *= -1;
        }

        if((particles[i]->centerX - particles[i]->rad) < 0) {
            particles[i]->centerX = 0 + particles[i]->rad;
            particles[i]->vX *= -1;
        }

        if((particles[i]->centerY + particles[i]->rad) > HEIGHT) {
            particles[i]->centerY = HEIGHT - particles[i]->rad;
            particles[i]->vY *= -1;
        }

        if((particles[i]->centerY - particles[i]->rad) < 0) {
            particles[i]->centerY = 0 + particles[i]->rad;
            particles[i]->vY *= -1;
        }

        particles[i]->centerX += particles[i]->vX;
        particles[i]->centerY += particles[i]->vY;
    }
}

void DrawParticles() {
    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        Particle* cPart = particles[i];
        Vector2 particleVector = {0};

        ConvertParticleToParticleVector(&particleVector, cPart);

        DrawCircleV(particleVector, cPart->rad, RED);
    }
}

void DestroyParticles()
{
    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        free(particles[i]);
    }
    free(particles);
}

int main(void)
{
    srand(time(NULL));
    InitWindow(WIDTH, HEIGHT, "Elastic Collision Simulator");

    SetTargetFPS(60);

    float positionsX[PARTICLE_NUMS] = {0};
    float positionsY[PARTICLE_NUMS] = {0};

    PRandomNumGen(positionsX, 1.f, WIDTH);
    PRandomNumGen(positionsY, 1.f, HEIGHT);
    SpawnParticles(positionsX, positionsY);

    while(!WindowShouldClose()) {
        BeginDrawing();
          ClearBackground(WHITE);
          DrawParticles();
          UpdateParticles();
        EndDrawing();
    }

    DestroyParticles();
    CloseWindow();

    return 0;
}
