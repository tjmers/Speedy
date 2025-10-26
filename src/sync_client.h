#pragma once

#include <boost/asio.hpp>

#include <mutex>
#include <string>


using boost::asio::ip::tcp;

class Syncer {
    
public:

    Syncer(const std::string& ip, int port, const std::string& filename);

    bool update_from_remote(std::string& remote);
    
    
    void write_to_remote(const std::string& contents);

    std::string set_file(const std::string& file);

private:
    
    std::string filename;
    boost::asio::io_context context;
    tcp::socket socket;
    int version;
    std::mutex update_mutex;

};