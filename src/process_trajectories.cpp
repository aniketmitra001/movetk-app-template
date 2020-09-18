/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * License-Filename: LICENSE
 */

#include "movetk/GeolifeTrajectoryTraits.h"
#include "movetk/io/ProbeReader.h"
#include "movetk/TrajectoryReader.h"
#include "movetk/io/GeoJSONUtils.h"

//
// Created by Mitra, Aniket on 2020-09-18.
//

// Specializations for the Geolife raw probe format
using TrajectoryTraits = geolife::c2d::raw::TabularTrajectoryTraits;
using ProbeTraits = typename TrajectoryTraits::ProbeTraits;

int main(int argc, char **argv) {
	  return 0;
}
