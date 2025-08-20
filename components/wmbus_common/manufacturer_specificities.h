/*
 Copyright (C) 2019 Jacek Tomasiak (gpl-3.0-or-later)
 Copyright (C) 2021 Vincent Privat (gpl-3.0-or-later)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MANUFACTURER_SPECIFICITIES_H
#define MANUFACTURER_SPECIFICITIES_H

#include "util.h"
#include "wmbus.h"

using std::string;
using std::vector;

// Common: add default manufacturers key if none specified and we know one for
// the given frame
void addDefaultManufacturerKeyIfAny(const std::vector<uchar> &frame,
                                    TPLSecurityMode tpl_sec_mode,
                                    MeterKeys *meter_keys);

uint32_t uint32FromBytes(const std::vector<uchar> &data, int offset,
                         bool reverse = false);

// Diehl: initialize support of default keys in a meter
void initializeDiehlDefaultKeySupport(
    const std::vector<uchar> &confidentiality_key, std::vector<uint32_t> &keys);

// Diehl: check method of LFSR decryption algorithm
enum class DiehlLfsrCheckMethod { CHECKSUM_AND_0XEF, HEADER_1_BYTE };

// Diehl: decode LFSR encrypted data used in Izar/PRIOS and Sharky meters
std::vector<uchar> decodeDiehlLfsr(const std::vector<uchar> &origin,
                                   const std::vector<uchar> &frame,
                                   uint32_t key,
                                   DiehlLfsrCheckMethod check_method,
                                   uint32_t check_value);

// Diehl: frame interpretation
enum class DiehlFrameInterpretation {
  NA, // N/A: not a Diehl frame
  REAL_DATA,
  OMS,
  PRIOS,
  SAP_PRIOS,
  SAP_PRIOS_STD,
  PRIOS_SCR,
  RESERVED
};

const char *toString(DiehlFrameInterpretation i);

// Diehl: address transformation method
enum class DiehlAddressTransformMethod {
  NONE,      // "A field" coded as per standard
  SWAPPING,  // "A field" coded as version/type/serialnumber instead of standard
             // serialnumber/version/type
  SAP_PRIOS, // Version and type not included in telegram. Must be hardcoded to
             // 0 and 7
  SAP_PRIOS_STANDARD // ?
};

const char *toString(DiehlAddressTransformMethod m);

// Diehl: Determines how to interpret frame
DiehlFrameInterpretation
detectDiehlFrameInterpretation(const std::vector<uchar> &frame);

// Diehl: Is "A field" coded differently from standard?
DiehlAddressTransformMethod
mustTransformDiehlAddress(const std::vector<uchar> &frame);

// Diehl: transform "A field" to make it compliant to standard
void transformDiehlAddress(std::vector<uchar> &frame,
                           DiehlAddressTransformMethod method);

// Diehl: Is payload real data crypted (LFSR)?
bool mustDecryptDiehlRealData(const std::vector<uchar> &frame);

// Diehl: decrypt real data payload (LFSR)
bool decryptDielhRealData(Telegram *t, std::vector<uchar> &frame,
                          std::vector<uchar>::iterator &pos,
                          const std::vector<uchar> &meterkey);

void qdsExtractWalkByField(Telegram *t, Meter *driver, DVEntry &mfctEntry,
                           int pos, int n, const std::string &key_s,
                           const std::string &fieldName, Quantity quantity);

#endif
