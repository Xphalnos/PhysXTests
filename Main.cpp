#include <raylib.h>
#include <raymath.h>
#include <PxPhysicsAPI.h>

using namespace physx;

constexpr int TOTAL_CUBES = 30;
constexpr int TOTAL_SPHERES = 30;

PxDefaultAllocator      gAllocator;
PxDefaultErrorCallback  gErrorCallback;
PxPhysics*              gPhysics    = nullptr;
PxScene*                gScene      = nullptr;
PxMaterial*             gMaterial   = nullptr;

void InitPhysX() {
    PxFoundation* gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0F, -9.81F, 0.0F);

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
        .position   = { 10.0F, 10.0F, 10.0F },
        .target     = { 0.0F, 0.0F, 0.0F },
        .up         = { 0.0F, 1.0F, 0.0F },
        .fovy       = 50.0F,
        .projection = CAMERA_PERSPECTIVE
    };

    InitPhysX();

    // Static Cube
    constexpr Vector3 cube_static_size = { 10.0F, 0.5F, 10.0F };
    const Model cube_static_model = LoadModelFromMesh(GenMeshCube(cube_static_size.x, cube_static_size.y, cube_static_size.z));
    const PxRigidStatic* cube_static = CreateStaticCube({ 0.0F, 0.0F, 0.0F }, (cube_static_size * 0.5F));

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

        // Static cube (blue)
        {
            const PxTransform transform = cube_static->getGlobalPose();

            PxVec3 axisVec; float angle;
            transform.q.toRadiansAndUnitAxis(angle, axisVec);
            const Vector3 position = { transform.p.x, transform.p.y, transform.p.z };
            const Vector3 axis = { axisVec.x, axisVec.y, axisVec.z };
            DrawModelEx(cube_static_model, position, axis, (angle * RAD2DEG), { 1.0F, 1.0F, 1.0F }, BLUE);
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
