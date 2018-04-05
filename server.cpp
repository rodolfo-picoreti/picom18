#include <is/msgs/image.pb.h>
#include <chrono>
#include <is/rpc.hpp>
#include <is/rpc/interceptors/log-interceptor.hpp>
#include <is/rpc/interceptors/metrics-interceptor.hpp>
#include <thread>

int main(int argc, char** argv) {
  using namespace std::chrono;

  std::string uri;
  double cpu_boundedness;
  int low, high;

  is::po::options_description opts("Options");
  auto opt_add = opts.add_options();
  opt_add("uri,u", is::po::value<std::string>(&uri)->default_value("amqp://localhost"),
          "amqp broker uri");
  opt_add("low", is::po::value<int>(&low)->default_value(20), "lowest service latency");
  opt_add("high", is::po::value<int>(&high)->default_value(40), "highest service latency");
  opt_add("cpu", is::po::value<double>(&cpu_boundedness)->default_value(0.8),
          "service cpu boundedness [0.0,1.0]");
  is::parse_program_options(argc, argv, opts);

  assert(cpu_boundedness > 0.0 && cpu_boundedness < 1.0 &&
         "cpu_boundedness must be in the range [0.0,1.0]");
  assert(low < high && "low threshold must be smaller than the high threshold");

  prometheus::Exposer exposer("8080");  // Expose metrics at http://<ip>:8080/metrics
  auto registry = std::make_shared<prometheus::Registry>();
  exposer.RegisterCollectable(registry);
  auto& specific_metric =
      prometheus::BuildGauge().Name("specific_metric").Register(*registry).Add({});

  is::ServiceProvider provider;
  // Enable logging middleware for our services
  provider.add_interceptor(is::LogInterceptor{});
  // Enable metrics middleware for our services
  provider.add_interceptor(is::MetricsInterceptor{registry});
  // Connect to the AMQP broker
  is::info("Connecting to {}", uri);
  provider.connect(uri);
  auto tag = provider.declare_queue("MyService");

  // Random uniform distribution for the rpc latencies
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(low, high);

  // Service business logic mockup: simulation of an image processing service with a particular cpu
  // boundedness and a very application specific metric
  provider.delegate<is::vision::Image, is::vision::ImageAnnotations>(
      tag, "ProcImage", [&](auto const& request, auto* reply) {
        auto latency_ms = dis(gen);
        auto cpu_time_ms = latency_ms * cpu_boundedness;
        auto io_time_ms = latency_ms * (1.0 - cpu_boundedness);

        auto cpu_deadline =
            high_resolution_clock::now() + milliseconds(static_cast<int>(cpu_time_ms));
        while (high_resolution_clock::now() < cpu_deadline) {
        }

        // Simulate different behaviour based on the image content
        if (!request.data().empty() && request.data()[0] == 'x') {
          specific_metric.Set(1);
        } else {
          specific_metric.Set(0);
        }

        std::this_thread::sleep_for(milliseconds(static_cast<int>(io_time_ms)));
        return is::make_status(is::StatusCode::OK);
      });

  provider.run();
}
