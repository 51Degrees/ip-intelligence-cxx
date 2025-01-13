/* *********************************************************************
* This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL)
 * v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include "wkbtot.h"
#include "common-cxx/string.h"

typedef struct {
    short dimensionsCount;
    const char *tag;
    size_t tagLength;
} fiftyoneDegreesWKBToT_CoordMode;


static const fiftyoneDegreesWKBToT_CoordMode fiftyoneDegreesWKBToT_CoordModes[] = {
    { 2, NULL, 0 },
    { 3, "Z", 1 },
    { 3, "M", 1 },
    { 4, "ZM", 2 },
};


typedef enum {
    fiftyoneDegreesWKBToT_XDR = 0, // Big Endian
    fiftyoneDegreesWKBToT_NDR = 1, // Little Endian
} fiftyoneDegreesWKBToT_ByteOrder;

typedef uint32_t (*fiftyoneDegreesWKBToT_IntReader)(const unsigned char *wkbBytes);
typedef double (*fiftyoneDegreesWKBToT_DoubleReader)(const unsigned char *wkbBytes);
typedef struct  {
    const char *name;
    fiftyoneDegreesWKBToT_IntReader readInt;
    fiftyoneDegreesWKBToT_DoubleReader readDouble;
} fiftyoneDegreesWKBToT_NumReader;

static uint32_t fiftyoneDegreesWKBToT_ReadIntMatchingBitness(const unsigned char *wkbBytes) {
    return *(uint32_t *)wkbBytes;
}
static double fiftyoneDegreesWKBToT_ReadDoubleMatchingBitness(const unsigned char *wkbBytes) {
    return *(double *)wkbBytes;
}

static uint32_t fiftyoneDegreesWKBToT_ReadIntMismatchingBitness(const unsigned char *wkbBytes) {
    unsigned char t[4];
    for (short i = 0; i < 4; i++) {
        t[i] = wkbBytes[3 - i];
    }
    return *(uint32_t *)t;
}
static double fiftyoneDegreesWKBToT_ReadDoubleMismatchingBitness(const unsigned char *wkbBytes) {
    unsigned char t[8];
    for (short i = 0; i < 8; i++) {
        t[i] = wkbBytes[7 - i];
    }
    return *(double *)t;
}

static const fiftyoneDegreesWKBToT_NumReader fiftyoneDegreesWKBToT_MatchingBitnessNumReader = {
    "Matching Bitness NumReader",
    fiftyoneDegreesWKBToT_ReadIntMatchingBitness,
    fiftyoneDegreesWKBToT_ReadDoubleMatchingBitness,
};

static const fiftyoneDegreesWKBToT_NumReader fiftyoneDegreesWKBToT_MismatchingBitnessNumReader = {
    "Mismatching Bitness NumReader",
    fiftyoneDegreesWKBToT_ReadIntMismatchingBitness,
    fiftyoneDegreesWKBToT_ReadDoubleMismatchingBitness,
};

static fiftyoneDegreesWKBToT_ByteOrder fiftyoneDegreesWKBToT_GetBitness() {
    unsigned char buffer[4];
    *(uint32_t *)buffer = 1;
    return buffer[0];
}


typedef struct {
    const unsigned char *binaryBuffer;
    fiftyoneDegreesStringBuilder * const stringBuilder;
    fiftyoneDegreesWKBToT_CoordMode coordMode;
    fiftyoneDegreesWKBToT_ByteOrder wkbByteOrder;
    fiftyoneDegreesWKBToT_ByteOrder const machineByteOrder;
    const fiftyoneDegreesWKBToT_NumReader *numReader;
} fiftyoneDegreesWKBToT_ProcessingContext;


static uint32_t fiftyoneDegreesWKBToT_ReadInt(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    const uint32_t result = context->numReader->readInt(context->binaryBuffer);
    context->binaryBuffer += 4;
    return result;
}

static double fiftyoneDegreesWKBToT_ReadDouble(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    const double result = context->numReader->readDouble(context->binaryBuffer);
    context->binaryBuffer += 8;
    return result;
}


static void fiftyoneDegreesWKBToT_WriteDouble(
    const fiftyoneDegreesWKBToT_ProcessingContext * const context, const double value) {
    // 64-bit max-length double is `-X.{X:16}e-308` => 24 characters + NULL
    char temp[27];
    if (snprintf(temp, sizeof(temp), "%.17g", value) > 0) {
        fiftyoneDegreesStringBuilderAddChars(
            context->stringBuilder,
            temp,
            strlen(temp));
    }
}

static void fiftyoneDegreesWKBToT_WriteEmpty(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    static const char empty[] = "EMPTY";
    fiftyoneDegreesStringBuilderAddChars(context->stringBuilder, empty, sizeof(empty));
}

static void fiftyoneDegreesWKBToT_WriteTaggedGeometryName(
    const fiftyoneDegreesWKBToT_ProcessingContext * const context,
    const char * const geometryName) {

    fiftyoneDegreesStringBuilderAddChars(
        context->stringBuilder,
        geometryName,
        strlen(geometryName));
    if (context->coordMode.tag) {
        fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ' ');
        fiftyoneDegreesStringBuilderAddChars(
            context->stringBuilder,
            context->coordMode.tag,
            context->coordMode.tagLength);
    }
    fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ' ');
}



typedef void (*fiftyoneDegreesWKBToT_LoopVisitor)(
    fiftyoneDegreesWKBToT_ProcessingContext * const context);

static void fiftyoneDegreesWKBToT_WithParenthesesIterate(
    fiftyoneDegreesWKBToT_ProcessingContext * const context,
    const fiftyoneDegreesWKBToT_LoopVisitor visitor,
    const uint32_t count) {

    fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, '(');
    for (uint32_t i = 0; i < count; i++) {
        if (i) {
            fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ',');
            fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ' ');
        }
        visitor(context);
    }
    fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ')');
}

static void fiftyoneDegreesWKBToT_HandlePointSegment(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    for (short i = 0; i < context->coordMode.dimensionsCount; i++) {
        if (i) {
            fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ' ');
        }
        const double nextCoord = fiftyoneDegreesWKBToT_ReadDouble(context);
        fiftyoneDegreesWKBToT_WriteDouble(context, nextCoord);
    }
}

static void fiftyoneDegreesWKBToT_HandleLoop(
    fiftyoneDegreesWKBToT_ProcessingContext * const context,
    const fiftyoneDegreesWKBToT_LoopVisitor visitor) {

    const uint32_t count = fiftyoneDegreesWKBToT_ReadInt(context);
    if (count) {
        fiftyoneDegreesWKBToT_WithParenthesesIterate(context, visitor, count);
    } else {
        fiftyoneDegreesWKBToT_WriteEmpty(context);
    }
}

static void fiftyoneDegreesWKBToT_HandleLinearRing(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    fiftyoneDegreesWKBToT_HandleLoop(
        context, fiftyoneDegreesWKBToT_HandlePointSegment);
}


typedef struct fiftyoneDegreesWKBToT_GeometryParser_t {
    const char * const nameToPrint;
    const bool hasChildCount;
    const struct fiftyoneDegreesWKBToT_GeometryParser_t * const childGeometry;
    const fiftyoneDegreesWKBToT_LoopVisitor childParser;
} fiftyoneDegreesWKBToT_GeometryParser;

static void fiftyoneDegreesWKBToT_HandleGeometry(
    fiftyoneDegreesWKBToT_ProcessingContext *context);



static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_Geometry = {
    // ABSTRACT -- ANY GEOMETRY BELOW QUALIFIES
    "Geometry",
    false,
    NULL,
    fiftyoneDegreesWKBToT_WriteEmpty,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_Point = {
    "Point",
    false,
    NULL,
    fiftyoneDegreesWKBToT_HandlePointSegment,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_LineString = {
    "LineString",
    true,
    NULL,
    fiftyoneDegreesWKBToT_HandlePointSegment,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_Polygon = {
    "Polygon",
    true,
    NULL,
    fiftyoneDegreesWKBToT_HandleLinearRing,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_MultiPoint = {
    "MultiPoint",
    true,
    &fiftyoneDegreesWKBToT_Geometry_Point,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_MultiLineString = {
    "MultiLineString",
    true,
    &fiftyoneDegreesWKBToT_Geometry_LineString,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_MultiPolygon = {
    "MultiPolygon",
    true,
    &fiftyoneDegreesWKBToT_Geometry_Polygon,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_GeometryCollection = {
    "GeometryCollection",
    true,
    NULL,
    fiftyoneDegreesWKBToT_HandleGeometry,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_CircularString = {
    // RESERVED IN STANDARD (OGC 06-103r4) FOR FUTURE USE
    "CircularString",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_CompoundCurve = {
    // RESERVED IN STANDARD (OGC 06-103r4) FOR FUTURE USE
    "CompoundCurve",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_CurvePolygon = {
    // RESERVED IN STANDARD (OGC 06-103r4) FOR FUTURE USE
    "CurvePolygon",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_MultiCurve = {
    // NON-INSTANTIABLE -- SEE `MultiLineString` SUBCLASS
    "MultiCurve",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_MultiSurface = {
    // NON-INSTANTIABLE -- SEE `MultiPolygon` SUBCLASS
    "MultiSurface",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_Curve = {
    // NON-INSTANTIABLE -- SEE `LineString` SUBCLASS. ALSO `LinearRing` and `Line`
    "Curve",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_Surface = {
    // NON-INSTANTIABLE -- SEE `Polygon` AND `PolyhedralSurface` SUBCLASSES.
    "Surface",
    false,
    NULL,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_PolyhedralSurface = {
    "PolyhedralSurface",
    true,
    &fiftyoneDegreesWKBToT_Geometry_Polygon,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_TIN = {
    "TIN",
    true,
    &fiftyoneDegreesWKBToT_Geometry_Polygon,
    NULL,
};
static const fiftyoneDegreesWKBToT_GeometryParser fiftyoneDegreesWKBToT_Geometry_Triangle = {
    "Triangle",
    true,
    NULL,
    fiftyoneDegreesWKBToT_HandleLinearRing,
};

static const fiftyoneDegreesWKBToT_GeometryParser * const fiftyoneDegreesWKBToT_Geometries[] = {
    &fiftyoneDegreesWKBToT_Geometry_Geometry,
    &fiftyoneDegreesWKBToT_Geometry_Point,
    &fiftyoneDegreesWKBToT_Geometry_LineString,
    &fiftyoneDegreesWKBToT_Geometry_Polygon,
    &fiftyoneDegreesWKBToT_Geometry_MultiPoint,
    &fiftyoneDegreesWKBToT_Geometry_MultiLineString,
    &fiftyoneDegreesWKBToT_Geometry_MultiPolygon,
    &fiftyoneDegreesWKBToT_Geometry_GeometryCollection,
    &fiftyoneDegreesWKBToT_Geometry_CircularString,
    &fiftyoneDegreesWKBToT_Geometry_CompoundCurve,
    &fiftyoneDegreesWKBToT_Geometry_CurvePolygon,
    &fiftyoneDegreesWKBToT_Geometry_MultiCurve,
    &fiftyoneDegreesWKBToT_Geometry_MultiSurface,
    &fiftyoneDegreesWKBToT_Geometry_Curve,
    &fiftyoneDegreesWKBToT_Geometry_Surface,
    &fiftyoneDegreesWKBToT_Geometry_PolyhedralSurface,
    &fiftyoneDegreesWKBToT_Geometry_TIN,
    &fiftyoneDegreesWKBToT_Geometry_Triangle,
};


static void fiftyoneDegreesWKBToT_UpdateWkbByteOrder(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    const fiftyoneDegreesWKBToT_ByteOrder newByteOrder = *context->binaryBuffer;
    context->binaryBuffer++;

    if (newByteOrder == context->wkbByteOrder) {
        return;
    }
    context->wkbByteOrder = newByteOrder;
    context->numReader = (
        (context->wkbByteOrder == context->machineByteOrder)
        ? &fiftyoneDegreesWKBToT_MatchingBitnessNumReader
        : &fiftyoneDegreesWKBToT_MismatchingBitnessNumReader);
}

static void fiftyoneDegreesWKBToT_HandleGeometry(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    fiftyoneDegreesWKBToT_UpdateWkbByteOrder(context);

    const uint32_t geometryTypeFull = fiftyoneDegreesWKBToT_ReadInt(context);
    const uint32_t coordType = geometryTypeFull / 1000;
    const uint32_t geometryCode = geometryTypeFull % 1000;

    context->coordMode = fiftyoneDegreesWKBToT_CoordModes[coordType];

    const fiftyoneDegreesWKBToT_GeometryParser * const parser =
        fiftyoneDegreesWKBToT_Geometries[geometryCode];
    if (parser->nameToPrint) {
        fiftyoneDegreesWKBToT_WriteTaggedGeometryName(context, parser->nameToPrint);
    }

    const fiftyoneDegreesWKBToT_LoopVisitor visitor = (parser->childGeometry
        ? fiftyoneDegreesWKBToT_HandleGeometry
        : (parser->childParser
            ? parser->childParser
            : fiftyoneDegreesWKBToT_HandleGeometry));

    if (parser->hasChildCount) {
        fiftyoneDegreesWKBToT_HandleLoop(context, visitor);
    } else {
        fiftyoneDegreesWKBToT_WithParenthesesIterate(context, visitor, 1);
    }
}

static void fiftyoneDegreesWKBToT_HandleWKBRoot(
    const unsigned char *binaryBuffer,
    fiftyoneDegreesStringBuilder * const stringBuilder) {

    fiftyoneDegreesWKBToT_ProcessingContext context = {
        binaryBuffer,
        stringBuilder,

        fiftyoneDegreesWKBToT_CoordModes[0],
        ~*binaryBuffer,
        fiftyoneDegreesWKBToT_GetBitness(),
        NULL,
    };

    fiftyoneDegreesWKBToT_HandleGeometry(&context);
}


fiftyoneDegreesWkbtotResult
fiftyoneDegreesConvertWkbToWkt(
    const unsigned char * const wellKnownBinary,
    char * const buffer, size_t const length,
    fiftyoneDegreesException * const exception) {

    fiftyoneDegreesStringBuilder stringBuilder = { buffer, length };
    fiftyoneDegreesStringBuilderInit(&stringBuilder);

    fiftyoneDegreesWKBToT_HandleWKBRoot(wellKnownBinary, &stringBuilder);

    fiftyoneDegreesStringBuilderComplete(&stringBuilder);

    const fiftyoneDegreesWkbtotResult result = {
        stringBuilder.added,
        stringBuilder.full,
    };
    return result;
}
