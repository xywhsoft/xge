/* Versioned particle data, using XRT's parsers and value ownership contracts. */
#include "xge_particle_internal.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum
{
	XP_IO_FLOAT,
	XP_IO_INT,
	XP_IO_UINT,
	XP_IO_BOOL,
	XP_IO_VEC,
	XP_IO_RANGE,
	XP_IO_RECT,
	XP_IO_COLOR,
	XP_IO_STRING
};
typedef struct xp_io_field
{
	const char *name;
	size_t offset;
	int type;
	size_t size;
} xp_io_field;
#define XP_FIELD(key, member, kind)                                                                          \
	{key, offsetof(xge_particle_emitter_t, member), kind, sizeof(((xge_particle_emitter_t *)0)->member)}
static const xp_io_field xp_fields[] = {
    XP_FIELD("name", sName, XP_IO_STRING),
    XP_FIELD("texture", sTexture, XP_IO_STRING),
    XP_FIELD("material", sMaterial, XP_IO_STRING),
    XP_FIELD("maxParticles", iMaxParticles, XP_IO_UINT),
    XP_FIELD("automatic", bAutomatic, XP_IO_BOOL),
    XP_FIELD("loop", bLoop, XP_IO_BOOL),
    XP_FIELD("space", iSpace, XP_IO_INT),
    XP_FIELD("shape", iShape, XP_IO_INT),
    XP_FIELD("delay", fDelay, XP_IO_FLOAT),
    XP_FIELD("duration", fDuration, XP_IO_FLOAT),
    XP_FIELD("rate", fRate, XP_IO_FLOAT),
    XP_FIELD("rateOverDistance", fRateOverDistance, XP_IO_FLOAT),
    XP_FIELD("offset", tOffset, XP_IO_VEC),
    XP_FIELD("shapeSize", tShapeSize, XP_IO_VEC),
    XP_FIELD("radius", fRadius, XP_IO_FLOAT),
    XP_FIELD("innerRadius", fInnerRadius, XP_IO_FLOAT),
    XP_FIELD("direction", fDirection, XP_IO_FLOAT),
    XP_FIELD("spread", fSpread, XP_IO_FLOAT),
    XP_FIELD("life", tLife, XP_IO_RANGE),
    XP_FIELD("speed", tSpeed, XP_IO_RANGE),
    XP_FIELD("size", tSize, XP_IO_RANGE),
    XP_FIELD("rotation", tRotation, XP_IO_RANGE),
    XP_FIELD("angularVelocity", tAngularVelocity, XP_IO_RANGE),
    XP_FIELD("startFrame", tStartFrame, XP_IO_RANGE),
    XP_FIELD("colorMin", iColorMin, XP_IO_COLOR),
    XP_FIELD("colorMax", iColorMax, XP_IO_COLOR),
    XP_FIELD("gravity", tGravity, XP_IO_VEC),
    XP_FIELD("drag", fDrag, XP_IO_FLOAT),
    XP_FIELD("radialAcceleration", fRadialAcceleration, XP_IO_FLOAT),
    XP_FIELD("tangentialAcceleration", fTangentialAcceleration, XP_IO_FLOAT),
    XP_FIELD("inheritVelocity", fInheritVelocity, XP_IO_FLOAT),
    XP_FIELD("noiseStrength", fNoiseStrength, XP_IO_FLOAT),
    XP_FIELD("noiseFrequency", fNoiseFrequency, XP_IO_FLOAT),
    XP_FIELD("noiseScroll", fNoiseScroll, XP_IO_FLOAT),
    XP_FIELD("aspect", fAspect, XP_IO_FLOAT),
    XP_FIELD("stretch", fStretch, XP_IO_FLOAT),
    XP_FIELD("alignVelocity", bAlignVelocity, XP_IO_BOOL),
    XP_FIELD("screenSpace", bScreenSpace, XP_IO_BOOL),
    XP_FIELD("blend", iBlend, XP_IO_INT),
    XP_FIELD("layer", iLayer, XP_IO_INT),
    XP_FIELD("order", iOrder, XP_IO_INT),
    XP_FIELD("textureRect", tTextureRect, XP_IO_RECT),
    XP_FIELD("columns", iColumns, XP_IO_INT),
    XP_FIELD("rows", iRows, XP_IO_INT),
    XP_FIELD("framesPerSecond", fFramesPerSecond, XP_IO_FLOAT),
    XP_FIELD("collision", iCollision, XP_IO_INT),
    XP_FIELD("collisionRadius", fCollisionRadius, XP_IO_FLOAT),
    XP_FIELD("restitution", fRestitution, XP_IO_FLOAT),
    XP_FIELD("friction", fFriction, XP_IO_FLOAT)};
#undef XP_FIELD
static xstrview xp_text(const char *s)
{
	return (xstrview){s, strlen(s)};
}
static xvalue *xp_get(const xvalue *object, const char *key)
{
	return xrtValueObjectGet(object, xp_text(key));
}
static int xp_key_equal(xstrview key, const char *text)
{
	return key.Size == strlen(text) && !memcmp(key.Data, text, key.Size);
}
static int xp_number(const xvalue *value, double *out)
{
	int64 si;
	uint64 ui;
	if (!value)
		return 0;
	if (xrtValueType(value) == XVALUE_FLOAT && xrtValueGetFloat(value, out))
		return isfinite(*out);
	if (xrtValueType(value) == XVALUE_INT && xrtValueGetInt(value, &si))
	{
		*out = (double)si;
		return 1;
	}
	if (xrtValueType(value) == XVALUE_UINT && xrtValueGetUInt(value, &ui))
	{
		*out = (double)ui;
		return 1;
	}
	return 0;
}
static int xp_array_size(const xvalue *value, size_t size)
{
	return value && xrtValueType(value) == XVALUE_ARRAY && xrtValueCount(value) == size;
}
static int xp_color_read(const xvalue *value, uint32_t *out)
{
	xstrview text;
	unsigned int r, g, b, a;
	double number;
	if (xrtValueType(value) == XVALUE_STRING && xrtValueGetString(value, &text))
	{
		char color[10];
		size_t i;
		if (text.Size != 9 || text.Data[0] != '#')
			return 0;
		for (i = 1; i < 9; i++)
			if (!((text.Data[i] >= '0' && text.Data[i] <= '9') ||
			      (text.Data[i] >= 'A' && text.Data[i] <= 'F') ||
			      (text.Data[i] >= 'a' && text.Data[i] <= 'f')))
				return 0;
		memcpy(color, text.Data, 9);
		color[9] = 0;
		if (sscanf(color, "#%2x%2x%2x%2x", &r, &g, &b, &a) != 4)
			return 0;
		*out = XGE_COLOR_RGBA(r, g, b, a);
		return 1;
	}
	if (!xp_number(value, &number) || number < 0 || number > UINT32_MAX || floor(number) != number)
		return 0;
	*out = (uint32_t)number;
	return 1;
}
static int xp_io_read_field(const xvalue *value, void *destination, const xp_io_field *field)
{
	double n;
	int i, count;
	bool boolean;
	xstrview text;
	switch (field->type)
	{
	case XP_IO_STRING:
		if (!xrtValueGetString(value, &text) || text.Size >= field->size || memchr(text.Data, 0, text.Size))
			return 0;
		memcpy(destination, text.Data, text.Size);
		((char *)destination)[text.Size] = 0;
		return 1;
	case XP_IO_BOOL:
		if (!xrtValueGetBool(value, &boolean))
			return 0;
		*(int *)destination = boolean;
		return 1;
	case XP_IO_COLOR:
		return xp_color_read(value, (uint32_t *)destination);
	case XP_IO_VEC:
	case XP_IO_RANGE:
	case XP_IO_RECT:
		count = field->type == XP_IO_RECT ? 4 : 2;
		if (!xp_array_size(value, (size_t)count))
			return 0;
		for (i = 0; i < count; i++)
		{
			float f;
			if (!xp_number(xrtValueArrayGet(value, (size_t)i), &n) || fabs(n) > 1e12)
				return 0;
			f = (float)n;
			memcpy((unsigned char *)destination + i * sizeof(float), &f, sizeof(float));
		}
		return 1;
	default:
		if (!xp_number(value, &n))
			return 0;
		if (field->type == XP_IO_FLOAT)
		{
			if (fabs(n) > 1e12)
				return 0;
			*(float *)destination = (float)n;
			return 1;
		}
		if (floor(n) != n)
			return 0;
		if (field->type == XP_IO_UINT)
		{
			if (n < 0 || n > UINT32_MAX)
				return 0;
			*(uint32_t *)destination = (uint32_t)n;
		}
		else
		{
			if (n < INT32_MIN || n > INT32_MAX)
				return 0;
			*(int *)destination = (int)n;
		}
		return 1;
	}
}
static int xp_read_curve(const xvalue *object, xge_particle_curve_t *curve)
{
	xvalue *keys;
	size_t i, j;
	double n;
	xstrview name;
	if (!object || xrtValueType(object) != XVALUE_OBJECT)
		return 0;
	for (i = 0; i < xrtValueCount(object); i++)
	{
		xrtValueObjectAt(object, i, &name);
		if (!xp_key_equal(name, "interpolation") && !xp_key_equal(name, "keys"))
			return 0;
	}
	if (xp_get(object, "interpolation"))
	{
		if (!xp_number(xp_get(object, "interpolation"), &n) || floor(n) != n || n < 0 || n > 2)
			return 0;
		curve->iInterpolation = (int)n;
	}
	keys = xp_get(object, "keys");
	if (!keys || xrtValueType(keys) != XVALUE_ARRAY || xrtValueCount(keys) > XGE_PARTICLE_MAX_KEYS)
		return 0;
	curve->iCount = (int)xrtValueCount(keys);
	for (i = 0; i < (size_t)curve->iCount; i++)
	{
		xvalue *key = xrtValueArrayGet(keys, i);
		size_t length = xrtValueCount(key);
		if (xrtValueType(key) != XVALUE_ARRAY || (length != 2 && length != 4))
			return 0;
		for (j = 0; j < length; j++)
		{
			float f;
			if (!xp_number(xrtValueArrayGet(key, j), &n) || fabs(n) > 1e12)
				return 0;
			f = (float)n;
			memcpy((unsigned char *)&curve->arrKeys[i] + j * sizeof(float), &f, sizeof(float));
		}
	}
	return 1;
}
static int xp_read_gradient(const xvalue *array, xge_particle_gradient_t *gradient)
{
	size_t i;
	double n;
	if (!array || xrtValueType(array) != XVALUE_ARRAY || xrtValueCount(array) > XGE_PARTICLE_MAX_KEYS)
		return 0;
	gradient->iCount = (int)xrtValueCount(array);
	for (i = 0; i < (size_t)gradient->iCount; i++)
	{
		xvalue *item = xrtValueArrayGet(array, i);
		if (!xp_array_size(item, 2) || !xp_number(xrtValueArrayGet(item, 0), &n) || n < 0 || n > 1 ||
		    !xp_color_read(xrtValueArrayGet(item, 1), &gradient->arrKeys[i].iColor))
			return 0;
		gradient->arrKeys[i].fTime = (float)n;
	}
	return 1;
}
static int xp_read_bursts(const xvalue *array, xge_particle_emitter_t *e)
{
	size_t i;
	double t, n;
	if (!array || xrtValueType(array) != XVALUE_ARRAY || xrtValueCount(array) > XGE_PARTICLE_MAX_BURSTS)
		return 0;
	e->iBurstCount = (int)xrtValueCount(array);
	for (i = 0; i < (size_t)e->iBurstCount; i++)
	{
		xvalue *item = xrtValueArrayGet(array, i);
		if (!xp_array_size(item, 2) || !xp_number(xrtValueArrayGet(item, 0), &t) || t < 0 || t > 1e12 ||
		    !xp_number(xrtValueArrayGet(item, 1), &n) || n < 0 || n > 1000000 || floor(n) != n)
			return 0;
		e->arrBursts[i] = (xge_particle_burst_t){(float)t, (uint32_t)n};
	}
	return 1;
}
static int xp_read_links(const xvalue *object, xge_particle_emitter_t *e)
{
	static const char *names[] = {"birth", "death", "collision"};
	size_t i, j;
	double n;
	xstrview key, subkey;
	if (!object || xrtValueType(object) != XVALUE_OBJECT)
		return 0;
	for (i = 0; i < xrtValueCount(object); i++)
	{
		xvalue *link = xrtValueObjectAt(object, i, &key);
		int index;
		for (index = 0; index < 3; index++)
			if (xp_key_equal(key, names[index]))
				break;
		if (index == 3 || xrtValueType(link) != XVALUE_OBJECT)
			return 0;
		for (j = 0; j < xrtValueCount(link); j++)
		{
			xvalue *value = xrtValueObjectAt(link, j, &subkey);
			if (!xp_number(value, &n))
				return 0;
			if (xp_key_equal(subkey, "emitter"))
			{
				if (floor(n) != n || n < -1 || n >= XGE_PARTICLE_MAX_EMITTERS)
					return 0;
				e->arrSubEmitters[index].iEmitter = (int)n;
			}
			else if (xp_key_equal(subkey, "count"))
			{
				if (floor(n) != n || n < 0 || n > 1000000)
					return 0;
				e->arrSubEmitters[index].iCount = (uint32_t)n;
			}
			else if (xp_key_equal(subkey, "inheritVelocity"))
			{
				if (fabs(n) > 1e12)
					return 0;
				e->arrSubEmitters[index].fInheritVelocity = (float)n;
			}
			else
				return 0;
		}
	}
	return 1;
}
static int xp_read_emitter(const xvalue *object, xge_particle_emitter_t *e, char *error, size_t capacity,
                           int index)
{
	size_t i, j;
	xstrview key = {0};
	int ok;
	if (!object || xrtValueType(object) != XVALUE_OBJECT)
		return 0;
	xgeParticleEmitterInit(e);
	for (i = 0; i < xrtValueCount(object); i++)
	{
		xvalue *value = xrtValueObjectAt(object, i, &key);
		ok = 0;
		for (j = 0; j < sizeof(xp_fields) / sizeof(xp_fields[0]); j++)
			if (xp_key_equal(key, xp_fields[j].name))
			{
				ok = xp_io_read_field(value, (unsigned char *)e + xp_fields[j].offset, &xp_fields[j]);
				break;
			}
		if (j == sizeof(xp_fields) / sizeof(xp_fields[0]))
		{
			if (xp_key_equal(key, "sizeOverLife"))
				ok = xp_read_curve(value, &e->tSizeOverLife);
			else if (xp_key_equal(key, "speedOverLife"))
				ok = xp_read_curve(value, &e->tSpeedOverLife);
			else if (xp_key_equal(key, "alphaOverLife"))
				ok = xp_read_curve(value, &e->tAlphaOverLife);
			else if (xp_key_equal(key, "colorOverLife"))
				ok = xp_read_gradient(value, &e->tColorOverLife);
			else if (xp_key_equal(key, "bursts"))
				ok = xp_read_bursts(value, e);
			else if (xp_key_equal(key, "subEmitters"))
				ok = xp_read_links(value, e);
		}
		if (!ok)
		{
			if (error && capacity)
				snprintf(error, capacity, "emitters[%d].%.*s: unknown field or invalid value", index,
				         (int)fmin(key.Size, 80), key.Data);
			return 0;
		}
	}
	return 1;
}
int xgeParticleDefinitionParse(xge_particle_definition *out, const char *text, size_t size, int xson,
                               char *error, size_t capacity)
{
	xvalue *root, *array;
	xge_particle_emitter_t *emitters = NULL;
	size_t i, count;
	double version;
	int result = XGE_ERROR_INVALID_ARGUMENT;
	if (error && capacity)
		error[0] = 0;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if (!text || !size || size > 1024u * 1024u)
		goto invalid;
	root = xson ? xrtXsonParse((xstrview){text, size}) : xrtJsonParse((xstrview){text, size});
	if (!root)
	{
		if (error && capacity)
			snprintf(error, capacity, "Invalid %s syntax or parser allocation failed",
			         xson ? "XSON" : "JSON");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	if (xrtValueType(root) != XVALUE_OBJECT)
		goto done;
	for (i = 0; i < xrtValueCount(root); i++)
	{
		xstrview key;
		xrtValueObjectAt(root, i, &key);
		if (!xp_key_equal(key, "version") && !xp_key_equal(key, "emitters"))
			goto done;
	}
	if (!xp_number(xp_get(root, "version"), &version) || version != 1)
		goto done;
	array = xp_get(root, "emitters");
	if (!array || xrtValueType(array) != XVALUE_ARRAY || !(count = xrtValueCount(array)) ||
	    count > XGE_PARTICLE_MAX_EMITTERS)
		goto done;
	emitters = (xge_particle_emitter_t *)xrtCalloc(count, sizeof(*emitters));
	if (!emitters)
	{
		result = XGE_ERROR_OUT_OF_MEMORY;
		goto done;
	}
	for (i = 0; i < count; i++)
		if (!xp_read_emitter(xrtValueArrayGet(array, i), &emitters[i], error, capacity, (int)i))
			goto done;
	result = xgeParticleDefinitionCreate(out, emitters, (int)count);
done:
	xrtFree(emitters);
	xrtValueRelease(root);
invalid:
	if (result != XGE_OK && error && capacity && !error[0])
		snprintf(error, capacity,
		         "Invalid particle definition: version, range, curve keys or subemitter cycle");
	return result;
}
static xvalue *xp_color_write(uint32_t color)
{
	char text[10];
	snprintf(text, sizeof(text), "#%02X%02X%02X%02X", XGE_COLOR_GET_R(color), XGE_COLOR_GET_G(color),
	         XGE_COLOR_GET_B(color), XGE_COLOR_GET_A(color));
	return xrtValueString(xp_text(text));
}
static xvalue *xp_write_floats(const void *source, int count)
{
	xvalue *array = xrtValueArray();
	int i;
	if (!array)
		return NULL;
	for (i = 0; i < count; i++)
	{
		float value;
		memcpy(&value, (const unsigned char *)source + i * sizeof(float), sizeof(float));
		if (!xrtValueArrayAppendNew(array, xrtValueFloat(value)))
		{
			xrtValueRelease(array);
			return NULL;
		}
	}
	return array;
}
static xvalue *xp_write_field(const xge_particle_emitter_t *e, const xp_io_field *field)
{
	const void *p = (const unsigned char *)e + field->offset;
	switch (field->type)
	{
	case XP_IO_STRING:
		return xrtValueString(xp_text((const char *)p));
	case XP_IO_BOOL:
		return xrtValueBool(*(const int *)p != 0);
	case XP_IO_COLOR:
		return xp_color_write(*(const uint32_t *)p);
	case XP_IO_FLOAT:
		return xrtValueFloat(*(const float *)p);
	case XP_IO_INT:
		return xrtValueInt(*(const int *)p);
	case XP_IO_UINT:
		return xrtValueUInt(*(const uint32_t *)p);
	default:
		return xp_write_floats(p, field->type == XP_IO_RECT ? 4 : 2);
	}
}
static int xp_put(xvalue *object, const char *key, xvalue *value)
{
	if (!value)
		return 0;
	return xrtValueObjectSetNew(object, xp_text(key), value);
}
static xvalue *xp_write_curve(const xge_particle_curve_t *curve)
{
	xvalue *object = xrtValueObject(), *keys = xrtValueArray();
	int i;
	if (!object || !keys)
		goto fail;
	for (i = 0; i < curve->iCount; i++)
		if (!xrtValueArrayAppendNew(keys, xp_write_floats(&curve->arrKeys[i], 4)))
			goto fail;
	if (!xp_put(object, "interpolation", xrtValueInt(curve->iInterpolation)))
		goto fail;
	if (!xrtValueObjectSetTake(object, XRT_STR_LITERAL("keys"), &keys))
		goto fail;
	return object;
fail:
	xrtValueRelease(keys);
	xrtValueRelease(object);
	return NULL;
}
static xvalue *xp_write_gradient(const xge_particle_gradient_t *gradient)
{
	xvalue *array = xrtValueArray();
	int i;
	if (!array)
		return NULL;
	for (i = 0; i < gradient->iCount; i++)
	{
		xvalue *key = xrtValueArray();
		if (!key)
			goto fail;
		if (!xrtValueArrayAppendNew(key, xrtValueFloat(gradient->arrKeys[i].fTime)) ||
		    !xrtValueArrayAppendNew(key, xp_color_write(gradient->arrKeys[i].iColor)))
		{
			xrtValueRelease(key);
			goto fail;
		}
		if (!xrtValueArrayAppendNew(array, key))
			goto fail;
	}
	return array;
fail:
	xrtValueRelease(array);
	return NULL;
}
static xvalue *xp_write_bursts(const xge_particle_emitter_t *e)
{
	xvalue *array = xrtValueArray();
	int i;
	if (!array)
		return NULL;
	for (i = 0; i < e->iBurstCount; i++)
	{
		xvalue *item = xrtValueArray();
		if (!item)
			goto fail;
		if (!xrtValueArrayAppendNew(item, xrtValueFloat(e->arrBursts[i].fTime)) ||
		    !xrtValueArrayAppendNew(item, xrtValueUInt(e->arrBursts[i].iCount)))
		{
			xrtValueRelease(item);
			goto fail;
		}
		if (!xrtValueArrayAppendNew(array, item))
			goto fail;
	}
	return array;
fail:
	xrtValueRelease(array);
	return NULL;
}
static xvalue *xp_write_links(const xge_particle_emitter_t *e)
{
	static const char *names[] = {"birth", "death", "collision"};
	xvalue *object = xrtValueObject();
	int i;
	if (!object)
		return NULL;
	for (i = 0; i < 3; i++)
	{
		xvalue *link = xrtValueObject();
		if (!link)
			goto fail;
		if (!xp_put(link, "emitter", xrtValueInt(e->arrSubEmitters[i].iEmitter)) ||
		    !xp_put(link, "count", xrtValueUInt(e->arrSubEmitters[i].iCount)) ||
		    !xp_put(link, "inheritVelocity", xrtValueFloat(e->arrSubEmitters[i].fInheritVelocity)))
		{
			xrtValueRelease(link);
			goto fail;
		}
		if (!xp_put(object, names[i], link))
			goto fail;
	}
	return object;
fail:
	xrtValueRelease(object);
	return NULL;
}
static xvalue *xp_write_emitter(const xge_particle_emitter_t *e)
{
	xvalue *object = xrtValueObject();
	size_t i;
	if (!object)
		return NULL;
	for (i = 0; i < sizeof(xp_fields) / sizeof(xp_fields[0]); i++)
		if (!xp_put(object, xp_fields[i].name, xp_write_field(e, &xp_fields[i])))
			goto fail;
	if (!xp_put(object, "sizeOverLife", xp_write_curve(&e->tSizeOverLife)) ||
	    !xp_put(object, "speedOverLife", xp_write_curve(&e->tSpeedOverLife)) ||
	    !xp_put(object, "alphaOverLife", xp_write_curve(&e->tAlphaOverLife)) ||
	    !xp_put(object, "colorOverLife", xp_write_gradient(&e->tColorOverLife)) ||
	    !xp_put(object, "bursts", xp_write_bursts(e)) || !xp_put(object, "subEmitters", xp_write_links(e)))
		goto fail;
	return object;
fail:
	xrtValueRelease(object);
	return NULL;
}
int xgeParticleDefinitionStringify(xge_particle_definition definition, int xson, char **out, size_t *size)
{
	xvalue *root = NULL, *emitters = NULL;
	size_t i;
	int result = XGE_ERROR_OUT_OF_MEMORY;
	if (size)
		*size = 0;
	if (!out)
		return XGE_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if (!definition)
		return XGE_ERROR_INVALID_ARGUMENT;
	root = xrtValueObject();
	emitters = xrtValueArray();
	if (!root || !emitters)
		goto done;
	for (i = 0; i < definition->emitters.Count; i++)
		if (!xrtValueArrayAppendNew(
		        emitters, xp_write_emitter(&((xge_particle_emitter_t *)definition->emitters.Data)[i])))
			goto done;
	if (!xp_put(root, "version", xrtValueInt(1)) ||
	    !xrtValueObjectSetTake(root, XRT_STR_LITERAL("emitters"), &emitters))
		goto done;
	*out = xson ? xrtXsonStringify(root, true, size) : xrtJsonStringify(root, true, size);
	if (*out)
		result = XGE_OK;
done:
	xrtValueRelease(emitters);
	xrtValueRelease(root);
	return result;
}
