#include <thread>
#include <chrono>


int main() {
    while (true) {
        std::string json = runCommand("curl http://127.0.0.1:5000/ping");

        std::vector<std::string> lines;
        ArrayParser parser(json);

        if (parser.parse(lines)) {
            std::cout << "Received response:\n";
            for (const auto& line : lines) {
                std::cout << line << "\n";
            }
        } else {
            std::cout << "Failed to parse JSON!\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
