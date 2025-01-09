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
    const unsigned char *binaryBuffer;
    fiftyoneDegreesStringBuilder *stringBuilder;
} fiftyoneDegreesWKBToT_Position;

typedef struct {
    short dimensionsCount;
    const char *tag;
} fiftyoneDegreesWKBToT_CoordMode;


static const fiftyoneDegreesWKBToT_CoordMode fiftyoneDegreesWKBToT_CoordModes[] = {
    { 2, NULL },
    { 3, "Z" },
    { 3, "M" },
    { 4, "ZM" },
};


typedef enum {
    fiftyoneDegreesWKBToT_XDR = 0, // Big Endian
    fiftyoneDegreesWKBToT_NDR = 1, // Little Endian
} fiftyoneDegreesWKBToT_ByteOrder;

typedef int32_t (*fiftyoneDegreesWKBToT_IntReader)(const unsigned char *wkbBytes);
typedef double (*fiftyoneDegreesWKBToT_DoubleReader)(const unsigned char *wkbBytes);
typedef struct  {
    const char *name;
    fiftyoneDegreesWKBToT_IntReader intReader;
    fiftyoneDegreesWKBToT_DoubleReader doubleReader;
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
    *(uint32_t *)buffer = 1;
    return buffer[0];
}


typedef struct {
    fiftyoneDegreesWKBToT_CoordMode coordMode;
    fiftyoneDegreesWKBToT_ByteOrder wkbByteOrder;
    fiftyoneDegreesWKBToT_ByteOrder machineByteOrder;
    fiftyoneDegreesWKBToT_NumReader numReader;
} fiftyoneDegreesWKBToT_FragmentContext;


static void fiftyoneDegreesWKBToT_WriteCharacter(
    const fiftyoneDegreesWKBToT_Position * const position, const char character) {
    fiftyoneDegreesStringBuilderAddChar(position->stringBuilder, character);
}

static void fiftyoneDegreesWKBToT_WriteDouble(
    const fiftyoneDegreesWKBToT_Position * const position, const double value) {
    // 64-bit max-length double is `-X.{X:16}e-308` => 24 characters + NULL
    char temp[27];
    if (snprintf(temp, sizeof(temp), "%.17g", value) > 0) {
        fiftyoneDegreesStringBuilderAddChars(
            position->stringBuilder,
            temp,
            strlen(temp));
    }
}

static void fiftyoneDegreesWKBToT_WriteTaggedGeometryName(
    const fiftyoneDegreesWKBToT_Position * const position,
    const char * const geometryName,
    const fiftyoneDegreesWKBToT_CoordMode coordMode) {
    fiftyoneDegreesStringBuilderAddChars(
        position->stringBuilder,
        geometryName,
        strlen(geometryName));
    if (coordMode.tag) {
        fiftyoneDegreesStringBuilderAddChar(position->stringBuilder, ' ');
        fiftyoneDegreesStringBuilderAddChars(
            position->stringBuilder,
            coordMode.tag,
            strlen(coordMode.tag));
    }
    fiftyoneDegreesStringBuilderAddChar(position->stringBuilder, ' ');
}



static void fiftyoneDegreesWKBToT_HandlePointSegment(
    fiftyoneDegreesWKBToT_Position * const position,
    const fiftyoneDegreesWKBToT_FragmentContext fragmentContext) {

    fiftyoneDegreesWKBToT_WriteCharacter(position, '(');
    for (short i = 0; i < fragmentContext.coordMode.dimensionsCount; i++) {
        if (i) {
            fiftyoneDegreesWKBToT_WriteCharacter(position, ' ');
        }
        const double nextCoord = fragmentContext.numReader.doubleReader(position->binaryBuffer);
        position->binaryBuffer += 8;
        fiftyoneDegreesWKBToT_WriteDouble(position, nextCoord);
    }
    fiftyoneDegreesWKBToT_WriteCharacter(position, ')');
}

static void fiftyoneDegreesWKBToT_HandlePoint(
    fiftyoneDegreesWKBToT_Position * const position,
    const fiftyoneDegreesWKBToT_FragmentContext fragmentContext) {

    position->binaryBuffer += 1; // skip byte order byte
    position->binaryBuffer += 4; // skip geometry type
    fiftyoneDegreesWKBToT_WriteTaggedGeometryName(position, "POINT", fragmentContext.coordMode);
    fiftyoneDegreesWKBToT_HandlePointSegment(position, fragmentContext);
}

static void fiftyoneDegreesWKBToT_HandleGeometry(
    fiftyoneDegreesWKBToT_Position * const position,
    const fiftyoneDegreesWKBToT_FragmentContext fragmentContext) {

    fiftyoneDegreesWKBToT_HandlePoint(position, fragmentContext);
}

static void fiftyoneDegreesWKBToT_HandleWKBRoot(
    fiftyoneDegreesWKBToT_Position * const position) {

    fiftyoneDegreesWKBToT_FragmentContext fragmentContext;

    fragmentContext.wkbByteOrder = *position->binaryBuffer;
    fragmentContext.machineByteOrder = fiftyoneDegreesWKBToT_GetBitness();
    fragmentContext.numReader = (
        (fragmentContext.wkbByteOrder == fragmentContext.machineByteOrder)
        ? fiftyoneDegreesWKBToT_MatchingBitnessNumReader
        : fiftyoneDegreesWKBToT_MismatchingBitnessNumReader);

    const int rootGeometryTypeFull = fragmentContext.numReader.intReader(position->binaryBuffer + 1);
    const int coordType = rootGeometryTypeFull / 100;
    fragmentContext.coordMode = fiftyoneDegreesWKBToT_CoordModes[coordType];

    fiftyoneDegreesWKBToT_HandleGeometry(position, fragmentContext);
}


fiftyoneDegreesWkbtotResult
fiftyoneDegreesConvertWkbToWkt(
    const unsigned char * const wellKnownBinary,
    char * const buffer, size_t const length,
    fiftyoneDegreesException * const exception) {

    fiftyoneDegreesStringBuilder stringBuilder = { buffer, length };
    fiftyoneDegreesStringBuilderInit(&stringBuilder);

    fiftyoneDegreesWKBToT_Position position = {
        wellKnownBinary,
        &stringBuilder,
    };

    fiftyoneDegreesWKBToT_HandleWKBRoot(&position);

    fiftyoneDegreesStringBuilderComplete(&stringBuilder);

    const fiftyoneDegreesWkbtotResult result = {
        position.stringBuilder->added,
        position.stringBuilder->full,
    };
    return result;
}
