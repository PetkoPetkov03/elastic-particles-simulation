#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/raylib/raylib.h"
#include <math.h>
#include <stdbool.h>
#define WIDTH 800
#define HEIGHT 650
#define PARTICLE_NUMS 50
#define MAX_PRAD 10


typedef struct {
    float mass;
    Vector2 position;
    Vector2 velocity;
    float rad;
} Particle;

static Particle particles[PARTICLE_NUMS];

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


void ConvertParticleToParticleVector(Vector2* vec, Particle part)
{
    vec->x = part.position.x;
    vec->y = part.position.y;
}

bool CirclesIntersect(Vector2 a, Vector2 b)
{
    float c1 = pow((a.x - a.y), 2);
    float c2 = pow((b.x - b.y), 2);
    float distance = sqrt(c1 + c2);

    return distance > (c1+c2);
}

void SpawnParticles(
float xpos[PARTICLE_NUMS],
float ypos[PARTICLE_NUMS])
{
    Particle part = {0};

    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        part.position = (Vector2){
            .x = xpos[i],
            .y = ypos[i]
        };
        int dirDeciderX = (rand() % 3) + 1;
        int dirDeciderY = (rand() % 3) + 1;
        bool directionX = dirDeciderX < 2 ? true: false;
        bool directionY = dirDeciderY < 2 ? true: false;

        if(directionX) {
            part.velocity.x = 1;
        }else {
            part.velocity.x = -1;
        }

        if(directionY) {
            part.velocity.y = 1;
        }else {
            part.velocity.y = -1;
        }
        part.rad = (rand() % MAX_PRAD) + 4;
        part.mass = part.rad * PI;

        particles[i] = part;
    }
}

void AbideBorder(size_t i)
{
    if((particles[i].position.x + particles[i].rad) > WIDTH) {
        particles[i].position.x = WIDTH - particles[i].rad;
        particles[i].velocity.x *= -1;
    }

    if((particles[i].position.x - particles[i].rad) < 0) {
        particles[i].position.x = 0 + particles[i].rad;
        particles[i].velocity.x *= -1;
    }

    if((particles[i].position.y + particles[i].rad) > HEIGHT) {
        particles[i].position.y = HEIGHT - particles[i].rad;
        particles[i].velocity.y *= -1;
    }

    if((particles[i].position.y - particles[i].rad) < 0) {
        particles[i].position.y = 0 + particles[i].rad;
        particles[i].velocity.y *= -1;
    }
}

void ParticleMove(size_t i)
{
    particles[i].position.x += particles[i].velocity.x;
    particles[i].position.y += particles[i].velocity.y;
}

typedef struct {
    void* first;
    void* second;
} Pair;

typedef struct {
    float distance;
    float r1;
    float r2;
} CheckResult;

Pair MakePair(const void* first, size_t first_size,
const void* second, size_t second_size)
{
    Pair result;

    result.first = malloc(first_size);
    result.second = malloc(second_size);

    if(result.first && first_size > 0) {
        memcpy(result.first, first, first_size);
    }

    if(result.second && second_size > 0) {
        memcpy(result.second, second, second_size);
    }

    return result;
}

Pair CheckIfParticlesIntersect(size_t i, size_t j)
{
    Particle particle1 = particles[i];
    Particle particle2 = particles[j];

    float r1 = pow((particle2.position.x - particle1.position.x), 2);
    float r2 = pow((particle2.position.y - particle1.position.y), 2);
    float distance = sqrt(r1 + r2);

    float absOfRadi = fabs(particle1.rad - particle2.rad);
    float sum = particle1.rad + particle2.rad;

    bool check = absOfRadi < distance && distance < sum;

    CheckResult cResult = {
        .distance = distance,
        .r1 = particle1.rad,
        .r2 = particle2.rad
    };

    Pair result = MakePair(&check, sizeof(check), &cResult, sizeof(cResult));

    return result;
}

void AddVectorWNum(Vector2 *a, float b)
{
    a->x += b;
    a->y += b;
}

Vector2 FindIntersectionPoint(
const Particle p1,
const Particle p2,
float distance,
float distance_intersection,
float height)
{
    float x1 = p1.position.x;
    float x2 = p2.position.x;
    float y1 = p1.position.y;
    float y2 = p2.position.y;
    Vector2 P2 = {
        .x = x1 + distance_intersection*((x2 - x1)/distance),
        .y = y1 + distance_intersection*((y2 - y1)/distance)
    };

    Vector2 P3 = {
        .x = P2.x + height*((y1 - P2.y)/distance),
        .y = P2.y + height*((x1 - P2.x)/distance)
    };

    return P3;
}

void CorrectParticles(const CheckResult cr, const size_t i, const size_t j)
{
    Particle* particle1 = &particles[i];
    Particle* particle2 = &particles[j];

    float distance = cr.distance;

    float r1 = particle1->rad;
    float r2 = particle2->rad;

    // float distance_intersection =
    // pow(r1, 2) - pow(r2, 2) + pow(distance, 2) / (2*distance);

    // float height = sqrt(pow(r1, 2) - pow(distance_intersection, 2));


    // Vector2 ipoint =
    // FindIntersectionPoint(*particle1, *particle2,
    // cr.distance, distance_intersection, height);

    // Pair reconstructionPoints =
    // FindColinears(ipoint, distance_intersection, height);

    Vector2 d = {
        .x = particle2->position.x - particle1->position.x,
        .y = particle2->position.y - particle1->position.y
    };

    float vectorLength = sqrt(pow(d.x, 2) + pow(d.y, 2));

    Vector2 nd = {
        .x = d.x / vectorLength,
        .y = d.y / vectorLength
    };

    Vector2 pr = {
        .x = nd.x * (r1 + r2),
        .y = nd.y * (r1 + r2)
    };

    particle2->position.x = particle1->position.x + pr.x;
    particle2->position.y = particle1->position.y + pr.y;

}

void UpdateParticles()
{
    bool particlesChecked[PARTICLE_NUMS][PARTICLE_NUMS] = {false};

    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        AbideBorder(i);
        for(size_t j = 0; j < PARTICLE_NUMS; j++) {
            if(particlesChecked[i][j]) {
                continue;
            }
            if(i == j) {
                continue;
            }

            Pair result = CheckIfParticlesIntersect(i, j);
            if(*(bool*)(result.first)) {
                particlesChecked[i][j] = true;
                CorrectParticles(*(CheckResult*)result.second, i, j);
                printf("Particle %ld and Particle %ld intersect\n", i , j);
            }
        }
        ParticleMove(i);
    }
}

void DrawParticles() {
    for(size_t i = 0; i < PARTICLE_NUMS; i++) {
        Particle cPart = particles[i];
        Vector2 particleVector = {0};

        ConvertParticleToParticleVector(&particleVector, cPart);

        DrawCircleV(particleVector, cPart.rad, RED);
        DrawText(TextFormat("P%ld", i), cPart.position.x - cPart.rad,
        cPart.position.y - cPart.rad, cPart.rad*1.5, GREEN);
    }
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
          DrawFPS(WIDTH - 100, 10);
          DrawParticles();
          UpdateParticles();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
