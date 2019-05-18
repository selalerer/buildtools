
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <sstream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

void usage(const char* exeName)
{
	std::cerr << exeName << " cpp_file_name obj_file_name" << std::endl;
}

const int COMPILE=0;
const int DONT_COMPILE=1;
bool debug = false;

class NeedCompilation
{
private:
	std::unordered_set<std::string> checkedHeaders;
	std::ostream& debug;
public:
	NeedCompilation(std::ostream& debug) : debug(debug) {}

	bool needCompile(const fs::path& projectPath, const fs::path& cppPath, const fs::path& objPath)
	{
		if (!fs::exists(objPath) || !fs::exists(cppPath))
		{
			return true;
		}

		auto objTime = fs::last_write_time(objPath);
		auto cppTime = fs::last_write_time(cppPath);

		if (cppTime > objTime)
		{
			return true;
		}

		std::ifstream cppFile(cppPath);
		if (!cppFile.good())
		{
			std::cerr << "Can't read file " << cppPath << std::endl;
			return true;
		}

		return checkHeaders(cppFile, projectPath, objTime);
	} 

private:
	bool checkHeaders(std::ifstream& content, const fs::path& projectPath, const fs::file_time_type& objTime)
	{
		std::string line;
		while (std::getline(content, line))
		{
			std::stringstream ss(line);
			std::string token;
			ss >> token;
			
			bool isInclude = token == "#include";
			if (token != "#" && !isInclude)
				continue;
			
			if (!isInclude)
			{
				ss >> token;
				isInclude = token == "include";
			}

			if (!isInclude)
				continue;

			std::string includedFile;
			ss >> includedFile;

			if (includedFile.size() < 3)
				continue;

			char startChar = includedFile[0];
			char lastChar = includedFile[includedFile.size() - 1];

			bool isQuoted = startChar == '\"' && lastChar == '\"';
			if (!isQuoted)
				isQuoted = startChar == '<' && lastChar == '>';

			if (!isQuoted)
				continue;

			includedFile = includedFile.substr(1, includedFile.size() - 2);
			debug << "DEBUG: Included file: " << includedFile << std::endl;

			fs::path incFilePath = includedFile;

			if (checkedHeaders.count(incFilePath) != 0)
				continue;

			if (!fs::exists(incFilePath))
			{
				debug << "DEBUG: File " << incFilePath << " not found. Skipping it." << std::endl;
				continue;
			}

			auto incFileTime = fs::last_write_time(incFilePath);

			if (incFileTime > objTime)
				return true;

			checkedHeaders.insert(incFilePath);

			std::ifstream incFile(incFilePath);
			if (!incFile.good())
			{
				std::cerr << "Can't read file " << incFilePath << std::endl;
				return true;
			}

			if (checkHeaders(incFile, projectPath, objTime))
				return true;
		}

		return false;
	}
};

int main (int argc, char* argv[])
{
	if (argc < 3)
	{
		usage(argv[0]);
		return COMPILE;
	}

	std::ostream* pDebug = &std::cerr;
	if (!debug)
	{
		pDebug = new std::stringstream();
		pDebug->setstate(std::ios_base::badbit);
	}

	fs::path projectPath = fs::current_path();
	fs::path cppPath = argv[1];
	fs::path objPath = argv[2];

	NeedCompilation needCompilation(*pDebug);
	int result = needCompilation.needCompile(projectPath, cppPath, objPath);

	//std::cout << (result ? "COMPILE" : "DONT_COMPILE");

	return result ? COMPILE : DONT_COMPILE;
}


