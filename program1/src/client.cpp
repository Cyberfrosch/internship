#include "client.h"

const std::string CInput::request_input = "Enter a string of numbers: ";

DataBuffer::DataBuffer() : data(), mtx(), cv(), is_data_available()
{
	
}

void DataBuffer::write(const std::string& input)
{
    std::unique_lock<std::mutex> locker(mtx);
    cv.wait(locker, [this] { return !is_data_available; });

    data = input;

    is_data_available = true;
    locker.unlock();
    cv.notify_one();
}

std::string DataBuffer::read()
{
    std::unique_lock<std::mutex> locker(mtx);
    cv.wait(locker, [this] { return is_data_available; });

    std::string input = data;
    data.clear();

    is_data_available = false;
    locker.unlock();
    cv.notify_one();

    return input;
}

CInput::CInput(DataBuffer& buffer) : buffer(buffer)
{
	
}

void CInput::operator()()
{
    while (true)
    {
        // Задержка для последовательного вывода сообщений двух потоков
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        std::string input;
        input = write();

        if (!is_valid(input))
        {
            std::cout << "Input error! Enter a string consisting only of digits and not exceeding 64 characters!\n\n";
            continue;
        }

        std::vector<int> numbers;
        stoi_vector(numbers, input);

        std::sort(numbers.rbegin(), numbers.rend());
        // Фильтр элементов вектора: лямбда-функция определяет четные элементы, а фильтр заменяет их на "KB"
        input = filter(numbers, "KB", [](const int el) { return el % 2 == 0; });

        buffer.write(input);
    }
}

std::string CInput::write() const
{
    std::string input;

    std::cout << request_input;
    std::getline(std::cin, input);

    return input;
}

bool CInput::is_valid(const std::string& s)
{
    boost::locale::generator gen;
    std::locale loc = gen("");
    bool is_valid = true;

    // Строка больше 64 символов или состоит только из пробелов (используется параметр локали для корректной работы в любых кодировках)
    if (s.size() > 64 || std::all_of(s.begin(), s.end(), [&loc](unsigned char c) { return std::isspace(c, loc); }))
        is_valid = false;
    else
    {
        for (std::string::size_type i = 0; i < s.length(); i++)
        {
            if (s[i] == '-')
            {
                // FALSE если "-" последний символ или следующий символ после "-" не цифра
                if (i + 1 > s.length() || !std::isdigit(s[i + 1], loc))
                {
                    is_valid = false;
                    break;
                }
            }
            // FALSE - если иной символ
            else if (!std::isdigit(s[i], loc) && !std::isspace(s[i], loc))
            {
                is_valid = false;
                break;
            }
        }
    }

    return is_valid;
}

void CInput::stoi_vector(std::vector<int>& vec, const std::string& s) const
{
    vec.clear();
    std::string::size_type pos = 0;
    while (pos < s.size())
    {
        // Нахождение позиции начала числа
        while (pos < s.size() && std::isspace(s[pos]))
            pos++;
        if (pos == s.size())
            break;

        // Нахождение позиции конца числа
        std::string::size_type next_pos = s.find(' ', pos);
        if (next_pos == std::string::npos)
            next_pos = s.size();
        try
        {
            vec.push_back(std::stoi(s.substr(pos, next_pos)));
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }

        // Переход к следующему элементу
        pos = next_pos;
    }
}

// Параметр лямбда-функция для гибкости метода
std::string CInput::filter(const std::vector<int>& vec, const std::string& change_to,
	const std::function<bool(int)>& condition) const
{
    std::stringstream ss;
    for (const auto& now : vec)
        ss << (condition(now) ? change_to : std::to_string(now)) << ' ';
    return ss.str();
}

CProccess::CProccess(DataBuffer& buffer) : CInput(buffer), buffer(buffer), is_connected(false)
{
	
}

void CProccess::operator()()
{
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket(io_context);
    while (true)
    {
        std::string input = buffer.read();
        std::cout << "The entered string: " << input << "\n\n";

        const int sum = accumulation(input);

        if (!is_connected)
            connect_to_server(io_context, socket);
        if (is_connected)
            send_to_server(std::to_string(sum) + '\n', socket);
    }
}

int CProccess::accumulation(const std::string& s) const
{
    int sum = 0;
    std::string::size_type pos = 0;
    while (pos < s.size())
    {
        // Нахождение позиции начала числа
        while (pos < s.size() && !std::isdigit(s[pos]) && s[pos] != '-')
            pos++;
        if (pos == s.size())
            break;

        // Нахождение позиции конца числа
        std::string::size_type next_pos = s.find(' ', pos);
        if (next_pos == std::string::npos)
            next_pos = s.size();
        try
        {
            // Отрицательное число: выделение подстроки от позиции следующей за "-" до конечной, преобразование в int и вычитание из суммы
            if (s[pos] == '-')
                sum -= std::stoi(s.substr(pos + 1, next_pos));
            else
                sum += std::stoi(s.substr(pos, next_pos));
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }

        // Переход к следующему числу
        pos = next_pos;
    }
    return sum;
}

void CProccess::send_to_server(const std::string& input, boost::asio::ip::tcp::socket& socket)
{
    try
    {
        boost::system::error_code error;
        boost::asio::write(socket, boost::asio::buffer(input), error);
        if (error)
            is_connected = false;
    }
    catch (const boost::system::system_error& err)
    {
        std::cerr << "\n\n" << err.what() << "\n\n";
    	std::cout << request_input;
        is_connected = false;
    }
}

void CProccess::connect_to_server(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket& socket)
{
    try
    {
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("localhost", "8888");
        boost::asio::connect(socket, endpoints);
        is_connected = true;
    }
    catch (const boost::system::system_error& err)
    {
        std::cerr << "\n\n" << err.what() << "\n\n";
    	std::cout << request_input;
        is_connected = false;
    }
}
