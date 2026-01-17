#include <raylib.h>
#include <raymath.h>
#include <PxPhysicsAPI.h>

using namespace physx;

constexpr int TOTAL_CUBES = 50;

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

    sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(3); // Cores for Physics
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    gScene = gPhysics->createScene(sceneDesc);

    gMaterial = gPhysics->createMaterial(0.5F, 0.5F, 0.6F);
}

PxRigidStatic* CreateStaticCube(const PxVec3& pos, const Vector3& size) {
    PxTransform transform(pos);
    PxBoxGeometry geometry(size.x, size.y, size.z);

    PxRigidStatic* body = PxCreateStatic(*gPhysics, transform, geometry, *gMaterial);
    gScene->addActor(*body);

    return body;
}

PxRigidDynamic* CreateDynamicCube(const PxVec3& pos, const Vector3& size, const PxReal& mass) {
    PxTransform transform(pos);
    PxBoxGeometry geometry(size.x, size.y, size.z);

    PxRigidDynamic* body = PxCreateDynamic(*gPhysics, transform, geometry, *gMaterial, mass);
    gScene->addActor(*body);

    return body;
}

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
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
    constexpr Vector3 cube_dynamic_size = { 1.0F, 1.0F, 1.0F };
    Model cube_dynamic_model[TOTAL_CUBES];
    PxRigidDynamic* cube_dynamic[TOTAL_CUBES];

    for (int i = 0; i < TOTAL_CUBES; i++) {
        cube_dynamic_model[i] = LoadModelFromMesh(GenMeshCube(cube_dynamic_size.x, cube_dynamic_size.y, cube_dynamic_size.z));
        cube_dynamic[i] = CreateDynamicCube({ (float)GetRandomValue(-5, 5), (5.0F + (i * 2.0F)), (float)GetRandomValue(-5, 5) }, (cube_dynamic_size * 0.5F), 100.0F);
    }

    while (!WindowShouldClose()) {
        gScene->simulate(GetFrameTime());
        gScene->fetchResults(true);

        BeginDrawing();
        ClearBackground(DARKGRAY);
        UpdateCamera(&camera, CAMERA_ORBITAL);
        BeginMode3D(camera);

        // Static cube (red)
        const PxTransform tStatic = cube_static->getGlobalPose();
        const Vector3 staticPos = { tStatic.p.x, tStatic.p.y, tStatic.p.z };
        PxVec3 staticAxisVec; float staticAngle;
        tStatic.q.toRadiansAndUnitAxis(staticAngle, staticAxisVec);
        staticAngle *= RAD2DEG;
        const Vector3 staticAxis = { staticAxisVec.x, staticAxisVec.y, staticAxisVec.z };
        DrawModelEx(cube_static_model, staticPos, staticAxis, staticAngle, { 1.0f, 1.0f, 1.0f }, BLUE);

        // Dynamic cube (blue)
        for (int i = 0; i < TOTAL_CUBES; i++) {
            const PxTransform tDynamic = cube_dynamic[i]->getGlobalPose();
            const Vector3 dynamicPos = { tDynamic.p.x, tDynamic.p.y, tDynamic.p.z };
            PxVec3 dynamicAxisVec; float dynamicAngle;
            tDynamic.q.toRadiansAndUnitAxis(dynamicAngle, dynamicAxisVec);
            dynamicAngle *= RAD2DEG;
            const Vector3 dynamicAxis = { dynamicAxisVec.x, dynamicAxisVec.y, dynamicAxisVec.z };
            DrawModelEx(cube_dynamic_model[i], dynamicPos, dynamicAxis, dynamicAngle, { 1.0f, 1.0f, 1.0f }, RED);
        }

        EndMode3D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
