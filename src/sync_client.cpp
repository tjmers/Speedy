#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <filesystem>

using boost::asio::ip::tcp;

std::atomic<bool> running(true);

void listen_for_updates(tcp::socket& socket, const std::string& filename) {
    boost::asio::streambuf buf;
    while (running) {
        boost::system::error_code ec;
        size_t bytes = boost::asio::read_until(socket, buf, "\n", ec);
        if (ec) break;
        std::istream is(&buf);
        std::string message;
        std::getline(is, message);

        std::ofstream file(filename, std::ios::trunc);
        file << message;
        file.close();

        std::cout << "[Update] File synced from server.\n";
    }
}

void watch_file(tcp::socket& socket, const std::string& filename) {
    std::string last_content;

    while (running) {
        std::ifstream file(filename);
        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

        if (content != last_content) {
            boost::asio::write(socket, boost::asio::buffer(content + "\n"));
            last_content = content;
            std::cout << "[Sent] Changes synced to server.\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./sync_client <server_ip> <filename>\n";
        return 1;
    }

    std::string server_ip = argv[1];
    std::string filename = argv[2];

    boost::asio::io_context io;
    tcp::socket socket(io);
    socket.connect({boost::asio::ip::make_address(server_ip), 8080});

    std::thread listener(listen_for_updates, std::ref(socket), filename);
    std::thread watcher(watch_file, std::ref(socket), filename);

    std::cout << "Connected to server. Syncing file: " << filename << "\n";
    listener.join();
    watcher.join();
}
