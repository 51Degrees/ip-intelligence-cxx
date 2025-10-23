/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

#ifndef FIFTYONE_DEGREES_CONSTANTSIPI_H
#define FIFTYONE_DEGREES_CONSTANTSIPI_H

#include <stdint.h>

/**
 * @def FIFTYONE_DEGREES_WKT_DECIMAL_PLACES
 * Defines the number of decimal places to use when formatting WKT
 * (Well-Known Text) coordinate values. This can be overridden at compile time
 * by defining this macro before including this header or via compiler flags.
 *
 * Example: -DFIFTYONE_DEGREES_WKT_DECIMAL_PLACES=2
 *
 * Default: 2 decimal places
 * Range: 0-15 (limited by FIFTYONE_DEGREES_MAX_DOUBLE_DECIMAL_PLACES)
 */
#ifndef FIFTYONE_DEGREES_WKT_DECIMAL_PLACES
#define FIFTYONE_DEGREES_WKT_DECIMAL_PLACES 2
#endif

static const uint8_t fiftyoneDegreesDefaultWktDecimalPlaces = FIFTYONE_DEGREES_WKT_DECIMAL_PLACES;

#endif //FIFTYONE_DEGREES_CONSTANTSIPI_H
