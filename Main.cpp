#include <raylib.h>
#include <raymath.h>
#include <PxPhysicsAPI.h>

#include <cmath>

#ifdef _WIN32
extern "C" {
    __declspec(dllexport) extern const unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) extern const int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

using namespace physx;

constexpr int TOTAL_CUBES = 30;
constexpr int TOTAL_SPHERES = 30;

PxDefaultAllocator      gAllocator;
PxDefaultErrorCallback  gErrorCallback;
PxPhysics*              gPhysics    = nullptr;
PxScene*                gScene      = nullptr;
PxMaterial*             gMaterial   = nullptr;

void InitPhysX(void) {
    PxFoundation* gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = {0.0F, -9.81F, 0.0F};

    sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2); // Cores for Physics
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    gScene = gPhysics->createScene(sceneDesc);

    gMaterial = gPhysics->createMaterial(0.5F, 0.5F, 0.6F);
}

PxRigidStatic* CreateStaticCube(const PxVec3& pos, const Vector3& size) {
    PxBoxGeometry geometry(size.x, size.y, size.z);

    PxRigidStatic* body = PxCreateStatic(*gPhysics, PxTransform(pos), geometry, *gMaterial);
    gScene->addActor(*body);

    return body;
}

PxRigidDynamic* CreateDynamicCube(const PxVec3& pos, const Vector3& size, const PxReal& mass) {
    PxBoxGeometry geometry(size.x, size.y, size.z);

    PxRigidDynamic* body = PxCreateDynamic(*gPhysics, PxTransform(pos), geometry, *gMaterial, mass);
    gScene->addActor(*body);

    return body;
}

PxRigidDynamic* CreateDynamicSphere(const PxVec3& pos, const PxReal& size, const PxReal& mass) {
    PxSphereGeometry geometry(size);

    PxRigidDynamic* body = PxCreateDynamic(*gPhysics, PxTransform(pos), geometry, *gMaterial, mass);
    gScene->addActor(*body);

    return body;
}

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "PhysX Test");

    Camera3D camera = {
        .position   = { 15.0F, 10.0F, 10.0F },
        .target     = { 0.0F, 0.0F, 0.0F },
        .up         = { 0.0F, 1.0F, 0.0F },
        .fovy       = 50.0F,
        .projection = CAMERA_PERSPECTIVE
    };

    InitPhysX();

    // Platform
    constexpr Vector3 platform_size = { 10.0F, 0.5F, 10.0F };
    const Model platform_model = LoadModelFromMesh(GenMeshCube(platform_size.x, platform_size.y, platform_size.z));
    PxRigidStatic* platform = CreateStaticCube({ 0.0F, 0.0F, 0.0F }, (platform_size * 0.5F));

    // Pusher
    constexpr Vector3 pusher_size = { 10.0F, 2.0F, 0.5F };
    const Model pusher_model = LoadModelFromMesh(GenMeshCube(pusher_size.x, pusher_size.y, pusher_size.z));
    PxRigidStatic* pusher = CreateStaticCube({ 0.0F, 0.0F, 0.0F }, (pusher_size * 0.5F));

    // Dynamic Cubes
    constexpr Vector3 cube_dynamic_size = { 0.5F, 0.5F, 0.5F };
    Model cube_dynamic_model[TOTAL_CUBES];
    PxRigidDynamic* cube_dynamic[TOTAL_CUBES];

    for (int i = 0; i < TOTAL_CUBES; i++) {
        cube_dynamic_model[i] = LoadModelFromMesh(GenMeshCube(cube_dynamic_size.x, cube_dynamic_size.y, cube_dynamic_size.z));
        cube_dynamic[i] = CreateDynamicCube({ (float)GetRandomValue(-5, 5), (5.0F + (i * 2.0F)), (float)GetRandomValue(-5, 5) }, (cube_dynamic_size * 0.5F), 100.0F);
        cube_dynamic[i]->userData = reinterpret_cast<void*>(i);
    }

    // Dynamic Spheres
    constexpr float sphere_dynamic_size = 0.5F;
    Model sphere_dynamic_model[TOTAL_SPHERES];
    PxRigidDynamic* sphere_dynamic[TOTAL_SPHERES];

    for (int i = 0; i < TOTAL_SPHERES; i++) {
        sphere_dynamic_model[i] = LoadModelFromMesh(GenMeshSphere(sphere_dynamic_size, 32, 32));
        sphere_dynamic[i] = CreateDynamicSphere({ (float)GetRandomValue(-5, 5), (5.0F + (i * 2.0F)), (float)GetRandomValue(-5, 5) }, sphere_dynamic_size, 100.0F);
        sphere_dynamic[i]->userData = reinterpret_cast<void*>(i);
    }

    while (!WindowShouldClose()) {
        gScene->simulate(GetFrameTime());
        gScene->fetchResults(true);

        BeginDrawing();
        ClearBackground(DARKGRAY);
        UpdateCamera(&camera, CAMERA_ORBITAL);
        BeginMode3D(camera);
        DrawGrid(50, 1.0F);

        constexpr float bias  = 5.0F;
        static float dyn_bias = 5.0F;
        static float accel    = 0.0F;

        if (accel <= -bias + 1.0F) dyn_bias = bias;
        if (accel >= bias  - 1.0F) dyn_bias = -bias;

        accel = std::lerp(accel, dyn_bias, GetFrameTime() * 1.5F);

        // Platform (blue)
        {
            PxVec3 platform_pos(0, accel, 0);
            platform->setGlobalPose(PxTransform(platform_pos));
            const PxTransform transform = platform->getGlobalPose();

            PxVec3 axisVec; float angle;
            transform.q.toRadiansAndUnitAxis(angle, axisVec);
            const Vector3 position = { transform.p.x, transform.p.y, transform.p.z };
            const Vector3 axis = { axisVec.x, axisVec.y, axisVec.z };
            DrawModelEx(platform_model, position, axis, (angle * RAD2DEG), { 1.0F, 1.0F, 1.0F }, BLUE);
        }

        // Pusher (purple)
        {
            static float i = 0.0F;
            PxVec3 pusher_pos(0, accel + 1.0F, 0);
            PxQuat rotation(PxPi * (i += GetFrameTime()), {0.0F, 1.0F, 0.0F});
            pusher->setGlobalPose(PxTransform(pusher_pos, rotation));
            const PxTransform transform = pusher->getGlobalPose();

            PxVec3 axisVec; float angle;
            transform.q.toRadiansAndUnitAxis(angle, axisVec);
            const Vector3 position = { transform.p.x, transform.p.y, transform.p.z };
            const Vector3 axis = { axisVec.x, axisVec.y, axisVec.z };
            DrawModelEx(pusher_model, position, axis, (angle * RAD2DEG), { 1.0F, 1.0F, 1.0F }, PURPLE);
        }

        // Dynamic cubes (red)
        for (int i = 0; i < TOTAL_CUBES; i++) {
            const PxTransform transform = cube_dynamic[i]->getGlobalPose();

            PxVec3 axisVec; float angle;
            transform.q.toRadiansAndUnitAxis(angle, axisVec);
            const Vector3 position = { transform.p.x, transform.p.y, transform.p.z };
            const Vector3 axis = { axisVec.x, axisVec.y, axisVec.z };
            DrawModelEx(cube_dynamic_model[i], position, axis, (angle * RAD2DEG), { 1.0F, 1.0F, 1.0F }, RED);

            if (transform.p.y < -20.0F) {
                cube_dynamic[i]->setGlobalPose({ (float)GetRandomValue(-5, 5), 10.0F, (float)GetRandomValue(-5, 5) });
                cube_dynamic[i]->setLinearVelocity({ 0.0F, 0.0F, 0.0F });
                cube_dynamic[i]->setAngularVelocity({ 0.0F, 0.0F, 0.0F });
                cube_dynamic[i]->clearForce();
                cube_dynamic[i]->clearTorque();
            }
        }

        // Dynamic spheres (green)
        for (int i = 0; i < TOTAL_SPHERES; i++) {
            const PxTransform transform = sphere_dynamic[i]->getGlobalPose();

            PxVec3 axisVec; float angle;
            transform.q.toRadiansAndUnitAxis(angle, axisVec);
            const Vector3 position = { transform.p.x, transform.p.y, transform.p.z };
            const Vector3 axis = { axisVec.x, axisVec.y, axisVec.z };
            DrawModelEx(sphere_dynamic_model[i], position, axis, (angle * RAD2DEG), { 1.0F, 1.0F, 1.0F }, GREEN);

            if (transform.p.y < -20.0F) {
                sphere_dynamic[i]->setGlobalPose({ (float)GetRandomValue(-5, 5), 10.0F, (float)GetRandomValue(-5, 5) });
                sphere_dynamic[i]->setLinearVelocity({ 0.0F, 0.0F, 0.0F });
                sphere_dynamic[i]->setAngularVelocity({ 0.0F, 0.0F, 0.0F });
                sphere_dynamic[i]->clearForce();
                sphere_dynamic[i]->clearTorque();
            }
        }

        EndMode3D();

        DrawFPS((GetRenderWidth() * 0.01F), (GetRenderHeight() * 0.01F));

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
