# movetk-app-template
![CI-macOS](https://github.com/aniketmitra001/movetk-app-template/workflows/CI-macOS/badge.svg?branch=master&event=push) ![CI-Ubuntu](https://github.com/aniketmitra001/movetk-app-template/workflows/CI-Ubuntu/badge.svg?branch=master&event=push)

An example for using [MoveTK](https://github.com/heremaps/movetk) in your application

# Overview

This project demonstrates how to use [MoveTK](https://github.com/heremaps/movetk) in your application. In this example we will create an application that uses `MoveTK` to 

  - Read [GeoLife GPS Trajectories](https://www.microsoft.com/en-us/download/details.aspx?id=52367&from=https%3A%2F%2Fresearch.microsoft.com%2Fen-us%2Fdownloads%2Fb16d359d-d164-469e-9fd4-daa38f2b2e13%2F)
  - Compute trajectory statistics (length, duration, average speed...)
  - Write the trajectories in GeoJSON format
  - Visualize the trajectories in [kepler.gl](https://kepler.gl/)

  ![visualization](kepler_viz.html)

# Setup

You can build this project locally by replicating the steps in the workflow for [Mac](https://github.com/aniketmitra001/movetk-app-template/blob/master/.github/workflows/build-macos.yml) and [Ubuntu](https://github.com/aniketmitra001/movetk-app-template/blob/master/.github/workflows/build-ubuntu.yml)

# Build Artifacts

The workflow creates the following build artifacts
  - A **trajectories.csv** file that is generated by running the ```geolife.py``` script on the downloaded GeoLife trajectories
  - A **trajectory_statistics.csv** file that contains the stastics per trajectory. It is generated by running ```process_trajectories``` on trajectories.csv
  - An **output_trajectories.geojson** file that contains the trajectories in GeoJSON format. This file too gets generated on running ```process_trajectories``` 

These files can be accessed from the *Artifacts* section of the latest run of the workflow
