#include <is/msgs/camera.pb.h>
#include <chrono>
#include <is/core.hpp>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  std::string uri;
  std::vector<int> rates;
  int n_req, rate_duration_s, packet_size;

  is::po::options_description opts("Options");
  auto opt_add = opts.add_options();
  opt_add("uri,u", is::po::value<std::string>(&uri)->default_value("amqp://localhost"),
          "amqp broker uri");
  opt_add("hz,f", is::po::value<std::vector<int>>(&rates)->multitoken()->required(),
          "list of requests frequency");
  opt_add("n_req,n", is::po::value<int>(&n_req)->default_value(4), "number of requests per period");
  opt_add("duration,d", is::po::value<int>(&rate_duration_s)->default_value(120),
          "duration of each rate in seconds");
  opt_add("packet_size,p", is::po::value<int>(&packet_size)->default_value(1 * 1024 * 1024),
          "size of the request packet");
  is::parse_program_options(argc, argv, opts);

  is::info("Connecting to {}", uri);
  auto channel = is::make_channel(uri);

  char payload = 'x';
  for (;;) {
    for (auto rate : rates) {
      // Simulate different type of payloads every rate
      payload = (payload == 'x') ? 'y' : 'x';

      is::vision::Image image;
      std::string bytes(packet_size, payload);
      image.set_data(bytes);

      auto message = is::pack_proto(image);
      auto deadline = is::current_time();
      auto period = is::pb::TimeUtil::NanosecondsToDuration(int(1.0e9 / rate));

      is::info("rate={} payload={}", rate, payload);
      for (int n = 0; n < rate_duration_s * rate; ++n) {
        deadline += period;
        is::set_deadline(message, deadline);

        // Simulate n_req publishers
        for (int i = 0; i < n_req; ++i)
          is::publish(channel, "MyService.ProcImage", message);

        auto delta_ns = is::pb::TimeUtil::DurationToNanoseconds(deadline - is::current_time());
        std::this_thread::sleep_for(std::chrono::nanoseconds(delta_ns));
      }
    }
  }
}
