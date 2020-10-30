#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "GL/glew.h"
#include "Math/float3x3.h"
#include "Math/Quat.h"

ModuleCamera::ModuleCamera()
{
    frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
    frustum.SetViewPlaneDistances(0.1f, 200.0f);
    frustum.SetHorizontalFovAndAspectRatio(DEGTORAD * 90.0f, 1.3f);

    frustum.SetPos(float3(0, 1, -2));
    frustum.SetFront(float3::unitZ);
    frustum.SetUp(float3::unitY);
}

ModuleCamera::~ModuleCamera()
{
}

bool ModuleCamera::Init()
{
    return true;
}

update_status ModuleCamera::PreUpdate()
{

    return UPDATE_CONTINUE;
}

update_status ModuleCamera::Update()
{
    Rotate(20 * DEGTORAD * App->GetDeltaTime(), 0, 0);

    return UPDATE_CONTINUE;
}

update_status ModuleCamera::PostUpdate()
{
    float4x4 projectionGL = GetProjectionMatrix();

    //Send the frustum projection matrix to OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(*projectionGL.v);

    float4x4 viewGL = GetViewMatrix();

    //Send the frustum view matrix to OpenGL
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(*viewGL.v);

    return UPDATE_CONTINUE;
}

bool ModuleCamera::CleanUp()
{
    return true;
}

void ModuleCamera::WindowResized(Sint32 width, Sint32 height)
{
    SetAspectRatio(width / (float) height);
}

void ModuleCamera::SetFOV(float h_fov)
{
    frustum.SetHorizontalFovAndAspectRatio(h_fov, frustum.AspectRatio());
}

void ModuleCamera::SetAspectRatio(float aspect_ratio)
{
    frustum.SetHorizontalFovAndAspectRatio(frustum.HorizontalFov(), aspect_ratio);
}

void ModuleCamera::SetPlaneDistances(float near_plane, float far_plane)
{
    frustum.SetViewPlaneDistances(near_plane, far_plane);
}

void ModuleCamera::SetPosition(float x, float y, float z)
{
    frustum.SetPos(vec(x, y, z));
}

void ModuleCamera::SetOrientation(float x, float y, float z)
{
    frustum.SetFront(float3::unitZ);
    frustum.SetUp(float3::unitY);
    Rotate(x, y, z);
}

void ModuleCamera::Rotate(float x, float y, float z)
{
    Rotate(Quat::FromEulerXYZ(x, y, z));
}

void ModuleCamera::Rotate(Quat rotation)
{
    Quat irotation = rotation.Inverted();
    vec up = frustum.Up();
    vec front = frustum.Front();
    Quat qup = Quat(up.x, up.y, up.z, 0);
    Quat qfront = Quat(front.x, front.y, front.z, 0);
    Quat nqup = rotation * qup * irotation;
    Quat nqfront = rotation * qfront * irotation;
    vec nup = vec(nqup.x, nqup.y, nqup.z);
    vec nfront = vec(nqfront.x, nqfront.y, nqfront.z);
    nup.Normalize();
    nfront.Normalize();
    frustum.SetUp(nup);
    frustum.SetFront(nfront);
}

void ModuleCamera::LookAt(float x, float y, float z)
{
    vec direction = vec(x, y, z) - frustum.Pos();
    direction.Normalize();
    Rotate(Quat::LookAt(frustum.Front(), direction, frustum.Up(), float3::unitY));
}

float4x4 ModuleCamera::GetProjectionMatrix()
{
    return frustum.ProjectionMatrix().Transposed();
}

float4x4 ModuleCamera::GetViewMatrix()
{
    return float4x4(frustum.ViewMatrix()).Transposed();
}
