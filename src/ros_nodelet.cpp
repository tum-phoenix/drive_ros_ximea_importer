/******************************************************************************

Copyright 2016  Simon Schulz (University of Bielefeld)
                      [sschulz@techfak.uni-bielefeld.de]

All rights reserved.

********************************************************************************/

#include <string>
#include "ximea_camera/ros_nodelet.h"

using ximea_camera::RosNodelet;
using ximea_camera::RosDriver;

RosNodelet::RosNodelet() : running_(false) {
    // nothing to do
}

RosNodelet::~RosNodelet() {
    if (running_) {
        NODELET_INFO("ximea_camera: shutting down driver thread");
        running_ = false;
        deviceThread_->join();
        NODELET_INFO("ximea_camera: driver thread stopped");
    }
    if (drv_->hasValidHandle()) {
        drv_->stopAcquisition();
        drv_->closeDevice();
    }
}

void RosNodelet::onInit() {
    std::string camera_name;
    std::string config_filename;

    NODELET_DEBUG("ximea_camera: onInit()");

    ros::NodeHandle priv_nh(getPrivateNodeHandle());
    ros::NodeHandle node(getNodeHandle());

    if (!priv_nh.getParam("settings_yaml", config_filename)) {
        NODELET_ERROR("ximea_camera: failed to load camera settings. please pass config yaml "
                      "as parameter 'settings_yaml' "
                      "(e.g. using _settings_yaml:=\"ximea_mq022.yaml\")\n");
        exit(EXIT_FAILURE);
    }


    // create driver
    drv_.reset(new RosDriver(node, config_filename));

    // open device
    drv_->openDevice();

    // start acquisition
    drv_->startAcquisition();

    // spawn device thread
    running_ = true;
    deviceThread_ = boost::shared_ptr< boost::thread >
            (new boost::thread(boost::bind(&RosNodelet::devicePoll, this)));
}



void RosNodelet::devicePoll() {
    while (running_) {
        drv_->acquireImage();
        drv_->publishImageAndCamInfo();
    }
}

// Register this plugin with pluginlib.  Names must match nodelet_velodyne.xml.
PLUGINLIB_EXPORT_CLASS(ximea_camera::RosNodelet, nodelet::Nodelet);
