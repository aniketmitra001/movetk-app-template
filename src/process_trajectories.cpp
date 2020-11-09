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
#include <sstream>
#include <iomanip>
#include "movetk/logging.h"
#include "movetk/test_data.h"
#include "movetk/GeolifeTrajectoryTraits.h"
#include "movetk/io/ProbeReader.h"
#include "movetk/TrajectoryReader.h"
#include "movetk/io/GeoJSONUtils.h"
#include "movetk/io/GeoJSON.h"
#include "movetk/algo/Statistics.h"
#include "movetk/geo/geo.h"
#include "GeometryBackendTraits.h"

template <class T>
std::string create_gradient(T value, std::size_t min,
                            std::size_t max, std::size_t bucket_size)
{
    std::size_t steps = (max - min) / bucket_size;
    std::size_t buckets = 255 / bucket_size;
    std::size_t num_buckets = buckets < steps ? buckets : steps;
    std::size_t idx = 0;
    while (idx++ != (num_buckets - 1))
    {
        std::size_t ll = (idx - 1) * bucket_size + min;
        std::size_t ul = idx * bucket_size + min;
        if ((value > ll) & (value <= ul))
        {
            std::ostringstream hex_code;
            std::size_t r = 255;
            std::size_t g = 20 * idx * bucket_size;
            std::size_t b = 20 * idx * bucket_size;
            hex_code << "#";
            hex_code << std::setfill('0') << std::setw(2) << std::hex << r;
            hex_code << std::setfill('0') << std::setw(2) << std::hex << g;
            hex_code << std::setfill('0') << std::setw(2) << std::hex << b;
            return hex_code.str();
        }
    }

    return "#ffffff";
}

/**
 * Example: Process a stream of probe points to create a trajectory. Then,
 * display statistics of the trajectory.
 */
template <typename TrajectoryTraits>
void run(int argc, char **argv)
{
    using ProbeTraits = typename TrajectoryTraits::ProbeTraits;

    // Create trajectory reader
    std::unique_ptr<ProbeReader<ProbeTraits>> probe_reader;
    if (argc < 2)
    {
        // Use built-in test data if a file is not specified
        probe_reader = ProbeReaderFactory::create_from_string<ProbeTraits>(testdata::c2d_format_geolife_csv);
    }
    else
    {
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

    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);

    // Create an output geojson file
    std::ofstream ofjson("output_trajectories.geojson");
    ofjson.setf(std::ios::fixed);

    ofjson << "{\"type\":\"FeatureCollection\",\"features\":[";
    //std::vector<typename TrajectoryTraits::trajectory_type> trajectories;

    std::ofstream ofcsv("trajectory.csv");
    ofcsv << "Traectory_ID,TRIP_GEOJSON\n";

    // Create an output csv file
    std::ofstream ofcsv_stats("trajectory_statistics.csv");
    ofcsv_stats.setf(std::ios::fixed);
    ofcsv_stats << "TRAJECTORY_ID, NUM_POINTS, LENGTH, DURATION, AVG_SPEED, MEDIAN_SPEED, MIN_SPEED, MAX_SPEED, VAR_SPEED, TIME_MODE\n";
    for (auto trajectory : trajectory_reader)
    {

        auto trajectory_id = trajectory.template get<TRAJID>()[0];
        auto num_points = trajectory.size();
        //trajectories.push_back(trajectory);
        ofcsv_stats << trajectory_id << ",";
        ofcsv_stats << num_points << ",";
        using Trajectory_t = decltype(trajectory);
        using GeomKernel = GeometryKernel::MovetkGeometryKernel;

        Distance distanceFunc;

        movetk_algorithms::TrajectoryLength<Trajectory_t, GeomKernel, Distance, LON_Idx, LAT_Idx> lenCalc(distanceFunc);
        auto length = lenCalc(trajectory);
        ofcsv_stats << length << ",";
        movetk_algorithms::TrajectoryDuration<Trajectory_t, TS_Idx> duration;
        auto traj_duration = duration(trajectory);
        ofcsv_stats << traj_duration << ",";

        // Show speed statistics:
        using SpeedStat = movetk_algorithms::TrajectorySpeedStatistic<Trajectory_t, GeomKernel, Distance, LON_Idx, LAT_Idx, TS_Idx>;
        SpeedStat speedStat(distanceFunc);
        using Stat = typename SpeedStat::Statistic;
        auto mean_speed = speedStat(trajectory, Stat::Mean);
        auto median_speed = speedStat(trajectory, Stat::Median);
        auto min_speed = speedStat(trajectory, Stat::Min);
        auto max_speed = speedStat(trajectory, Stat::Max);
        auto var_speed = speedStat(trajectory, Stat::Variance);

        ofcsv_stats << mean_speed << ",";
        ofcsv_stats << median_speed << ",";
        ofcsv_stats << min_speed << ",";
        ofcsv_stats << max_speed << ",";
        ofcsv_stats << var_speed << ",";
        // Show time mode
        movetk_algorithms::TrajectoryTimeIntervalMode<Trajectory_t, TS_Idx> timeMode;

        auto time_mode = timeMode(trajectory);
        ofcsv_stats << time_mode << "\n";

        std::vector<std::pair<std::string, std::string>> trajectory_properties({std::make_pair("trajectory_id", trajectory_id),
                                                                                std::make_pair("num_points", std::to_string(num_points)),
                                                                                std::make_pair("length", std::to_string(length)),
                                                                                std::make_pair("duration", std::to_string(traj_duration)),
                                                                                std::make_pair("mean_speed", std::to_string(mean_speed)),
                                                                                std::make_pair("median_speed", std::to_string(median_speed)),
                                                                                std::make_pair("min_speed", std::to_string(min_speed)),
                                                                                std::make_pair("max_speed", std::to_string(max_speed)),
                                                                                std::make_pair("var_speed", std::to_string(var_speed)),
                                                                                std::make_pair("time_mode", std::to_string(time_mode))});

        //write GeoJSON
        GeoJSONGeometry geom;
        GeoJSONGeometry geom1;
        GeoJSONProperties prop;
        GeoJSONProperties prop1;
        GeoJSONFeature feat;
        GeoJSONFeature feat1;

        auto lats = trajectory.template get<LAT_Idx>();
        auto lons = trajectory.template get<LON_Idx>();
        auto ts = trajectory.template get<TS_Idx>();

        std::vector<int> timestamps;
        using Time_t = decltype(*std::begin(ts));
        std::transform(std::begin(ts), std::end(ts), std::back_inserter(timestamps),
                       [](auto val) -> int { return static_cast<int>(val); });

        rapidjson::Document geometry = geom(std::begin(lats), std::end(lats),
                                            std::begin(lons), std::begin(timestamps));
        std::string colour_code = create_gradient(speedStat(trajectory, Stat::Median), 0, 24, 3);
        //rapidjson::Document properties = prop(trajecoty_id, colour_code, 4);
        rapidjson::Document properties = prop(std::cbegin(trajectory_properties), std::cend(trajectory_properties));
        rapidjson::Document feature = feat(geometry, properties);

        feature.Accept(writer);
        ofjson << strbuf.GetString();
        ofjson << ",";
        strbuf.Clear();

        rapidjson::Document geometry1 = geom1(std::begin(lats), std::end(lats),
                                              std::begin(lons), std::begin(timestamps));
        rapidjson::Document properties1 = prop1(std::cbegin(trajectory_properties), std::cend(trajectory_properties));
        rapidjson::Document feature1 = feat1(geometry1, properties1);

        feature1.Accept(writer);
        ofcsv << trajectory_id << ",";
        ofcsv << strbuf.GetString() << "\n";
        strbuf.Clear();

        count++;
    }

    ofjson << "]}" << std::endl;
    // movetk_io::trajectories_to_geojson<
    //         typename std::vector<typename TrajectoryTraits::trajectory_type>::iterator,
    //         ProbeTraits::ProbeColumns::LAT,
    //         ProbeTraits::ProbeColumns::LON>(ofjson,
    //                                         trajectories.begin(),
    //                                         std::prev(trajectories.end()));
}

int main(int argc, char **argv)
{
    std::ios_base::sync_with_stdio(false);
    run<geolife::c2d::raw::TabularTrajectoryTraits>(argc, argv);
    return 0;
}
