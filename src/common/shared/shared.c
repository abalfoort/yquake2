/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * Support functions, linked into client, server, renderer and game.
 *
 * =======================================================================
 */

#include <ctype.h>

#ifndef _MSC_VER
#include <strings.h>
#endif

#include "../header/shared.h"

#define DEG2RAD(a) (a * M_PI) / 180.0F

vec3_t vec3_origin = {0, 0, 0};

/* ============================================================================ */

void
RotatePointAroundVector(vec3_t dst, const vec3_t dir,
		const vec3_t point, float degrees)
{
	float m[3][3];
	float im[3][3];
	float zrot[3][3];
	float tmpmat[3][3];
	float rot[3][3];
	int i;
	vec3_t vr, vup, vf;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector(vr, dir);
	CrossProduct(vr, vf, vup);

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy(im, m, sizeof(im));

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset(zrot, 0, sizeof(zrot));
	zrot[2][2] = 1.0F;

	zrot[0][0] = (float)cos(DEG2RAD(degrees));
	zrot[0][1] = (float)sin(DEG2RAD(degrees));
	zrot[1][0] = (float)-sin(DEG2RAD(degrees));
	zrot[1][1] = (float)cos(DEG2RAD(degrees));

	R_ConcatRotations(m, zrot, tmpmat);
	R_ConcatRotations(tmpmat, im, rot);

	for (i = 0; i < 3; i++)
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] *
				 point[2];
	}
}

void
AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle;
	static float sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * (M_PI * 2 / 360);
	sy = (float)sin(angle);
	cy = (float)cos(angle);
	angle = angles[PITCH] * (M_PI * 2 / 360);
	sp = (float)sin(angle);
	cp = (float)cos(angle);
	angle = angles[ROLL] * (M_PI * 2 / 360);
	sr = (float)sin(angle);
	cr = (float)cos(angle);

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = (-1 * sr * sp * cy + - 1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + - 1 * cr * cy);
		right[2] = -1 * sr * cp;
	}

	if (up)
	{
		up[0] = (cr * sp * cy + - sr * -sy);
		up[1] = (cr * sp * sy + - sr * cy);
		up[2] = cr * cp;
	}
}

void
AngleVectors2(const vec3_t value1, vec3_t angles)
{
	float yaw, pitch;

	if ((value1[1] == 0) && (value1[0] == 0))
	{
		yaw = 0;

		if (value1[2] > 0)
		{
			pitch = 90;
		}

		else
		{
			pitch = 270;
		}
	}
	else
	{
		float forward;

		if (value1[0])
		{
			yaw = ((float)atan2(value1[1], value1[0]) * 180 / M_PI);
		}

		else if (value1[1] > 0)
		{
			yaw = 90;
		}

		else
		{
			yaw = 270;
		}

		if (yaw < 0)
		{
			yaw += 360;
		}

		forward = (float)sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = ((float)atan2(value1[2], forward) * 180 / M_PI);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

void
ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/* assumes "src" is normalized */
void
PerpendicularVector(vec3_t dst, const vec3_t src)
{
	int pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/* find the smallest magnitude axially aligned vector */
	for (pos = 0, i = 0; i < 3; i++)
	{
		if (fabs(src[i]) < minelem)
		{
			pos = i;
			minelem = (float)fabs(src[i]);
		}
	}

	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/* project the point onto the plane defined by src */
	ProjectPointOnPlane(dst, tempvec, src);

	/* normalize the result */
	VectorNormalize(dst);
}

void
R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}

void
R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
				in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
				in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
				in1[2][2] * in2[2][3] + in1[2][3];
}

/* ============================================================================ */

float
Q_fabs(float f)
{
	int tmp = *(int *)&f;

	tmp &= 0x7FFFFFFF;
	return *(float *)&tmp;
}

float
LerpAngle(float a2, float a1, float frac)
{
	if (a1 - a2 > 180)
	{
		a1 -= 360;
	}

	if (a1 - a2 < -180)
	{
		a1 += 360;
	}

	return a2 + frac * (a1 - a2);
}

float
anglemod(float a)
{
	a = (360.0 / 65536) * ((int)(a * (65536 / 360.0)) & 65535);
	return a;
}

/*
 * This is the slow, general version
 */
int
BoxOnPlaneSide2(vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int i;
	float dist1, dist2;
	int sides;
	vec3_t corners[2];

	for (i = 0; i < 3; i++)
	{
		if (p->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}

	dist1 = DotProduct(p->normal, corners[0]) - p->dist;
	dist2 = DotProduct(p->normal, corners[1]) - p->dist;
	sides = 0;

	if (dist1 >= 0)
	{
		sides = 1;
	}

	if (dist2 < 0)
	{
		sides |= 2;
	}

	return sides;
}

/*
 * Returns 1, 2, or 1 + 2
 */
int
BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float dist1, dist2;
	int sides;

	/* fast axial cases */
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
		{
			return 1;
		}

		if (p->dist >= emaxs[p->type])
		{
			return 2;
		}

		return 3;
	}

	/* general case */
	switch (p->signbits)
	{
		case 0:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] +
					p->normal[2] * emins[2];
			break;
		case 1:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] +
					p->normal[2] * emins[2];
			break;
		case 2:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] +
					p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emins[2];
			break;
		case 3:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] +
					p->normal[2] * emaxs[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emins[2];
			break;
		case 4:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emins[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] +
					p->normal[2] * emaxs[2];
			break;
		case 5:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emins[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] +
					p->normal[2] * emaxs[2];
			break;
		case 6:
			dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] +
					p->normal[2] * emins[2];
			dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emaxs[2];
			break;
		case 7:
			dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] +
					p->normal[2] * emins[2];
			dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] +
					p->normal[2] * emaxs[2];
			break;
		default:
			dist1 = dist2 = 0;
			break;
	}

	sides = 0;

	if (dist1 >= p->dist)
	{
		sides = 1;
	}

	if (dist2 < p->dist)
	{
		sides |= 2;
	}

	return sides;
}

void
ClearBounds(vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void
AddPointToBounds(vec3_t v, vec3_t mins, vec3_t maxs)
{
	int i;
	vec_t val;

	for (i = 0; i < 3; i++)
	{
		val = v[i];

		if (val < mins[i])
		{
			mins[i] = val;
		}

		if (val > maxs[i])
		{
			maxs[i] = val;
		}
	}
}

void
ClosestPointOnBounds(const vec3_t p, const vec3_t amin, const vec3_t amax, vec3_t out)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		if (amin[i] > p[i])
		{
			out[i] = amin[i];
		}
		else if (amax[i] < p[i])
		{
			out[i] = amax[i];
		}
		else
		{
			out[i] = p[i];
		}
	}
}

int
VectorCompare(vec3_t v1, vec3_t v2)
{
	if ((v1[0] != v2[0]) || (v1[1] != v2[1]) || (v1[2] != v2[2]))
	{
		return 0;
	}

	return 1;
}

vec_t
VectorNormalize(vec3_t v)
{
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = (float)sqrt(length);

	if (length)
	{
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

vec_t
VectorNormalize2(vec3_t v, vec3_t out)
{
	VectorCopy(v, out);

	return VectorNormalize(out);
}

void
VectorMA(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

vec_t
_DotProduct(vec3_t v1, vec3_t v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void
_VectorSubtract(vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}

void
_VectorAdd(vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

void
_VectorCopy(vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void
CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

double sqrt(double x);

vec_t
VectorLength(vec3_t v)
{
	return sqrtf((v[0] * v[0]) +
               (v[1] * v[1]) +
	       (v[2] * v[2]));
}

void
VectorInverse(vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void
VectorScale(vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

int
Q_log2(int val)
{
	int answer = 0;

	while (val >>= 1)
	{
		answer++;
	}

	return answer;
}

/* ==================================================================================== */

char *
COM_SkipPath(char *pathname)
{
	char *last;

	last = pathname;

	while (*pathname)
	{
		if (*pathname == '/')
		{
			last = pathname + 1;
		}

		pathname++;
	}

	return last;
}

void
COM_StripExtension(char *in, char *out)
{
	while (*in && *in != '.')
	{
		*out++ = *in++;
	}

	*out = 0;
}

const char *
COM_FileExtension(const char *in)
{
	const char *ext = strrchr(in, '.');

	if (!ext || ext == in)
	{
		return "";
	}

	return ext + 1;
}

void
COM_FileBase(char *in, char *out)
{
	char *s, *s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
	{
		s--;
	}

	for (s2 = s; s2 != in && *s2 != '/'; s2--)
	{
	}

	if (s - s2 < 2)
	{
		out[0] = 0;
	}
	else
	{
		s--;
		memcpy(out, s2 + 1, s - s2);
		out[s - s2] = 0;
	}
}

/*
 * Returns the path up to, but not including the last /
 */
void
COM_FilePath(const char *in, char *out)
{
	const char *s;

	s = in + strlen(in) - 1;

	while (s != in && *s != '/')
	{
		s--;
	}

	memcpy(out, in, s - in);
	out[s - in] = 0;
}

void
COM_DefaultExtension(char *path, const char *extension)
{
	char *src;

	/* */
	/* if path doesn't have a .EXT, append extension */
	/* (extension should include the .) */
	/* */
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
		{
			return;                 /* it has an extension */
		}

		src--;
	}

	strcat(path, extension);
}

/*
 * ============================================================================
 *
 *                  BYTE ORDER FUNCTIONS
 *
 * ============================================================================
 */

qboolean bigendien;

/* can't just use function pointers, or dll linkage can
   mess up when qcommon is included in multiple places */
short (*_BigShort)(short l);
short (*_LittleShort)(short l);
int (*_BigLong)(int l);
int (*_LittleLong)(int l);
float (*_BigFloat)(float l);
float (*_LittleFloat)(float l);

short
BigShort(short l)
{
	return _BigShort(l);
}

short
LittleShort(short l)
{
	return _LittleShort(l);
}

int
BigLong(int l)
{
	return _BigLong(l);
}

int
LittleLong(int l)
{
	return _LittleLong(l);
}

float
BigFloat(float l)
{
	return _BigFloat(l);
}

float
LittleFloat(float l)
{
	return _LittleFloat(l);
}

short
ShortSwap(short l)
{
	byte b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short
ShortNoSwap(short l)
{
	return l;
}

int
LongSwap(int l)
{
	byte b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int
LongNoSwap(int l)
{
	return l;
}

float
FloatSwap(float f)
{
	union
	{
		float f;
		byte b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float
FloatNoSwap(float f)
{
	return f;
}

void
Swap_Init(void)
{
	byte swaptest[2] = {1, 0};
	short swapTestShort;
	YQ2_STATIC_ASSERT(sizeof(short) == 2, "invalid short size");
	memcpy(&swapTestShort, swaptest, 2);

	/* set the byte swapping variables in a portable manner */
	if (swapTestShort == 1)
	{
		bigendien = false;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
		Com_Printf("Byte ordering: little endian\n\n");
	}
	else
	{
		bigendien = true;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
		Com_Printf("Byte ordering: big endian\n\n");
	}

	if (LittleShort(swapTestShort) != 1)
		assert("Error in the endian conversion!");
}

/*
 * does a varargs printf into a temp buffer, so I don't
 * need to have varargs versions of all text functions.
 */
char *
va(const char *format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsnprintf(string, 1024, format, argptr);
	va_end(argptr);

	return string;
}

char com_token[MAX_TOKEN_CHARS];

/*
 * Parse a token out of a string
 */
char *
COM_Parse(char **data_p)
{
	int c;
	int len;
	char *data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	if (!data)
	{
		*data_p = NULL;
		return "";
	}

skipwhite:

	while ((c = *data) <= ' ')
	{
		if (c == 0)
		{
			*data_p = NULL;
			return "";
		}

		data++;
	}

	/* skip // comments */
	if ((c == '/') && (data[1] == '/'))
	{
		while (*data && *data != '\n')
		{
			data++;
		}

		goto skipwhite;
	}

	/* handle quoted strings specially */
	if (c == '\"')
	{
		data++;

		while (1)
		{
			c = *data++;

			if ((c == '\"') || !c)
			{
				goto done;
			}

			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	/* parse a regular word */
	do
	{
		if (len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}

		data++;
		c = *data;
	}
	while (c > 32);

done:
	if (len == MAX_TOKEN_CHARS)
	{
		len = 0;
	}

	com_token[len] = 0;

	*data_p = data;
	return com_token;
}

static int paged_total = 0;

void
Com_PageInMemory(byte *buffer, int size)
{
	int i;

	for (i = size - 1; i > 0; i -= 4096)
	{
		paged_total += buffer[i];
	}
}

/*
 * ============================================================================
 *
 *                  LIBRARY REPLACEMENT FUNCTIONS
 *
 * ============================================================================
 */

int
Q_stricmp(const char *s1, const char *s2)
{
#ifdef _MSC_VER
	return stricmp(s1, s2);
#else
	return strcasecmp(s1, s2);
#endif
}

int
Q_strncasecmp(const char *s1, const char *s2, int n)
{
	int c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
		{
			return 0; /* strings are equal until end point */
		}

		if (c1 != c2)
		{
			if ((c1 >= 'a') && (c1 <= 'z'))
			{
				c1 -= ('a' - 'A');
			}

			if ((c2 >= 'a') && (c2 <= 'z'))
			{
				c2 -= ('a' - 'A');
			}

			if (c1 != c2)
			{
				return -1; /* strings not equal */
			}
		}
	}
	while (c1);

	return 0; /* strings are equal */
}

char *Q_strcasestr(const char *haystack, const char *needle)
{
	size_t len = strlen(needle);

	for (; *haystack; haystack++)
	{
		if (!Q_strncasecmp(haystack, needle, len))
		{
			return (char *)haystack;
		}
	}
	return 0;
}

int
Q_strcasecmp(const char *s1, const char *s2)
{
	return Q_strncasecmp(s1, s2, 99999);
}

void
Com_sprintf(char *dest, int size, char *fmt, ...)
{
	int len;
	va_list argptr;

	va_start(argptr, fmt);
	len = vsnprintf(dest, size, fmt, argptr);
	va_end(argptr);

	if (len >= size)
	{
		Com_Printf("Com_sprintf: overflow\n");
	}
}

char *
Q_strlwr ( char *s )
{
	char *p = s;

	while ( *s )
	{
		*s = tolower( (unsigned char)*s );
		s++;
	}

	return ( p );
}

int
Q_strlcpy(char *dst, const char *src, int size)
{
	const char *s = src;

	while (*s)
	{
		if (size > 1)
		{
			*dst++ = *s;
			size--;
		}
		s++;
	}
	if (size > 0)
	{
		*dst = '\0';
	}

	return s - src;
}

int
Q_strlcat(char *dst, const char *src, int size)
{
	char *d = dst;

	while (size > 0 && *d)
	{
		size--;
		d++;
	}

	return (d - dst) + Q_strlcpy(d, src, size);
}

void
Q_strdel(char *s, size_t i, size_t n)
{
	size_t len;

	if (!n)
	{
		return;
	}

	len = strlen(s);

	if (i >= len || n > (len - i))
	{
		return;
	}

	memmove(s + i, s + i + n, len - i);
	s[len - n] = '\0';
}

size_t
Q_strins(char *dest, const char *src, size_t i, size_t n)
{
	size_t dlen;
	size_t slen;

	if (!src || *src == '\0')
	{
		return 0;
	}

	slen = strlen(src);
	dlen = strlen(dest);

	if (i > dlen || (dlen + slen + 1) > n)
	{
		return 0;
	}

	memmove(dest + i + slen, dest + i, dlen - i + 1);
	memcpy(dest + i, src, slen);

	return slen;
}

qboolean
Q_strisnum(const char *s)
{
	for (; *s != '\0'; s++)
	{
		if (!isdigit(*s))
		{
			return false;
		}
	}

	return true;
}

/*
 * An unicode compatible fopen() Wrapper for Windows.
 */
#ifdef _WIN32
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
FILE *Q_fopen(const char *file, const char *mode)
{
	WCHAR wfile[MAX_OSPATH];
	WCHAR wmode[16];

	int len = MultiByteToWideChar(CP_UTF8, 0, file, -1, wfile, MAX_OSPATH);

	if (len > 0)
	{
		if (MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 16) > 0)
		{
			// make sure it's a regular file and not a directory or sth, see #394
			struct _stat buf;
			int statret = _wstat(wfile, &buf);
			if((statret == 0 && (buf.st_mode & _S_IFREG) != 0) || (statret == -1 && errno == ENOENT))
			{
				return _wfopen(wfile, wmode);
			}
		}
	}

	return NULL;
}
#else
#include <sys/stat.h>
#include <errno.h>
FILE *Q_fopen(const char *file, const char *mode)
{
	// make sure it's a regular file and not a directory or sth, see #394
	struct stat statbuf;
	int statret = stat(file, &statbuf);
	// (it's ok if it doesn't exist though, maybe we wanna write/create)
	if((statret == -1 && errno != ENOENT) || (statret == 0 && (statbuf.st_mode & S_IFREG) == 0))
	{
		return NULL;
	}
	return fopen(file, mode);
}
#endif

int
Q_sort_stricmp(const void *s1, const void *s2)
{
	return Q_stricmp(*(char**)s1, *(char**)s2);
}

int
Q_sort_strcomp(const void *s1, const void *s2)
{
	return strcmp(*(char **)s1, *(char **)s2);
}

/*
 * =====================================================================
 *
 * INFO STRINGS
 *
 * =====================================================================
 */

/*
 * Searches the string for the given
 * key and returns the associated value,
 * or an empty string.
 */
char *
Info_ValueForKey(char *s, char *key)
{
	char pkey[512];
	static char value[2][512]; /* use two buffers so compares
							     work without stomping on each other */
	static int valueindex;
	char *o;

	valueindex ^= 1;

	if (*s == '\\')
	{
		s++;
	}

	while (1)
	{
		o = pkey;

		while (*s != '\\')
		{
			if (!*s)
			{
				return "";
			}

			*o++ = *s++;
		}

		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}

		*o = 0;

		if (!strcmp(key, pkey))
		{
			return value[valueindex];
		}

		if (!*s)
		{
			return "";
		}

		s++;
	}
}

void
Info_RemoveKey(char *s, char *key)
{
	char *start;
	char pkey[512];
	char value[512];
	char *o;

	if (strstr(key, "\\"))
	{
		return;
	}

	while (1)
	{
		start = s;

		if (*s == '\\')
		{
			s++;
		}

		o = pkey;

		while (*s != '\\')
		{
			if (!*s)
			{
				return;
			}

			*o++ = *s++;
		}

		*o = 0;
		s++;

		o = value;

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}

		*o = 0;

		if (!strcmp(key, pkey))
		{
			memmove(start, s, strlen(s) + 1); /* remove this part */
			return;
		}

		if (!*s)
		{
			return;
		}
	}
}

/*
 * Some characters are illegal in info strings
 * because they can mess up the server's parsing
 */
qboolean
Info_Validate(char *s)
{
	if (strstr(s, "\""))
	{
		return false;
	}

	if (strstr(s, ";"))
	{
		return false;
	}

	return true;
}

void
Info_SetValueForKey(char *s, char *key, char *value)
{
	char newi[MAX_INFO_STRING], *v;
	int c;
	int maxsize = MAX_INFO_STRING;

	if (!key)
	{
		return;
	}

	if (strstr(key, "\\") || (value && strstr(value, "\\")))
	{
		Com_Printf("Can't use keys or values with a \\\n");
		return;
	}

	if (strstr(key, ";"))
	{
		Com_Printf("Can't use keys with a semicolon\n");
		return;
	}

	if (strstr(key, "\"") || (value && strstr(value, "\"")))
	{
		Com_Printf("Can't use keys or values with a \"\n");
		return;
	}

	if ((strlen(key) > MAX_INFO_KEY - 1) || (value && (strlen(value) > MAX_INFO_KEY - 1)))
	{
		Com_Printf("Keys and values must be < 64 characters.\n");
		return;
	}

	Info_RemoveKey(s, key);

	if (!value || !strlen(value))
	{
		return;
	}

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) >= maxsize)
	{
		Com_Printf("Info string length exceeded\n");
		return;
	}

	/* only copy ascii values */
	s += strlen(s);
	v = newi;

	while (*v)
	{
		c = *v++;
		c &= 127; /* strip high bits */

		if ((c >= 32) && (c < 127))
		{
			*s++ = c;
		}
	}

	*s = 0;
}
