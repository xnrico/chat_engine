#include "client/sessions/camera_streamer.hpp"

std::unordered_map<int, std::tuple<std::shared_ptr<rtc::Description::Video>, int, int>>
    camera_streamer::media_map{};  // map media port to [description, socket, freq]