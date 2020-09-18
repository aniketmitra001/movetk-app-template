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

//
// Created by Custers, Bram on 2020-02-08.
//


#include <vector>
#include "movetk/logging.h"
#include "movetk/test_data.h"
#include "movetk/GeolifeTrajectoryTraits.h"
#include "movetk/io/ProbeReader.h"
#include "movetk/TrajectoryReader.h"
#include "movetk/io/GeoJSONUtils.h"
#include "movetk/algo/Statistics.h"
#include "movetk/geo/geo.h"
#include "GeometryBackendTraits.h"


/**
 * Example: Process a stream of probe points to create a trajectory. Then,
 * display statistics of the trajectory.
 */
template<typename TrajectoryTraits>
void run(int argc, char** argv)
{
    using ProbeTraits = typename TrajectoryTraits::ProbeTraits;

    // Create trajectory reader
    std::unique_ptr<ProbeReader<ProbeTraits>> probe_reader;
    if (argc < 2) {
        // Use built-in test data if a file is not specified
        probe_reader = ProbeReaderFactory::create_from_string<ProbeTraits>(testdata::c2d_format_geolife_csv);
    }
    else {
        // Example: Process trajectories from a (zipped) CSV file (e.g., probe_data_lametro.20180918.wayne.csv.gz)
        probe_reader = ProbeReaderFactory::create<ProbeTraits>(argv[1]);
    }
    using ProbeInputIterator = decltype(probe_reader->begin());
    auto trajectory_reader = TrajectoryReader<TrajectoryTraits, ProbeInputIterator>(probe_reader->begin(), probe_reader->end());

    /**
     * The points in the trajectory are assumed to be lat-lon coordinates. So, use the appropriate distance between the coordinates.
     */
    struct Distance
    {
        GeometryKernel::NT operator()(GeometryKernel::MovetkGeometryKernel::MovetkPoint p0, GeometryKernel::MovetkGeometryKernel::MovetkPoint p1)
        {
            std::vector<GeometryKernel::NT> ll0;
            std::copy(p0.begin(), p0.end(), std::back_inserter(ll0));
            std::vector<GeometryKernel::NT> ll1;
            std::copy(p1.begin(), p1.end(), std::back_inserter(ll1));
            return distance_exact(ll0[1], ll0[0], ll1[1], ll1[0]);
        }
    };

    // Process trajectories in a streaming fashion
    std::size_t count = 0;
    // Shorthands for fields
    constexpr int LON_Idx = ProbeTraits::ProbeColumns::LON;
    constexpr int LAT_Idx = ProbeTraits::ProbeColumns::LAT;
    constexpr int TS_Idx = ProbeTraits::ProbeColumns::DATETIME;
    constexpr int TRAJID = ProbeTraits::ProbeColumns::TRAJID;


    // Create an output geojson file
    std::ofstream ofjson("output_trajectories.geojson");
    ofjson.setf(std::ios::fixed);

    std::vector<typename TrajectoryTraits::trajectory_type> trajectories;

    // Create an output csv file
    std::ofstream ofcsv_stats("trajectory_statistics.csv");
    ofcsv_stats.setf(std::ios::fixed);
    ofcsv_stats<<"TRAJECTORY_ID, NUM_POINTS, LENGTH, DURATION, AVG_SPEED, MEDIAN_SPEED, MIN_SPEED, MAX_SPEED, VAR_SPEED, TIME_MODE\n";
    for (auto trajectory : trajectory_reader) {

        trajectories.push_back(trajectory);
        ofcsv_stats<< trajectory.template get<TRAJID>()[0] <<",";
        ofcsv_stats<<trajectory.size()<<",";
        using Trajectory_t = decltype(trajectory);
        using GeomKernel = GeometryKernel::MovetkGeometryKernel;

        Distance distanceFunc;

        movetk_algorithms::TrajectoryLength<Trajectory_t, GeomKernel, Distance, LON_Idx, LAT_Idx> lenCalc(distanceFunc);
        ofcsv_stats<<lenCalc(trajectory)<<",";
        movetk_algorithms::TrajectoryDuration<Trajectory_t, TS_Idx> duration;
        ofcsv_stats<<duration(trajectory)<<",";

        // Show speed statistics:
        using SpeedStat = movetk_algorithms::TrajectorySpeedStatistic<Trajectory_t, GeomKernel, Distance, LON_Idx, LAT_Idx, TS_Idx>;
        SpeedStat speedStat(distanceFunc);
        using Stat = typename SpeedStat::Statistic;
        ofcsv_stats<<speedStat(trajectory,Stat::Mean)<<",";
        ofcsv_stats<<speedStat(trajectory, Stat::Median)<<",";
        ofcsv_stats<<speedStat(trajectory, Stat::Min)<<",";
        ofcsv_stats<<speedStat(trajectory, Stat::Max)<<",";
        ofcsv_stats<<speedStat(trajectory, Stat::Variance)<<",";
        // Show time mode
        movetk_algorithms::TrajectoryTimeIntervalMode<Trajectory_t, TS_Idx> timeMode;
        ofcsv_stats<<timeMode(trajectory)<<"\n";

        count++;
    }

    movetk_io::trajectories_to_geojson<
            typename std::vector<typename TrajectoryTraits::trajectory_type>::iterator,
            ProbeTraits::ProbeColumns::LAT,
            ProbeTraits::ProbeColumns::LON>(ofjson,
                                            trajectories.begin(),
                                            std::prev(trajectories.end()));
}

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    run<geolife::c2d::raw::TabularTrajectoryTraits>(argc, argv);
    return 0;
}
