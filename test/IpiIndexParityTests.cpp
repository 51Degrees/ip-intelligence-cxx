/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
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

/**
 * @file IpiIndexParityTests.cpp
 * @brief Tests that the property value index (propertyValueIndex
 * configuration option, issue #96) returns exactly the same values as the
 * non indexed value search.
 *
 * Two managers are created from the same data file with configurations that
 * differ only in the propertyValueIndex option. The same IP addresses are
 * then processed by both and every available property is compared for:
 * - the has values state,
 * - the number of weighted values,
 * - each value's weighting,
 * - the values string representation.
 */

#include "pch.h"
#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>

extern "C" {
#include "../src/ipi.h"
#include "../src/fiftyone.h"
}

#include "Constants.hpp"

// Buffer size for the values string of a single property.
static const size_t VALUES_BUFFER_SIZE = 8192;

class IpiIndexParityTests : public ::testing::Test {
protected:
	fiftyoneDegreesResourceManager managerWithIndex = { nullptr };
	fiftyoneDegreesResourceManager managerNoIndex = { nullptr };
	bool managersInitialised = false;
	char dataFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH] = "";

	void SetUp() override {
		// Prefer the small ASN data file included with the
		// ip-intelligence-data submodule: parity does not depend on the data
		// file contents, and building the index for a full size file with
		// the file backed Balanced collections takes many minutes. Fall back
		// to the data files used by the engine tests.
		const char* fileNames[] = {
			"51Degrees-IPIV4AsnIpiV41.ipi",
			_IpiFileNames[0],
			_IpiFileNames[1],
		};
		fiftyoneDegreesStatusCode status =
			FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND;
		for (size_t i = 0; i < sizeof(fileNames) / sizeof(fileNames[0]); i++) {
			status = fiftyoneDegreesFileGetPath(
				_dataFolderName,
				fileNames[i],
				dataFilePath,
				sizeof(dataFilePath));
			if (status == FIFTYONE_DEGREES_STATUS_SUCCESS) {
				break;
			}
		}
		if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
			GTEST_SKIP() << "No IP Intelligence data file was found.";
		}

		// Create one manager with the property value index and one without.
		// Everything else about the configurations is identical.
		initManager(&managerWithIndex, true);
		if (HasFatalFailure()) return;
		initManager(&managerNoIndex, false);
		if (HasFatalFailure()) return;
		managersInitialised = true;

		// Check the index was created for the first manager and not for the
		// second.
		fiftyoneDegreesDataSetIpi* dataSet =
			fiftyoneDegreesDataSetIpiGet(&managerWithIndex);
		ASSERT_NE(nullptr, dataSet);
		EXPECT_NE(nullptr, dataSet->b.b.indexPropertyProfile) <<
			"The property value index should have been created when "
			"propertyValueIndex is enabled.";
		fiftyoneDegreesDataSetIpiRelease(dataSet);
		dataSet = fiftyoneDegreesDataSetIpiGet(&managerNoIndex);
		ASSERT_NE(nullptr, dataSet);
		EXPECT_EQ(nullptr, dataSet->b.b.indexPropertyProfile) <<
			"The property value index should not have been created when "
			"propertyValueIndex is disabled.";
		fiftyoneDegreesDataSetIpiRelease(dataSet);
	}

	void TearDown() override {
		if (managersInitialised) {
			fiftyoneDegreesResourceManagerFree(&managerWithIndex);
			fiftyoneDegreesResourceManagerFree(&managerNoIndex);
			managersInitialised = false;
		}
	}

	void initManager(
		fiftyoneDegreesResourceManager* manager,
		bool propertyValueIndex) {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		fiftyoneDegreesConfigIpi config = fiftyoneDegreesIpiBalancedConfig;
		config.b.propertyValueIndex = propertyValueIndex;
		fiftyoneDegreesPropertiesRequired properties =
			fiftyoneDegreesPropertiesDefault;
		fiftyoneDegreesStatusCode status = fiftyoneDegreesIpiInitManagerFromFile(
			manager,
			&config,
			&properties,
			dataFilePath,
			exception);
		ASSERT_EQ(FIFTYONE_DEGREES_STATUS_SUCCESS, status) <<
			fiftyoneDegreesStatusGetMessage(status, dataFilePath);
		ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
			fiftyoneDegreesExceptionGetMessage(exception);
	}

	// Processes the IP address with both managers and compares the results
	// of every available property.
	void compareIpAddress(const char* ipAddress) {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		fiftyoneDegreesResultsIpi* resultsWithIndex =
			fiftyoneDegreesResultsIpiCreate(&managerWithIndex);
		ASSERT_NE(nullptr, resultsWithIndex);
		fiftyoneDegreesResultsIpi* resultsNoIndex =
			fiftyoneDegreesResultsIpiCreate(&managerNoIndex);
		ASSERT_NE(nullptr, resultsNoIndex);

		fiftyoneDegreesResultsIpiFromIpAddressString(
			resultsWithIndex,
			ipAddress,
			strlen(ipAddress),
			exception);
		ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
			fiftyoneDegreesExceptionGetMessage(exception);
		fiftyoneDegreesResultsIpiFromIpAddressString(
			resultsNoIndex,
			ipAddress,
			strlen(ipAddress),
			exception);
		ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
			fiftyoneDegreesExceptionGetMessage(exception);

		const fiftyoneDegreesDataSetIpi* dataSet =
			(fiftyoneDegreesDataSetIpi*)resultsWithIndex->b.dataSet;
		const uint32_t availableCount = dataSet->b.b.available->count;

		for (uint32_t i = 0; i < availableCount; i++) {
			SCOPED_TRACE(
				std::string("IP address '") + ipAddress +
				"' required property index " + std::to_string(i));

			// The has values state must be the same.
			const bool hasValuesWithIndex = fiftyoneDegreesResultsIpiGetHasValues(
				resultsWithIndex,
				(int)i,
				exception);
			ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
				fiftyoneDegreesExceptionGetMessage(exception);
			const bool hasValuesNoIndex = fiftyoneDegreesResultsIpiGetHasValues(
				resultsNoIndex,
				(int)i,
				exception);
			ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
				fiftyoneDegreesExceptionGetMessage(exception);
			EXPECT_EQ(hasValuesNoIndex, hasValuesWithIndex);

			// The weighted values must be the same, in the same order.
			const fiftyoneDegreesWeightedItem* firstWithIndex =
				fiftyoneDegreesResultsIpiGetValues(
					resultsWithIndex,
					(int)i,
					exception);
			ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
				fiftyoneDegreesExceptionGetMessage(exception);
			const uint32_t countWithIndex = resultsWithIndex->values.count;
			const fiftyoneDegreesWeightedItem* firstNoIndex =
				fiftyoneDegreesResultsIpiGetValues(
					resultsNoIndex,
					(int)i,
					exception);
			ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
				fiftyoneDegreesExceptionGetMessage(exception);
			const uint32_t countNoIndex = resultsNoIndex->values.count;

			EXPECT_EQ(firstNoIndex == nullptr, firstWithIndex == nullptr);
			ASSERT_EQ(countNoIndex, countWithIndex);
			for (uint32_t j = 0; j < countNoIndex; j++) {
				EXPECT_EQ(
					resultsNoIndex->values.items[j].rawWeighting,
					resultsWithIndex->values.items[j].rawWeighting) <<
					"Weighting differs for value " << j;
			}

			// The string representation of the values must be the same.
			char bufferWithIndex[VALUES_BUFFER_SIZE];
			char bufferNoIndex[VALUES_BUFFER_SIZE];
			const size_t addedWithIndex =
				fiftyoneDegreesResultsIpiGetValuesStringByRequiredPropertyIndex(
					resultsWithIndex,
					(int)i,
					bufferWithIndex,
					sizeof(bufferWithIndex),
					"|",
					exception);
			ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
				fiftyoneDegreesExceptionGetMessage(exception);
			const size_t addedNoIndex =
				fiftyoneDegreesResultsIpiGetValuesStringByRequiredPropertyIndex(
					resultsNoIndex,
					(int)i,
					bufferNoIndex,
					sizeof(bufferNoIndex),
					"|",
					exception);
			ASSERT_TRUE(FIFTYONE_DEGREES_EXCEPTION_OKAY) <<
				fiftyoneDegreesExceptionGetMessage(exception);
			EXPECT_EQ(addedNoIndex, addedWithIndex);
			EXPECT_STREQ(bufferNoIndex, bufferWithIndex);
		}

		fiftyoneDegreesResultsIpiFree(resultsWithIndex);
		fiftyoneDegreesResultsIpiFree(resultsNoIndex);
	}
};

// Compares a spread of IPv4 addresses across the address space.
TEST_F(IpiIndexParityTests, Ipv4Spread) {
	char ipAddress[64];
	for (int a = 1; a < 256; a += 24) {
		for (int b = 0; b < 256; b += 51) {
			for (int c = 0; c < 256; c += 128) {
				snprintf(
					ipAddress,
					sizeof(ipAddress),
					"%d.%d.%d.1",
					a,
					b,
					c);
				compareIpAddress(ipAddress);
				if (HasFatalFailure()) return;
			}
		}
	}
}

// Compares specific well known and boundary IP addresses.
TEST_F(IpiIndexParityTests, KnownAndBoundaryAddresses) {
	const char* addresses[] = {
		"0.0.0.0",
		"8.8.8.8",
		"1.1.1.1",
		"185.28.167.77",
		"255.255.255.255",
		"::1",
		"2001:4860:4860::8888",
		"2606:4700:4700::1111",
		"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
	};
	for (size_t i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++) {
		compareIpAddress(addresses[i]);
		if (HasFatalFailure()) return;
	}
}
