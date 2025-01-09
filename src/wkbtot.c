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

typedef int32_t (*fiftyoneDegreesWKBToT_IntReader)(const unsigned char *wkbBytes);
typedef double (*fiftyoneDegreesWKBToT_DoubleReader)(const unsigned char *wkbBytes);
typedef struct  {
    const char *name;
    fiftyoneDegreesWKBToT_IntReader readInt;
    fiftyoneDegreesWKBToT_DoubleReader readDouble;
} fiftyoneDegreesWKBToT_NumReader;

static int32_t fiftyoneDegreesWKBToT_ReadIntMatchingBitness(const unsigned char *wkbBytes) {
    return *(int32_t *)wkbBytes;
}
static double fiftyoneDegreesWKBToT_ReadDoubleMatchingBitness(const unsigned char *wkbBytes) {
    return *(double *)wkbBytes;
}

static int32_t fiftyoneDegreesWKBToT_ReadIntMismatchingBitness(const unsigned char *wkbBytes) {
    unsigned char t[4];
    for (short i = 0; i < 4; i++) {
        t[i] = wkbBytes[3 - i];
    }
    return *(int32_t *)t;
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
    *(int32_t *)buffer = 1;
    return buffer[0];
}


typedef struct {
    const unsigned char *binaryBuffer;
    fiftyoneDegreesStringBuilder * const stringBuilder;
    fiftyoneDegreesWKBToT_CoordMode const coordMode;
    fiftyoneDegreesWKBToT_ByteOrder const wkbByteOrder;
    fiftyoneDegreesWKBToT_ByteOrder const machineByteOrder;
    fiftyoneDegreesWKBToT_NumReader const numReader;
} fiftyoneDegreesWKBToT_ProcessingContext;


static int32_t fiftyoneDegreesWKBToT_ReadInt(
    fiftyoneDegreesWKBToT_ProcessingContext * const context,
    bool movePointer) {

    const int32_t result = context->numReader.readInt(context->binaryBuffer);
    if (movePointer) {
        context->binaryBuffer += 4;
    }
    return result;
}

static double fiftyoneDegreesWKBToT_ReadDouble(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    const double result = context->numReader.readDouble(context->binaryBuffer);
    context->binaryBuffer += 8;
    return result;
}


static void fiftyoneDegreesWKBToT_WriteCharacter(
    const fiftyoneDegreesWKBToT_ProcessingContext * const context, const char character) {
    fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, character);
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
    // fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ' ');
}



typedef void (*fiftyoneDegreesWKBToT_LoopVisitor)(
    fiftyoneDegreesWKBToT_ProcessingContext * const context);

static void fiftyoneDegreesWKBToT_WithParenthesesIterate(
    fiftyoneDegreesWKBToT_ProcessingContext * const context,
    const fiftyoneDegreesWKBToT_LoopVisitor visitor,
    const int32_t count) {

    fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, '(');
    for (int32_t i = 0; i < count; i++) {
        if (i) {
            fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ',');
        }
        visitor(context);
    }
    fiftyoneDegreesStringBuilderAddChar(context->stringBuilder, ')');
}

static void fiftyoneDegreesWKBToT_HandlePointSegment(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    for (short i = 0; i < context->coordMode.dimensionsCount; i++) {
        if (i) {
            fiftyoneDegreesWKBToT_WriteCharacter(context, ' ');
        }
        const double nextCoord = fiftyoneDegreesWKBToT_ReadDouble(context);
        fiftyoneDegreesWKBToT_WriteDouble(context, nextCoord);
    }
}

static void fiftyoneDegreesWKBToT_HandleLoop(
    fiftyoneDegreesWKBToT_ProcessingContext * const context,
    const fiftyoneDegreesWKBToT_LoopVisitor visitor) {

    const int32_t count = fiftyoneDegreesWKBToT_ReadInt(context, true);
    if (!count) {
        static const char empty[] = "EMPTY";
        fiftyoneDegreesStringBuilderAddChars(context->stringBuilder, empty, sizeof(empty));
        return;
    }
    fiftyoneDegreesWKBToT_WithParenthesesIterate(context, visitor, count);
}

static void fiftyoneDegreesWKBToT_SkipGeometryHeader(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    context->binaryBuffer += 1; // skip byte order byte
    context->binaryBuffer += 4; // skip geometry type
}

static void fiftyoneDegreesWKBToT_HandlePoint(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    fiftyoneDegreesWKBToT_SkipGeometryHeader(context);
    fiftyoneDegreesWKBToT_WriteTaggedGeometryName(context, "POINT");

    fiftyoneDegreesWKBToT_WithParenthesesIterate(
        context, fiftyoneDegreesWKBToT_HandlePointSegment, 1);
}

static void fiftyoneDegreesWKBToT_HandleGeometry(
    fiftyoneDegreesWKBToT_ProcessingContext * const context) {

    fiftyoneDegreesWKBToT_HandlePoint(context);
}

static void fiftyoneDegreesWKBToT_HandleWKBRoot(
    const unsigned char *binaryBuffer,
    fiftyoneDegreesStringBuilder * const stringBuilder) {

    const fiftyoneDegreesWKBToT_ByteOrder machineByteOrder = fiftyoneDegreesWKBToT_GetBitness();
    const fiftyoneDegreesWKBToT_ByteOrder wkbByteOrder = *binaryBuffer;
    const fiftyoneDegreesWKBToT_NumReader numReader = (
        (wkbByteOrder == machineByteOrder)
        ? fiftyoneDegreesWKBToT_MatchingBitnessNumReader
        : fiftyoneDegreesWKBToT_MismatchingBitnessNumReader);

    const int32_t rootGeometryTypeFull = numReader.readInt(binaryBuffer + 1);
    const int32_t coordType = rootGeometryTypeFull / 100;

    fiftyoneDegreesWKBToT_ProcessingContext context = {
        binaryBuffer,
        stringBuilder,

        fiftyoneDegreesWKBToT_CoordModes[coordType],
        wkbByteOrder,
        machineByteOrder,
        numReader,
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
