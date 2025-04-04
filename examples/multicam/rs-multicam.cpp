// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp>     // Include RealSense Cross Platform API
#include <iostream>
#include <map>
#include <vector>
#include "third-party/stb_image_write.h"
#include <unistd.h>  // for sleep()

int main(int argc, char * argv[]) try
{
    rs2::context                          ctx;        // Create librealsense context for managing devices

    std::vector<rs2::pipeline>            pipelines;

    // Capture serial numbers before opening streaming
    std::vector<std::string>              serials;
    for (auto&& dev : ctx.query_devices())
        serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));

    // Start a streaming pipe per each connected device
    for (auto&& serial : serials)
    {
        rs2::pipeline pipe(ctx);
        rs2::config cfg;
        cfg.enable_device(serial);
        pipe.start(cfg);
        pipelines.emplace_back(pipe);
        // Map from each device's serial number to a different colorizer
    }

    // We'll keep track of the last frame of each stream available to make the presentation persistent
    std::map<int, rs2::frame> render_frames;

    // Main app loop
    while (true)
    {
        sleep(1);
        // Collect the new frames from all the connected devices
        std::vector<rs2::depth_frame> new_frames;
        for (auto &&pipe : pipelines)
        {
            rs2::frameset fs;
            if (pipe.poll_for_frames(&fs))
            {
                new_frames.emplace_back(fs.get_depth_frame());
            }
        }

        // Convert the newly-arrived frames to render-friendly format
        for (const auto& depth : new_frames)
        {
            // Get the serial number of the current frame's device
            auto serial = rs2::sensor_from_frame(depth)->get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
            // Apply the colorizer of the matching device and store the colorized frame

            // Get the depth frame's dimensions
            auto width = depth.get_width();
            auto height = depth.get_height();

            // Query the distance from the camera to the object in the center of the image
            float dist_to_center = depth.get_distance(width / 2, height / 2);

            // Print the distance
            std::cout << "The camera " << std::string(serial) << " is facing an object " << dist_to_center << " meters away \n";
        }
    }

    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
