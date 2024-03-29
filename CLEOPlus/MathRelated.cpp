#include "OpcodesCommon.h"
#include "CGeneral.h"
#include "PerlinNoise\SimplexNoise.h"
#include "Fx_c.h"
#include <algorithm>

OpcodeResult WINAPI GET_ANGLE_FROM_TWO_COORDS(CScriptThread* thread)
{
	float cx = CLEO_GetFloatOpcodeParam(thread);
	float cy = CLEO_GetFloatOpcodeParam(thread);
	float ex = CLEO_GetFloatOpcodeParam(thread);
	float ey = CLEO_GetFloatOpcodeParam(thread);

	float dx, dy, q1, q2, r;
	dy = ey;
	dy -= cy;
	dx = ex;
	dx -= cx;
	//to magnitude
	q1 = dx;
	q1 *= dx;
	q2 = dy;
	q2 *= dy;
	q1 += q2;
	r = sqrt(q1);
	dx /= r;
	dy /= r;

	float result = CGeneral::GetATanOfXY(dx, dy) * 57.295776f - 90.0f;

	while (result < 0.0) {
		result += 360.0;
	}
	if (_isnan(result)) {
		result = 0.0f;
	}

	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI QUAT_SLERP(CScriptThread* thread)
{
	CQuaternion *from = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	CQuaternion *to = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	float t = CLEO_GetFloatOpcodeParam(thread);
	CQuaternion *result = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	result->Slerp(*from, *to, t);
	return OR_CONTINUE;
}

/*
 * Functions taken from Tween.js - Licensed under the MIT license
 * at https://github.com/sole/tween.js
 */
const float PI = 3.14159265359f;

float EaseBounceOut(float k)
{
	if (k < (1.0f / 2.75f)) {
		return 7.5625f*k*k;
	}
	else if (k < (2.0f / 2.75f)) {
		return 7.5625f*(k -= (1.5f / 2.75f))*k + 0.75f;
	}
	else if (k < (2.5f / 2.75f)) {
		return 7.5625f *(k -= (2.25f / 2.75f))*k + 0.9375f;
	}
	else {
		return 7.5625f*(k -= (2.625f / 2.75f))*k + 0.984375f;
	}
}

OpcodeResult WINAPI EASE(CScriptThread* thread)
{
	float k = CLEO_GetFloatOpcodeParam(thread);
	int mode = CLEO_GetIntOpcodeParam(thread);
	int way = CLEO_GetIntOpcodeParam(thread);

	float result = 0.0f;

	switch (mode)
	{
		case 1: // Quadratic
		{
			switch (way)
			{
				case 0: //In
				{
					result = k * k;
					break;
				}
				case 1: //Out
				{
					result = k * (2.0f - k);
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = 0.5f*k*k;
					else result = -0.5f*((k -= 1.0f)*(k - 2.0f) - 1.0f);
					break;
				}
			}
			break;
		};
		case 2: // Cubic
		{
			switch (way)
			{
				case 0: //In
				{
					result = k * k*k;
					break;
				}
				case 1: //Out
				{
					result = 1.0f + ((k -= 1.0f)*k*k);
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = 0.5f*k*k*k;
					else result = 0.5f*((k -= 2.0f)*k*k + 2.0f);
					break;
				}
			}
			break;
		}
		case 3: // Quartic
		{
			switch (way)
			{
				case 0: //In
				{
					result = k * k*k*k;
					break;
				}
				case 1: //Out
				{
					result = 1.0f - ((k -= 1.0f)*k*k*k);
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = 0.5f*k*k*k*k;
					else result = -0.5f*((k -= 2.0f)*k*k*k - 2.0f);
					break;
				}
			}
			break;
		}
		case 4: // Quintic
		{
			switch (way)
			{
				case 0: //In
				{
					result = k * k*k*k*k;
					break;
				}
				case 1: //Out
				{
					result = 1.0f - ((k -= 1.0f)*k*k*k*k);
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = 0.5f*k*k*k*k*k;
					result = 0.5f*((k -= 2.0f)*k*k*k*k + 2.0f);
					break;
				}
			}
			break;
		}
		case 5: // Sinusoidal
		{
			switch (way)
			{
				case 0: //In
				{
					result = 1.0f - cosf(k*PI / 2.0f);
					break;
				}
				case 1: //Out
				{
					result = sinf(k*PI / 2.0f);
					break;
				}
				case 2: //InOut
				{
					result = 0.5f*(1.0f - cosf(PI*k));
					break;
				}
			}
			break;
		}
		case 6: // Exponential
		{
			switch (way)
			{
				case 0: //In
				{
					result = k == 0.0f ? 0.0f : powf(1024.0f, k - 1.0f);
					break;
				}
				case 1: //Out
				{
					result = k == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f*k);
					break;
				}
				case 2: //InOut
				{
					if (k == 0.0f) result = 0.0f;
					else if (k == 1.0f) result = 1.0f;
					else if ((k *= 2.0f) < 1.0f) result = 0.5f*powf(1024.0f, k - 1.0f);
					else result = 0.5f*(-powf(2.0f, -10.0f*(k - 1.0f)) + 2.0f);
					break;
				}
			}
			break;
		}
		case 7: // Circular
		{
			switch (way)
			{
				case 0: //In
				{
					result = 1.0f - sqrtf(1.0f - k * k);
					break;
				}
				case 1: //Out
				{
					result = sqrtf(1.0f - ((k -= 1.0f)*k));
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = -0.5f*(sqrtf(1.0f - k * k) - 1);
					else result = 0.5f*(sqrtf(1.0f - (k -= 2.0f)*k) + 1.0f);
					break;
				}
			}
			break;
		}
		case 8: // Elastic
		{
			switch (way)
			{
				case 0: //In
				{
					if (k == 0) result = 0;
					if (k == 1) result = 1;
					result = -powf(2.0f, 10.0f*(k -= 1.0f))*sinf((k - 0.1f)*(2.0f*PI) / 0.4f);
					break;
				}
				case 1: //Out
				{
					if (k == 0) result = 0;
					if (k == 1) result = 1;
					result = powf(2.0f, -10.0f*k)*sinf((k - 0.1f)*(2.0f*PI) / 0.4f) + 1.0f;
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = -0.5f*powf(2.0f, 10.0f*(k -= 1.0f))*sinf((k - 0.1f)*(2.0f*PI) / 0.4f);
					else result = powf(2.0f, -10.0f*(k -= 1.0f))*sinf((k - 0.1f)*(2.0f*PI) / 0.4f)*0.5f + 1.0f;
					break;
				}
			}
			break;
		}
		case 9: // Back
		{
			static float s = 1.70158f;
			static float s2 = 2.5949095f;

			switch (way)
			{
				case 0: //In
				{
					result = k * k*((s + 1.0f)*k - s);
					break;
				}
				case 1: //Out
				{
					result = (k -= 1.0f)*k*((s + 1.0f)*k + s) + 1.0f;
					break;
				}
				case 2: //InOut
				{
					if ((k *= 2.0f) < 1.0f) result = 0.5f*(k*k*((s2 + 1.0f)*k - s2));
					else result = 0.5f*((k -= 2.0f)*k*((s2 + 1.0f)*k + s2) + 2.0f);
					break;
				}
			}
			break;
		}
		case 10: // Bounce
		{
			switch (way)
			{
				case 0: //In
				{
					result = 1.0f - EaseBounceOut(1.0f - k);
					break;
				}
				case 1: //Out
				{
					EaseBounceOut(k);
					break;
				}
				case 2: //InOut
				{
					if (k < 0.5f) result = (1.0f - EaseBounceOut(1.0f - k * 2.0f))*0.5f;
					else result = EaseBounceOut(k * 2.0f - 1.0f)*0.5f + 0.5f;
					break;
				}
			}
			break;
		}
		default: // Linear
		{
			result = k;
			break;
		}
	}
	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI PERLIN_NOISE(CScriptThread* thread)
{
	float result = SimplexNoise::noise(CLEO_GetFloatOpcodeParam(thread));
	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI PERLIN_NOISE_FRACTAL(CScriptThread* thread)
{
	float f = CLEO_GetFloatOpcodeParam(thread);
	int octaves = CLEO_GetIntOpcodeParam(thread);
	float frequency = CLEO_GetFloatOpcodeParam(thread);
	float amplitude = CLEO_GetFloatOpcodeParam(thread);
	float lacunarity = CLEO_GetFloatOpcodeParam(thread);
	float persistence = CLEO_GetFloatOpcodeParam(thread);

	SimplexNoise *simplexNoise = new SimplexNoise(frequency, amplitude, lacunarity, persistence);
	float result = simplexNoise->fractal(octaves, f);
	delete simplexNoise;

	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI PERLIN_NOISE_FRACTAL_2D(CScriptThread* thread)
{
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	int octaves = CLEO_GetIntOpcodeParam(thread);
	float frequency = CLEO_GetFloatOpcodeParam(thread);
	float amplitude = CLEO_GetFloatOpcodeParam(thread);
	float lacunarity = CLEO_GetFloatOpcodeParam(thread);
	float persistence = CLEO_GetFloatOpcodeParam(thread);

	SimplexNoise *simplexNoise = new SimplexNoise(frequency, amplitude, lacunarity, persistence);
	float result = simplexNoise->fractal(octaves, x, y);
	delete simplexNoise;

	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI PERLIN_NOISE_FRACTAL_3D(CScriptThread* thread)
{
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float z = CLEO_GetFloatOpcodeParam(thread);
	int octaves = CLEO_GetIntOpcodeParam(thread);
	float frequency = CLEO_GetFloatOpcodeParam(thread);
	float amplitude = CLEO_GetFloatOpcodeParam(thread);
	float lacunarity = CLEO_GetFloatOpcodeParam(thread);
	float persistence = CLEO_GetFloatOpcodeParam(thread);

	SimplexNoise *simplexNoise = new SimplexNoise(frequency, amplitude, lacunarity, persistence);
	float result = simplexNoise->fractal(octaves, x, y, z);
	delete simplexNoise;

	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_COORD_FROM_ANGLED_DISTANCE(CScriptThread* thread)
{
	float x = CLEO_GetFloatOpcodeParam(thread);
	float y = CLEO_GetFloatOpcodeParam(thread);
	float angle = CLEO_GetFloatOpcodeParam(thread);
	float dist = CLEO_GetFloatOpcodeParam(thread);

	angle *= 0.0174533f;
	angle *= -1.0f;
	float eq1 = sin(angle);
	float eq2 = cos(angle);

	eq1 *= dist;
	eq2 *= dist;

	x += eq1;
	y += eq2;

	CLEO_SetFloatOpcodeParam(thread, x);
	CLEO_SetFloatOpcodeParam(thread, y);
	return OR_CONTINUE;
}

OpcodeResult WINAPI CLAMP_FLOAT(CScriptThread* thread)
{
	float f = CLEO_GetFloatOpcodeParam(thread);
	float min = CLEO_GetFloatOpcodeParam(thread);
	float max = CLEO_GetFloatOpcodeParam(thread);
	float result = 0.0f;
	if (!_isnan(f)) {
		result = clamp(f, min, max);
	}
	CLEO_SetFloatOpcodeParam(thread, result);
	return OR_CONTINUE;
}
 
OpcodeResult WINAPI CLAMP_INT(CScriptThread* thread)
{
	int i = CLEO_GetIntOpcodeParam(thread);
	int min = CLEO_GetIntOpcodeParam(thread);
	int max = CLEO_GetIntOpcodeParam(thread);
	CLEO_SetIntOpcodeParam(thread, clamp(i, min, max));
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_MATRIX_ROTATION_FROM_QUAT(CScriptThread* thread)
{
	CMatrix *matrix = (CMatrix *)CLEO_GetIntOpcodeParam(thread);
	CQuaternion *quat = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	matrix->SetRotate(*quat);
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_QUAT_FROM_MATRIX(CScriptThread* thread)
{
	RwMatrix *matrix = (RwMatrix *)CLEO_GetIntOpcodeParam(thread);
	CQuaternion *quat = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	quat->Set(*matrix);
	return OR_CONTINUE;
}

OpcodeResult WINAPI ROTATE_QUAT_ON_AXIS(CScriptThread* thread)
{
	RtQuat *quat;
	RwV3d axis;
	float angle;
	unsigned int combineOp;

	quat = (RtQuat *)CLEO_GetIntOpcodeParam(thread);
	axis.x = CLEO_GetFloatOpcodeParam(thread);
	axis.y = CLEO_GetFloatOpcodeParam(thread);
	axis.z = CLEO_GetFloatOpcodeParam(thread);
	angle = CLEO_GetFloatOpcodeParam(thread);
	combineOp = CLEO_GetIntOpcodeParam(thread);

	RtQuatRotate(quat, &axis, angle, (RwOpCombineType)combineOp);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_NORMALISED_QUAT(CScriptThread* thread)
{
	CQuaternion *dst, *src; CQuaternion quat;

	src = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	dst = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);

	quat = *src;
	quat.Normalise();
	*dst = quat;
	return OR_CONTINUE;
}

OpcodeResult WINAPI MULTIPLY_QUATS(CScriptThread* thread)
{
	CQuaternion *result, *quat1, *quat2;

	quat1 = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	quat2 = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	result = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);

	result->Multiply(*quat1, *quat2);
	return OR_CONTINUE;
}

OpcodeResult WINAPI INITIALISE_QUAT(CScriptThread* thread)
{
	CQuaternion *quat = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	quat->imag.x = CLEO_GetFloatOpcodeParam(thread);
	quat->imag.y = CLEO_GetFloatOpcodeParam(thread);
	quat->imag.z = CLEO_GetFloatOpcodeParam(thread);
	quat->real = CLEO_GetFloatOpcodeParam(thread);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_QUAT_ELEMENTS(CScriptThread* thread)
{
	CQuaternion *quat = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, quat->imag.x);
	CLEO_SetFloatOpcodeParam(thread, quat->imag.y);
	CLEO_SetFloatOpcodeParam(thread, quat->imag.z);
	CLEO_SetFloatOpcodeParam(thread, quat->real);
	return OR_CONTINUE;
}

float dotProduct(CVector *v1, CVector *v2) {
	return v1->z * v2->z + v1->y * v2->y + v1->x * v2->x;
}

OpcodeResult WINAPI CONVERT_DIRECTION_TO_QUAT(CScriptThread* thread)
{
	CVector dir;
	CQuaternion *quaternion = (CQuaternion *)CLEO_GetIntOpcodeParam(thread);

	dir.x = CLEO_GetFloatOpcodeParam(thread);
	dir.y = CLEO_GetFloatOpcodeParam(thread);
	dir.z = CLEO_GetFloatOpcodeParam(thread);

	CVector forward = dir;
	forward.Normalise();

	// Fixes a bug, I don't know...
	if (forward.x == 0.0f) forward.x = 0.001f;
	if (forward.y == 0.0f) forward.y = 0.001f;
	if (forward.z == 0.0f) forward.z = 0.001f;

	CVector up = { 0.0f, 0.0, 1.0f };

	up = up - (forward * dotProduct(&up, &forward));
	up.Normalise();

	///////////////////////

	CVector vector = forward;
	vector.Normalise();

	CVector vector2;
	vector2.Cross(up, vector);

	CVector vector3;
	vector3.Cross(vector, vector2);

	float m00 = vector2.x;
	float m01 = vector2.y;
	float m02 = vector2.z;
	float m10 = vector3.x;
	float m11 = vector3.y;
	float m12 = vector3.z;
	float m20 = vector.x;
	float m21 = vector.y;
	float m22 = vector.z;


	float num8 = (m00 + m11) + m22;

	if (num8 > 0.0f)
	{
		float num = (float)sqrt(num8 + 1.0f);
		quaternion->real = num * 0.5f;
		num = 0.5f / num;
		quaternion->imag.x = (m12 - m21) * num;
		quaternion->imag.y = (m20 - m02) * num;
		quaternion->imag.z = (m01 - m10) * num;
	}
	if ((m00 >= m11) && (m00 >= m22))
	{
		float num7 = (float)sqrt(((1.0f + m00) - m11) - m22);
		float num4 = 0.5f / num7;
		quaternion->imag.x = 0.5f * num7;
		quaternion->imag.y = (m01 + m10) * num4;
		quaternion->imag.z = (m02 + m20) * num4;
		quaternion->real = (m12 - m21) * num4;
	}
	if (m11 > m22)
	{
		float num6 = (float)sqrt(((1.0f + m11) - m00) - m22);
		float num3 = 0.5f / num6;
		quaternion->imag.x = (m10 + m01) * num3;
		quaternion->imag.y = 0.5f * num6;
		quaternion->imag.z = (m21 + m12) * num3;
		quaternion->real = (m20 - m02) * num3;
	}
	else {
		float num5 = (float)sqrt(((1.0f + m22) - m00) - m11);
		float num2 = 0.5f / num5;
		quaternion->imag.x = (m20 + m02) * num2;
		quaternion->imag.y = (m21 + m12) * num2;
		quaternion->imag.z = 0.5f * num5;
		quaternion->real = (m01 - m10) * num2;
	}

	quaternion->Normalise();
	return OR_CONTINUE;
}

OpcodeResult WINAPI ROTATE_MATRIX_ON_AXIS(CScriptThread* thread)
{
	RwMatrix *matrix; RwV3d axis; float angle; unsigned int combineOp;

	matrix = (RwMatrix *)CLEO_GetIntOpcodeParam(thread);

	axis.x = CLEO_GetFloatOpcodeParam(thread);
	axis.y = CLEO_GetFloatOpcodeParam(thread);
	axis.z = CLEO_GetFloatOpcodeParam(thread);
	angle = CLEO_GetFloatOpcodeParam(thread);
	combineOp = CLEO_GetIntOpcodeParam(thread);
	RwMatrixRotate(matrix, &axis, angle, (RwOpCombineType)combineOp);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MATRIX_X_ANGLE(CScriptThread* thread)
{
	RwMatrix *matrix = (RwMatrix *)CLEO_GetIntOpcodeParam(thread);
	float x = matrix->right.x;
	float y = matrix->right.y;
	float z = matrix->right.z;
	float angle = CGeneral::GetATanOfXY(z, sqrt(x * x + y * y)) * 57.295776f - 90.0f;
	while (angle < 0.0)
		angle += 360.0;
	CLEO_SetFloatOpcodeParam(thread, angle);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MATRIX_Y_ANGLE(CScriptThread* thread)
{
	RwMatrix *matrix = (RwMatrix *)CLEO_GetIntOpcodeParam(thread);
	float x = matrix->up.x;
	float y = matrix->up.y;
	float z = matrix->up.z;
	float angle = CGeneral::GetATanOfXY(z, sqrt(x * x + y * y)) * 57.295776f - 90.0f;
	while (angle < 0.0)
		angle += 360.0;
	CLEO_SetFloatOpcodeParam(thread, angle);
	return OR_CONTINUE;
}

OpcodeResult WINAPI GET_MATRIX_Z_ANGLE(CScriptThread* thread)
{
	RwMatrix *matrix = (RwMatrix *)CLEO_GetIntOpcodeParam(thread);
	float angle = CGeneral::GetATanOfXY(matrix->right.x, matrix->right.y) * 57.295776f - 90.0f;
	while (angle < 0.0)
		angle += 360.0;
	CLEO_SetFloatOpcodeParam(thread, angle);
	return OR_CONTINUE;
}

OpcodeResult WINAPI LERP(CScriptThread* thread)
{
	float a = CLEO_GetFloatOpcodeParam(thread);
	float b = CLEO_GetFloatOpcodeParam(thread);
	float t = CLEO_GetFloatOpcodeParam(thread);
	CLEO_SetFloatOpcodeParam(thread, (a * (1 - t) + b * t));
	return OR_CONTINUE;
}

OpcodeResult WINAPI SET_MATRIX_LOOK_DIRECTION(CScriptThread* thread)
{
	RwMatrixTag* matrix = (RwMatrixTag*)CLEO_GetIntOpcodeParam(thread);
	CVector origin;
	CVector direction;
	origin.x = CLEO_GetFloatOpcodeParam(thread);
	origin.y = CLEO_GetFloatOpcodeParam(thread);
	origin.z = CLEO_GetFloatOpcodeParam(thread);
	direction.x = CLEO_GetFloatOpcodeParam(thread);
	direction.y = CLEO_GetFloatOpcodeParam(thread);
	direction.z = CLEO_GetFloatOpcodeParam(thread);
	g_fx.CreateMatFromVec(matrix, &origin, &direction);
	return OR_CONTINUE;
}

