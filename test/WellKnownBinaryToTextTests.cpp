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

#include "pch.h"
#include "../src/wkbtot.h"

static bool CheckResult(const char *result, const char *expected, uint16_t const size) {
	bool match = true;
	for (uint16_t i = 0; i < size; i++) {
		match = match && (*result == *expected);
		result++;
		expected++;
	}
	return match;
}

static size_t constexpr DEFAULT_BUFFER_SIZE = 1024;

TEST(WKBToT, WKBToT_Point_2D_NDR)
{
	const uint8_t wkbBytes[] = {
		0x00, // big endian
      	0x00,0x00,0x00,0x01, // POINT (2D)
        0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 2.0: x-coordinate
      	0x40,0x10,0x00,0x00,0x00,0x00,0x00,0x00, // 4.0: y-coordinate
    };
	const char * const expected = "POINT(2 4)";

	char buffer[DEFAULT_BUFFER_SIZE] = { 0 };
	FIFTYONE_DEGREES_EXCEPTION_CREATE;

	auto const result = fiftyoneDegreesConvertWkbToWkt(
		wkbBytes,
		buffer, std::size(buffer),
		exception);

	EXPECT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
		"Got exception while converting WKB: " << fiftyoneDegreesExceptionGetMessage(exception);

	EXPECT_FALSE(result.bufferTooSmall) <<
		"Buffer was deemed too small, requested " << result.written <<
		", available " <<  std::size(buffer);

	EXPECT_TRUE(
		CheckResult(buffer, expected, strlen(expected))) <<
		"The value of Point 2D (NDR) is not correctly converted -- '" << buffer <<
		"' -- vs expected -- '" << expected << "'";
}