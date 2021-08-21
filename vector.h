#pragma once

#include <d3d9.h>
#include <math.h>

#define UCONST_Pi 3.1415926535
#define RadianToURotation 180.0f / UCONST_Pi

class Vector3 {
public:
    Vector3() : x(0.f), y(0.f), z(0.f) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    ~Vector3() {}

    float x;
    float y;
    float z;

    inline float Dot(Vector3 v) {
        return x * v.x + y * v.y + z * v.z;
    }

    inline float Distance(Vector3 v) {
        return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
    }

    Vector3 operator+(Vector3 v) {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(Vector3 v) {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    inline float Length() {
        return sqrtf((x * x) + (y * y) + (z * z));
    }

};

struct Vector2 {
public:
    float x;
    float y;

    inline Vector2() : x(0), y(0) {}
    inline Vector2(float x, float y) : x(x), y(y) {}

    inline float Distance(Vector2 v) {
        return sqrtf(((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y)));
    }

    inline Vector2 operator+(const Vector2& v) const {
        return Vector2(x + v.x, y + v.y);
    }

    inline Vector2 operator-(const Vector2& v) const {
        return Vector2(x - v.x, y - v.y);
    }
};

struct FQuat {
    float x;
    float y;
    float z;
    float w;
};

struct FTransform {
    FQuat rot;
    Vector3 translation;
    char pad[4];
    Vector3 scale;
    char pad1[4];
    D3DMATRIX ToMatrixWithScale() {
        D3DMATRIX m;
        m._41 = translation.x;
        m._42 = translation.y;
        m._43 = translation.z;

        float x2 = rot.x + rot.x;
        float y2 = rot.y + rot.y;
        float z2 = rot.z + rot.z;

        float xx2 = rot.x * x2;
        float yy2 = rot.y * y2;
        float zz2 = rot.z * z2;
        m._11 = (1.0f - (yy2 + zz2)) * scale.x;
        m._22 = (1.0f - (xx2 + zz2)) * scale.y;
        m._33 = (1.0f - (xx2 + yy2)) * scale.z;

        float yz2 = rot.y * z2;
        float wx2 = rot.w * x2;
        m._32 = (yz2 - wx2) * scale.z;
        m._23 = (yz2 + wx2) * scale.y;

        float xy2 = rot.x * y2;
        float wz2 = rot.w * z2;
        m._21 = (xy2 - wz2) * scale.y;
        m._12 = (xy2 + wz2) * scale.x;

        float xz2 = rot.x * z2;
        float wy2 = rot.w * y2;
        m._31 = (xz2 + wy2) * scale.z;
        m._13 = (xz2 - wy2) * scale.x;

        m._14 = 0.0f;
        m._24 = 0.0f;
        m._34 = 0.0f;
        m._44 = 1.0f;

        return m;
    }
};

struct FMinimalViewInfo
{
    Vector3 Location; //+ 0x1260
    Vector3 Rotation; //+ 0x126C
    float FOV;     //+ 0x1278
};

class FRotator
{
public:
    float Pitch = 0.f;
    float Yaw = 0.f;
    float Roll = 0.f;

    D3DMATRIX GetAxes() {
        auto tempMatrix = Matrix();
        return tempMatrix;
    }

    D3DMATRIX Matrix(Vector3 origin = Vector3(0, 0, 0)) {
        float radPitch = (Pitch * float(UCONST_Pi) / 180.f);
        float radYaw = (Yaw * float(UCONST_Pi) / 180.f);
        float radRoll = (Roll * float(UCONST_Pi) / 180.f);
        float SP = sinf(radPitch);
        float CP = cosf(radPitch);
        float SY = sinf(radYaw);
        float CY = cosf(radYaw);
        float SR = sinf(radRoll);
        float CR = cosf(radRoll);

        D3DMATRIX matrix;
        matrix.m[0][0] = CP * CY;
        matrix.m[0][1] = CP * SY;
        matrix.m[0][2] = SP;
        matrix.m[0][3] = 0.f;

        matrix.m[1][0] = SR * SP * CY - CR * SY;
        matrix.m[1][1] = SR * SP * SY + CR * CY;
        matrix.m[1][2] = -SR * CP;
        matrix.m[1][3] = 0.f;

        matrix.m[2][0] = -(CR * SP * CY + SR * SY);
        matrix.m[2][1] = CY * SR - CR * SP * SY;
        matrix.m[2][2] = CR * CP;
        matrix.m[2][3] = 0.f;

        matrix.m[3][0] = origin.x;
        matrix.m[3][1] = origin.y;
        matrix.m[3][2] = origin.z;
        matrix.m[3][3] = 1.f;

        return matrix;
    }
};

D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
    D3DMATRIX pOut;
    pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
    pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
    pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
    pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
    pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
    pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
    pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
    pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
    pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
    pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
    pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
    pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
    pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
    pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
    pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
    pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

    return pOut;
}

D3DMATRIX toMatrix(Vector3 Rotation, Vector3 origin = Vector3(0, 0, 0))
{
    float Pitch = (Rotation.x * float(M_PI) / 180.f);
    float Yaw = (Rotation.y * float(M_PI) / 180.f);
    float Roll = (Rotation.z * float(M_PI) / 180.f);

    float SP = sinf(Pitch);
    float CP = cosf(Pitch);
    float SY = sinf(Yaw);
    float CY = cosf(Yaw);
    float SR = sinf(Roll);
    float CR = cosf(Roll);

    D3DMATRIX Matrix;
    Matrix._11 = CP * CY;
    Matrix._12 = CP * SY;
    Matrix._13 = SP;
    Matrix._14 = 0.f;

    Matrix._21 = SR * SP * CY - CR * SY;
    Matrix._22 = SR * SP * SY + CR * CY;
    Matrix._23 = -SR * CP;
    Matrix._24 = 0.f;

    Matrix._31 = -(CR * SP * CY + SR * SY);
    Matrix._32 = CY * SR - CR * SP * SY;
    Matrix._33 = CR * CP;
    Matrix._34 = 0.f;

    Matrix._41 = origin.x;
    Matrix._42 = origin.y;
    Matrix._43 = origin.z;
    Matrix._44 = 1.f;

    return Matrix;
}

Vector2 worldToScreen(Vector3 world_location, Vector3 position, Vector3 rotation, float fov)
{
    Vector2 screen_location = Vector2(0, 0);

    D3DMATRIX tempMatrix = toMatrix(rotation);

    Vector3 vAxisX, vAxisY, vAxisZ;

    vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
    vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
    vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

    Vector3 vDelta = world_location - position;
    Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

    if (vTransformed.z < .1f)
        vTransformed.z = .1f;

    float FovAngle = fov;
    float ScreenCenterX = 1920 / 2.0f;
    float ScreenCenterY = 1080 / 2.0f;

    screen_location.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
    screen_location.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

    return screen_location;
}
