#include "sync_client.h"

#include <boost/asio.hpp>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

struct Document {
    std::string text;
    size_t version;
    std::time_t timestamp;
};

Document parseDoc(const std::string& data) {
    Document d;
    size_t first_pipe = data.find('|');
    size_t second_pipe = data.find('|', first_pipe + 1);

    d.version = std::stoull(data.substr(0, first_pipe));
    d.timestamp = std::stoll(data.substr(first_pipe + 1, second_pipe - first_pipe - 1));
    d.text = data.substr(second_pipe + 1);
    return d;
}

Syncer::Syncer(const std::string& ip, int port, const std::string& filename)
    : filename(filename), context(), socket(context), version(0), update_mutex() {
        socket.connect({boost::asio::ip::make_address(ip), static_cast<short unsigned int>(port)});
    }


std::string Syncer::set_file(const std::string& file) {
    filename = file;

    std::string data;
    update_from_remote(data);

    return data;
}

bool Syncer::update_from_remote(std::string& output_string) {
    if (update_mutex.try_lock()) {
        boost::asio::streambuf buf;
        boost::system::error_code ec;
    
        std::cout << "A\n";
        size_t bytes = boost::asio::read_until(socket, buf, '\0', ec);
        std::cout << bytes << '\n';
        if (ec && ec != boost::asio::error::eof) {
            std::cerr << "[Error] Failed to read from socket: " << ec.message() << "\n";
            return "";
        }
        std::cout << "B\n";
    
        std::istream is(&buf);
        std::string message((std::istreambuf_iterator<char>(is)),
                             std::istreambuf_iterator<char>());

        Document d = parseDoc(message);
        version = d.version + 1;
        
    
        std::cout << "[Update] File synced from server (" << bytes << " bytes).\n";
        output_string = std::move(d.text);
        update_mutex.unlock();
        return true;
    }
    std::cout << "[Skipped] update_from_remote already in progress.\n";
    return false;
}


void Syncer::write_to_remote(const std::string& contents) {
    // Add metadata to contents
    std::string version_str = "0";
    time_t currentTime_t = time(nullptr);
    std::string time = std::to_string(currentTime_t);
    boost::asio::write(socket, boost::asio::buffer(version_str + '|' + time + '|' + contents + '\0'));
    std::cout << "[Sent] Changes synced to server. Version: " << version << '\n';
}
