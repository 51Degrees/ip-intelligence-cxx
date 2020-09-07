/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2020 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
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

#ifndef FIFTYONE_DEGREES_SYNONYM_IPI_INCLUDED
#define FIFTYONE_DEGREES_SYNONYM_IPI_INCLUDED

#include "ipi.h"
#include "common-cxx/fiftyone.h"

// Data types
MAP_TYPE(ProfilePercentage)
MAP_TYPE(ResultIpi)
MAP_TYPE(ResultsIpi)
MAP_TYPE(ResultIpiArray)
MAP_TYPE(ConfigIpi)
MAP_TYPE(DataSetIpi)
MAP_TYPE(DataSetIpiHeader)
MAP_TYPE(ProfileCombination)
MAP_TYPE(Ipv4Range)
MAP_TYPE(Ipv6Range)

// Methods
#define ResultsIpiCreate fiftyoneDegreesResultsIpiCreate
#define ResultsIpiFree fiftyoneDegreesResultsIpiFree
#define ResultsIpiFromIpAddress fiftyoneDegreesResultsIpiFromIpAddress
#define ResultsIpiFromIpAddressString fiftyoneDegreesResultsIpiFromIpAddressString
#define ResultsIpiFromEvidence fiftyoneDegreesResultsIpiFromEvidence
#define ResultsIpiGetValues fiftyoneDegreesResultsIpiGetValues
#define ResultsIpiGetValuesString fiftyoneDegreesResultsIpiGetValuesString
#define ResultsIpiGetValuesStringByRequiredPropertyIndex fiftyoneDegreesResultsIpiGetValuesStringByRequiredPropertyIndex
#define ResultsIpiGetHasValues fiftyoneDegreesResultsIpiGetHasValues
#define ResultsIpiGetNoValueReason fiftyoneDegreesResultsIpiGetNoValueReason
#define ResultsIpiGetNoValueReasonMessage fiftyoneDegreesResultsIpiGetNoValueReasonMessage
#define IpiInitManagerFromFile fiftyoneDegreesIpiInitManagerFromFile
#define IpiInitManagerFromMemory fiftyoneDegreesIpiInitManagerFromMemory
#define DataSetIpiGet fiftyoneDegreesDataSetIpiGet
#define DataSetIpiRelease fiftyoneDegreesDataSetIpiRelease
#define IpiReloadManagerFromOriginalFile fiftyoneDegreesIpiReloadManagerFromOriginalFile
#define IpiReloadManagerFromFile fiftyoneDegreesIpiReloadManagerFromFile
#define IpiReloadManagerFromMemory fiftyoneDegreesIpiReloadManagerFromMemory
#define IpiGetNetworkIdFromResult fiftyoneDegreesIpiGetNetworkIdFromResult
#define IpiGetNetworkIdFromResults fiftyoneDegreesIpiGetNetworkIdFromResults
#define IpiGetCoordinate fiftyoneDegreesIpiGetCoordinate
#define IpiGetIpRangeAsString fiftyoneDegreesIpiGetIpRangeAsString
#define IpiIterateProfilesForPropertyAndValue fiftyoneDegreesIpiIterateProfilesForPropertyAndValue

// Config
#define IpiInMemoryConfig fiftyoneDegreesIpiInMemoryConfig /**< Synonym for #fiftyoneDegreesIpiInMemoryConfig config. */
#define IpiHighPerformanceConfig fiftyoneDegreesIpiHighPerformanceConfig /**< Synonym for #fiftyoneDegreesIpiHighPerformanceConfig config. */
#define IpiLowMemoryConfig fiftyoneDegreesIpiLowMemoryConfig /**< Synonym for #fiftyoneDegreesIpiLowMemoryConfig config. */
#define IpiBalancedConfig fiftyoneDegreesIpiBalancedConfig /**< Synonym for #fiftyoneDegreesIpiBalancedConfig config. */
#define IpiBalancedTempConfig fiftyoneDegreesIpiBalancedTempConfig /**< Synonym for #fiftyoneDegreesIpiBalancedTempConfig config. */
#define IpiDefaultConfig fiftyoneDegreesIpiDefaultConfig /**< Synonym for #fiftyoneDegreesIpiDefaultConfig config. */

#endif