#include <iostream>
#include <tuple>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

template <typename T>
T setArgument(std::string inputValue)
{
	T arg;  std::stringstream buffer;

	buffer << inputValue;
	buffer >> arg;
	
	return arg;
}

template <typename Ch, typename Tr, size_t I, typename... Args>
class TuplePrinter
{
public:
	static void print(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
	{
		TuplePrinter<Ch, Tr, I - 1, Args...>::print(os, t);
		if (I < sizeof...(Args))
			os << " ";
		os << std::get<I>(t);
	}
};

template <typename Ch, typename Tr, typename... Args>
class TuplePrinter<Ch, Tr, 0, Args...>
{
public:
	static void print(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
	{
		os << std::get<0>(t);
	}
};

template <typename Ch, typename Tr, typename... Args>
class TuplePrinter<Ch, Tr, -1, Args...>
{
public:
	static void print(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t) {}
};

template <typename Ch, typename Tr, typename... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
{
	TuplePrinter<Ch, Tr, sizeof...(Args) - 1, Args...>::print(os, t);
}

template<typename... Args>
class CSVParser
{
public:
	CSVParser(std::ifstream& file, size_t skipLines, char delimeter=',', char safe='\"') : _file(file)
	{
		this->delimiter		= delimeter;
		this->saveSymbol	= safe;
		this->_readRows		= 0;
		this->_skipLines	= skipLines;

		if (!file.is_open()) throw std::runtime_error("File is not opened!");

		std::string __trash;
		while (this->_readRows < this->_skipLines)
		{
			std::getline(file, __trash);
			this->_readRows++;
		}

		//Start to pasre a file
		this->buffer = this->getString();
	}

	~CSVParser() = default;

	std::tuple<Args...> getString()
	{
		std::string					__readString;
		std::vector<std::string>	__splittedString;

		std::vector<std::string> output = getData();

		if(this->_file.eof()) return std::tuple<Args...>();

		auto flip = [&output]() -> std::string
		{
			std::string s = output.back();
			output.pop_back();
			return s;
		};

		if (output.size() != sizeof...(Args))	throw std::runtime_error("Invalid string");
		if (isSafe == true)						throw std::runtime_error("The save data wasn't closed!");
	
		return std::tuple<Args...>(setArgument<Args>(flip())...);
	}

	//Class iterator
	class CSVInputIterator
	{
	public:
		using iterator_category	= std::input_iterator_tag;
		using value_type		= std::tuple<Args...>;
		using difference_type	= std::ptrdiff_t;
		using pointer			= value_type*;
		using reference			= value_type&;

		CSVInputIterator(CSVParser<Args...>* _object = nullptr) : object(_object) {};
		CSVInputIterator(const CSVInputIterator&) = default;
		CSVInputIterator& operator=(const CSVInputIterator&) = default;

		friend void swap(CSVInputIterator& a, CSVInputIterator& b) { std::swap(a.object, b.object); }

		friend bool operator==(CSVInputIterator& a, CSVInputIterator& b) { return a.object == b.object; }
		friend bool operator!=(CSVInputIterator& a, CSVInputIterator& b) { return a.object != b.object; }

		value_type operator*() { if(!object->isEof) return object->buffer; }

		CSVInputIterator& operator++()
		{
			object->buffer = object->getString();

			if (object->isEof == true) object = nullptr;

			return *this;
		}

		CSVParser<Args...>* object;
	};

	CSVInputIterator begin() { return CSVInputIterator(this); }
	CSVInputIterator end() { return CSVInputIterator(); }

private:
	size_t				_readRows;
	size_t				_skipLines;
	std::ifstream&		_file;
	std::vector<char>	_symbolsDividers;
	std::tuple<Args...>	 buffer;

	char				delimiter;
	char				saveSymbol;
	bool				isSafe = false;
	bool				isEof = false;

	std::vector<std::string> getData()
	{
		std::string					__readString;
		std::vector<std::string>	__splittedString;
		std::vector <std::string>	__data;

		std::getline(this->_file, __readString);

		if(_file.eof())
		{
			this->isEof = true;
			return std::vector<std::string>();
		}

		this->_readRows++;

		size_t start = 0, end = 0;
		while ((start = __readString.find_first_not_of(delimiter, end)) != std::string::npos)
		{
			end = __readString.find(delimiter, start);
			__splittedString.push_back(__readString.substr(start, end - start));
		}

		std::string value;
		for (auto x : __splittedString)
		{
			value += x;

			if (x.find(saveSymbol) != std::string::npos)
			{
				isSafe = !isSafe;
			}

			if (!isSafe)
			{
				__data.push_back(value);
				value = "";
			}

			if (isSafe)
			{
				value += delimiter;
			}
		}

		if (this->_file.eof()) this->isEof = true;

		return __data;
	}
};


int main()
{
	std::ifstream fin("input.csv");
	CSVParser<int, std::string, double, int, int, int, double, int, double, int, std::string> p(fin, 3, ',' ,'/');

	for (auto x : p)
	{
		std::cout << x;
		std::cout << std::endl;
	}

	return 0;
}

