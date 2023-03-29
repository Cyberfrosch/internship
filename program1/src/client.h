#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/locale.hpp>

class DataBuffer
{
private:
    std::string data;
    std::mutex mtx;
    std::condition_variable cv;
    bool is_data_available;
public:
    DataBuffer();
    void write(const std::string& input);
    std::string read();
};

class CInput
{
private:
    DataBuffer& buffer;
protected:
    static const std::string request_input;
public:
    explicit CInput(DataBuffer& buffer);
    void operator()();
    std::string write() const;
    static bool is_valid(const std::string& s);
    void stoi_vector(std::vector<int>& vec, const std::string& s) const;
    std::string filter(const std::vector<int>& vec, const std::string& change_to, const std::function<bool(int)>& condition) const;
};

class CProccess : public CInput
{
private:
    DataBuffer& buffer;
    bool is_connected;
public:
    explicit CProccess(DataBuffer& buffer);
    void operator()();
    int accumulation(const std::string& s) const;
    void send_to_server(const std::string& input, boost::asio::ip::tcp::socket& socket);
    void connect_to_server(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket& socket);
};
