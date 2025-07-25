#include <cstring>
#include <cstdio>

#include "vector"
#include "string"

#include "filesystem"

using namespace std;

string mainDir = string();
string builderDir = string();

bool whitelistLibs = false;
bool whitelistAssets = false;
bool keepSettings = false;
bool customBuild = false;
bool pause = false;
vector<string> libraryData = vector<string>();
vector<string> assetsData = vector<string>();

vector<pair<string, string>> installLog = vector<pair<string, string>>();

#define ARRAY_COUNT(arr) sizeof(arr) / sizeof(arr[0])

#define NULL_CHARS string(" \n\t\"")
#define SEPARATOR '\\'
#define NOT_SEPARATOR '/'

#define PATH_FORMAT(path) (string("\"") + path + string("\""))

#define GLOBAL_DIR PathConcat(mainDir, "Global")
#define HEADER_DIR PathConcat(mainDir, "Headers")
#define PATCH_DIR PathConcat(mainDir, "Patches")
#define LIB_DIR PathConcat(mainDir, "Libraries")
#define ASSETS_DIR PathConcat(mainDir, "Assets")
#define EXTERNAL_DIR PathConcat(mainDir, "Externals")

#define BUILDER_DIR PathConcat(builderDir, "Builder")
#define BUILD_DIR PathConcat(BUILDER_DIR, "build")

#define ESDB_DIR PathConcat(mainDir, "ESDB.yml")

#define CTRMAP_DIR "%CTRMAP_DIR%"
#define ARM_NONE_EABI_DIR "%ARM_NONE_EABI_DIR%"
#define JAVA_DIR "%JAVA_DIR%"

#define SWAN_DIR "swan"
#define EXTLIB_DIR "ExtLib"
#define NK_DIR "NitroKernel\\include"
#define LIBRPM_DIR "libRPM\\include"

#define PROJECT_ASSETS_DIR(projectDir) string(projectDir) + "vfs\\data"
#define PROJECT_PATCH_DIR(projectDir) PathConcat(PROJECT_ASSETS_DIR(projectDir), "patches")
#define PROJECT_LIB_DIR(projectDir) PathConcat(PROJECT_ASSETS_DIR(projectDir), "lib")

#ifdef _WIN32
#define SCRIPT_TERMINATION ".bat"
#else
#define SCRIPT_TERMINATION ".sh"
#endif
#define SCRIPT_PATH(script) PathConcat(BUILD_DIR, string(script) + SCRIPT_TERMINATION)

#define ECHO_TITLE(str) string("echo ==============================\necho ") + str + "\necho ==============================\n"
#define ECHO_SUBTITLE(str) string("echo ---\necho ") + str + "\necho ---\n"
#define ECHO(str) string("echo ") + str + '\n'
#define ECHO_TAB(str) ""// string("echo \t") + str + '\n'

#define COMPILER string("\"") + ARM_NONE_EABI_DIR + "arm-none-eabi-g++" + "\" "
#define COMPILE_INCLUDE(path) string("-I ") + PATH_FORMAT(path) + ' '
#define COMPILE_INCLUDE_DIRS COMPILE_INCLUDE(PathConcat(EXTERNAL_DIR, SWAN_DIR)) + COMPILE_INCLUDE(PathConcat(EXTERNAL_DIR, EXTLIB_DIR)) + COMPILE_INCLUDE(PathConcat(EXTERNAL_DIR, NK_DIR)) + COMPILE_INCLUDE(PathConcat(EXTERNAL_DIR, LIBRPM_DIR)) + COMPILE_INCLUDE(HEADER_DIR) + COMPILE_INCLUDE(GLOBAL_DIR)
#define COMPILE_OUTPUT(path) string("-o ") + PATH_FORMAT(path) + ' '
#define COMPILE_FLAGS "-r -mthumb -march=armv5t -Os -mlong-calls"
#define COMPILE_EXTENSION(compileType) ((compileType == CPP) ? ".o" : "ARM.o")

#define MERGER string("\"") + ARM_NONE_EABI_DIR + "arm-none-eabi-ld" + "\" "
#define MERGE_FLAGS "-r "
#define MERGE_OUTPUT(path) string("-o ") + PATH_FORMAT(path) + ' '

#define LINKER string("\"") + JAVA_DIR + "java\" -cp \"" + CTRMAP_DIR + "CTRMap.jar\" rpm.cli.RPMTool "
#define LINK_INPUT(path) string("-i ") + PATH_FORMAT(path) + ' '
#define LINK_EXTENSION ".dll"
#define LINK_OUTPUT(path) string("-o ") + PATH_FORMAT(path) + ' '
#define LINK_ESDB string("--esdb ") + PATH_FORMAT(ESDB_DIR) + ' '
#define LINK_FLAGS "--fourcc DLXF --generate-relocations"

#define BUILD_SETTINGS_FILE "buildSettings.txt"
#define BUILD_SETTINGS_PATH PathConcat(BUILDER_DIR, BUILD_SETTINGS_FILE)
#define BUILD_SETTINGS_MAX_SIZE 8192
#define BUILD_SETTINGS_PROJECT_DIR "SET PROJECT_DIR="
#define BUILD_INSTALL_LOG_PATH PathConcat(BUILDER_DIR, "install.log")

#define SETTINGS_FILE PathConcat(mainDir, "settings.h")
#define SETTINGS_WHITELIST "whitelist.txt"
#define SETTINGS_LIBRARIES PathConcat(LIB_DIR, SETTINGS_WHITELIST)
#define SETTINGS_ASSETS PathConcat(ASSETS_DIR, SETTINGS_WHITELIST)
#define SETTINGS_PRECOMP_FLAGS "-x c -E "
#define SETTINGS_INCLUDE(path) string("-include ") + PATH_FORMAT(path) + ' '
#define SETTINGS_EXTENSION ".data"
#define SETTINGS_GROUP_KEYWORD "<...>"

#define INSTALL_LOG_SEPARATOR '<'

enum CompileType
{
	DONT_COMPILE = 0,
	CPP = 1,
	ARM = 2,
};
struct CompileObject
{
	string name = string();
	CompileType type = DONT_COMPILE;

	string input = string();
	string folder = string();
};
struct CompileStructure
{
	string name = string();
	vector<CompileObject> objects;
};

struct OutputObject
{
	string name = string();
	string path = string();

	bool upToDate = false;

	string folder = string();
};
#define OUTPUT_INVALID(obj) obj.path.empty()

string LowerCase(const string& input)
{
	string output;
	for (size_t idx = 0; idx < input.size(); ++idx)
		output.push_back(tolower(input[idx]));
	return output;
}

void PathCorrectSeparator(string& path)
{
	for (size_t idx = 0; idx < path.length(); ++idx)
		if (path[idx] == NOT_SEPARATOR)
			path[idx] = SEPARATOR;
}
string PathGetExtension(const string& path)
{
	size_t start = path.find_last_of('.');
	if (start == string::npos)
		return string();

	++start;
	return path.substr(start, path.length() - start);
}
string PathRemoveExtension(const string& path)
{
	string extension = PathGetExtension(path);
	if (extension.empty())
		return path;

	return path.substr(0, path.length() - extension.length() - 1);
}
string PathGetLast(const string& path)
{
	size_t lastSlash = path.find_last_of(SEPARATOR) + 1;
	if (lastSlash == string::npos)
		return path;
	return path.substr(lastSlash);
}
string PathRemoveLast(const string& path, bool removeSeparator = false)
{
	size_t lastSlash = path.find_last_of(SEPARATOR) + 1;
	if (lastSlash == string::npos)
		return string();
	if (removeSeparator)
		lastSlash -= 1;
	return path.substr(0, lastSlash);
}
string PathGetLastName(const string& path)
{
	string file = PathGetLast(path);
	if (file.empty())
		return string();

	return PathRemoveExtension(file);
}
string PathConcat(const string& path, const string& concat) 
{ 
	if (path.empty())
		return concat;
	if (concat.empty())
		return path;
	return path + SEPARATOR + concat; 
}
bool PathRemove(const string& path)
{
	if (!filesystem::exists(path))
		return true;

	std::error_code ec;
	if (!filesystem::remove_all(path, ec))
	{
		printf_s("Error removing path %s (%s)\n", path.c_str(), ec.message().c_str());
		return false;
	}
	return true;
}

bool FileIsUptoDate(const string& input, const string& output)
{
	if (!filesystem::exists(input) ||
		!filesystem::exists(output))
		return false;

	filesystem::file_time_type inTime = filesystem::last_write_time(input);
	filesystem::file_time_type outTime = filesystem::last_write_time(output);
	if (outTime > inTime)
		return true;
	return false;
}
bool FileCompare(const string& input, const string& output)
{
	std::error_code ec;
	size_t size = filesystem::file_size(input, ec);
	if (size == 0 || size != filesystem::file_size(output, ec))
		return false;

	FILE* inputFile = nullptr;
	fopen_s(&inputFile, input.c_str(), "rb");
	if (!inputFile)
		return false;

	char* data = new char[size * 2];
	if (fread_s(data, size, sizeof(char), size, inputFile) != size)
	{
		delete[] data;
		fclose(inputFile);
		return false;
	}
	fclose(inputFile);

	FILE* outputFile = nullptr;
	fopen_s(&outputFile, output.c_str(), "rb");
	if (!outputFile)
	{
		delete[] data;
		return false;
	}
	if (fread_s(data + size, size, sizeof(char), size, outputFile) != size)
	{
		delete[] data;
		fclose(outputFile);
		return false;
	}
	fclose(outputFile);

	for (size_t idx = 0; idx < size; ++idx)
	{
		if (data[idx] != data[idx + size])
		{
			delete[] data;
			return false;
		}
	}

	delete[] data;
	return true;
}

vector<string> FolderGetList(const string& path)
{
	vector<string> folderList;
	if (!filesystem::is_directory(path))
		return folderList;

	for (const filesystem::directory_entry& entry : filesystem::directory_iterator(path))
	{
		string path = entry.path().string();
		if (!path.empty())
		{
			PathCorrectSeparator(path);
			folderList.push_back(path);
		}
	}

	return folderList;
}
bool FolderCreate(const string& path)
{
	if (filesystem::exists(path))
		return true;

	std::error_code ec;
	if (!filesystem::create_directories(path, ec))
	{
		printf_s("Error creating directory %s (%s)\n", path.c_str(), ec.message().c_str());
		return false;
	}
	return true;
}

string AddScriptSettings(const string& path, string& buildScript)
{
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "r");
	if (!file)
	{
		printf_s("Couldn't open %s\n", path.c_str());
		printf_s("	Error opening build settings file\n");
		return string();
	}

	char buffer[BUILD_SETTINGS_MAX_SIZE];
	memset(buffer, 0, BUILD_SETTINGS_MAX_SIZE);

	size_t read = fread(buffer, sizeof(char), BUILD_SETTINGS_MAX_SIZE, file);
	if (!read)
	{
		printf_s("Couldn't read %s\n", path.c_str());
		printf_s("	Error reading build settings file\n");
		fclose(file);
		return string();
	}

	for (size_t i = 0; i < read; ++i)
		buildScript += buffer[i];
	buildScript += '\n';
	fclose(file);

	size_t start = buildScript.find(BUILD_SETTINGS_PROJECT_DIR);
	if (start == string::npos)
	{
		printf_s("Couldn't read %s\n", path.c_str());
		printf_s("	Project Directory variable missing!\n");
		printf_s("	%sAdd the path to your CTRMap project HERE\n", BUILD_SETTINGS_PROJECT_DIR);
		return string();
	}
	start += sizeof(BUILD_SETTINGS_PROJECT_DIR) - 1;
	size_t end = buildScript.find('\n', start);

	return buildScript.substr(start, end - start);
}
bool CreateScript(const string& path, const string& buildScript)
{
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "w");
	if (!file)
	{
		printf_s("Couldn't create script %s\n", path.c_str());
		printf_s("	Error opening file\n");
		return false;
	}

	if (fwrite(buildScript.c_str(), sizeof(char), buildScript.size(), file) != buildScript.size())
	{
		printf_s("Couldn't create script %s\n", path.c_str());
		printf_s("	Error writing file\n");
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}

string ProcompileDataFile(const string& path)
{
	string buildScript = "@ECHO OFF\n";
	string projectDir = AddScriptSettings(BUILD_SETTINGS_PATH, buildScript);
	if (projectDir.empty())
		return string();

	string command = COMPILER;
	command += SETTINGS_PRECOMP_FLAGS;
	command += SETTINGS_INCLUDE(SETTINGS_FILE);
	command += PATH_FORMAT(path) + ' ';

	string name = PathGetLastName(PathRemoveLast(path, true));
	string output = PathConcat(BUILD_DIR, name) + SETTINGS_EXTENSION;
	command += COMPILE_OUTPUT(output);

	buildScript += command + '\n';

	string script = SCRIPT_PATH(name);
	if (!CreateScript(script, buildScript))
		return string();
	system((PATH_FORMAT(script)).c_str());

	return output;
}
void CleanLine(string& line)
{
	size_t start = line.find_first_not_of(NULL_CHARS);
	if (start == string::npos)
		start = 0;
	size_t end = line.find_last_not_of(NULL_CHARS);
	if (end == string::npos)
		end = line.size();
	else
		++end;

	line = line.substr(start, end - start);
	int i = 0;
}
bool LoadDataFile(const string& path, vector<string>& data)
{
	string precompiled = ProcompileDataFile(path);
	if (precompiled.empty())
		return false;
		

	FILE* file = nullptr;
	fopen_s(&file, precompiled.c_str(), "r");
	if (!file)
	{
		printf_s("Couldn't open %s\n", path.c_str());
		printf_s("	Error opening whitelist file\n");
		return false;
	}

	char lineData[BUILD_SETTINGS_MAX_SIZE];
	fgets(lineData, BUILD_SETTINGS_MAX_SIZE, file);
	while (!feof(file))
	{
		if (lineData[0] != '#')
		{
			string line(lineData);
			CleanLine(line);
			if (!line.empty())
			{
				size_t groupIdx = line.find(SETTINGS_GROUP_KEYWORD);
				if (groupIdx != string::npos)
				{
					string startPath = line.substr(0, groupIdx);
					string endPath = line.substr(groupIdx + strlen(SETTINGS_GROUP_KEYWORD));
					CleanLine(startPath);
					CleanLine(endPath);

					string groupPath = PathRemoveLast(startPath);
					if (groupPath != PathRemoveLast(endPath))
					{
						printf("ERROR: Failed to load group!\n");
						printf("    Start and end are in different directories\n");
					}
					else
					{
						startPath = PathGetLastName(startPath);
						endPath = PathGetLastName(endPath);
						int start = stoi(startPath);
						int end = stoi(endPath);

						for (start; start <= end; ++start)
							data.push_back(groupPath + to_string(start));
					}
				}
				else
					data.push_back(line);
			}
		}
		fgets(lineData, BUILD_SETTINGS_MAX_SIZE, file);
	}

	fclose(file);
	return true;
}
bool FindData(bool condition, const string& dataName, vector<string> data)
{
	if (dataName == SETTINGS_WHITELIST)
		return false;
	if (!condition)
		return true;

	for (size_t asset = 0; asset < data.size(); ++asset)
		if (dataName == data[asset])
			return true;
	return false;
}

void LoadInstallLog(const string& path)
{
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "r");
	if (!file)
		return;

	char lineData[BUILD_SETTINGS_MAX_SIZE];
	fgets(lineData, BUILD_SETTINGS_MAX_SIZE, file);
	while (!feof(file))
	{
		string line = string(lineData);
		size_t separatorIdx = line.find(INSTALL_LOG_SEPARATOR);

		string destination = line;
		string source = string();
		if (separatorIdx != string::npos)
		{
			destination = line.substr(0, separatorIdx);
			source = line.substr(separatorIdx + 1);
			CleanLine(source);
		}
		CleanLine(destination);

		installLog.push_back(make_pair(destination, source));

		fgets(lineData, BUILD_SETTINGS_MAX_SIZE, file);
	}

	fclose(file);
}
void SaveInstallLog(const string& path)
{
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "w");
	if (!file)
	{
		printf_s("Couldn't save install log %s\n", path.c_str());
		printf_s("	Error opening file\n");
		return;
	}

	for (size_t log = 0; log < installLog.size(); ++log)
	{
		fwrite(installLog[log].first.c_str(), sizeof(char), installLog[log].first.length(), file);
		if (!installLog[log].second.empty())
		{
			fputc(INSTALL_LOG_SEPARATOR, file);
			fwrite(installLog[log].second.c_str(), sizeof(char), installLog[log].second.length(), file);
		}
		fputc('\n', file);
	}
	fclose(file);
}
void AddInstallLog(const string& installPath, const string& sourcePath)
{
	for (size_t log = 0; log < installLog.size(); ++log)
		if (installLog[log].first == installPath)
			return;

	installLog.push_back(make_pair(installPath, sourcePath));
}

CompileType GetCompileType(const string& path)
{
	string extension = LowerCase(PathGetExtension(path));
	if (extension == "cpp" || extension == "c")
		return CPP;
	if (extension == "s")
		return ARM;
	return DONT_COMPILE;
}
bool GetFolderCompilationData(const string& path, CompileStructure& data)
{
	if (!filesystem::exists(path))
	{
		printf_s("Couldn't Compile %s\n", path.c_str());
		printf_s("    Folder doesn't exist\n");
		return false;
	}

	if (!filesystem::is_directory(path))
	{
		printf_s("Couldn't Compile %s\n", path.c_str());
		printf_s("    Path is not a folder\n");
		return false;
	}

	vector<string> directoryEntries = FolderGetList(path);
	if (!directoryEntries.size())
	{
		printf_s("No compilable data found in %s!\n", path.c_str());
		return false;
	}
	
	for (size_t entryIdx = 0; entryIdx < directoryEntries.size(); ++entryIdx)
	{
		const string& entry = directoryEntries[entryIdx];
		if (filesystem::is_directory(entry))
			continue;

		CompileType compileType = GetCompileType(entry);
		if (compileType == DONT_COMPILE)
			continue;

		string fileName = PathGetLastName(entry);
		if (fileName.empty())
			continue;

		data.objects.push_back({ fileName , compileType, entry });
	}

	if (data.objects.empty())
	{
		printf_s("Couldn't Compile %s\n", path.c_str());
		printf_s("    No compilable files in folder\n");

		data = CompileStructure();
		return false;
	}

	data.name = PathGetLast(path);
	if (data.name.empty())
	{
		printf_s("Couldn't Compile %s\n", path.c_str());
		printf_s("    Could not resolve the output name\n");

		data = CompileStructure();
		return false;
	}

	return true;
}
bool GetFileCompilationData(const string& path, CompileObject& object)
{
	if (filesystem::is_directory(path))
	{
		printf_s("Couldn't Compile %s\n", path.c_str());
		printf_s("	Path is not a file\n");
		return false;
	}

	CompileType compileType = GetCompileType(path);
	if (compileType == DONT_COMPILE)
		return false;
	object.type = compileType;
	object.input = path;

	object.name = PathGetLastName(path);
	if (object.name.empty())
	{
		printf_s("Couldn't Compile %s\n", path.c_str());
		printf_s("	Couldn't resolve name\n");

		object = CompileObject();
		return false;
	}

	return true;
}

bool GetPatchesCompilationData(const string& path, vector<CompileStructure>& patches)
{
	vector<string> patchNames = FolderGetList(path);
	if (patchNames.empty())
	{
		printf_s("No patches to compile\n");
		return false;
	}

	patches.reserve(patchNames.size());
	for (size_t patchIdx = 0; patchIdx < patchNames.size(); ++patchIdx)
	{
		CompileStructure patch;
		if (!GetFolderCompilationData(patchNames[patchIdx], patch))
			continue;

		patches.push_back(patch);
	}
	if (patches.empty())
	{
		printf_s("No compilable data was found in any patch\n");
		return false;
	}

	return true;
}
void GetLibrariesCompilationData(const string& directory, vector<CompileObject>& libraries, const string& folder = string())
{
	string path = PathConcat(directory, folder);

	vector<string> entries = FolderGetList(path);
	if (!entries.empty())
	{
		for (size_t entryIdx = 0; entryIdx < entries.size(); ++entryIdx)
		{
			const string& entry = entries[entryIdx];
			if (filesystem::is_directory(entry))
			{
				string assetFolder = entry.substr(directory.length() + 1);
				if (!FindData(whitelistLibs, assetFolder, libraryData))
					continue;

				GetLibrariesCompilationData(directory, libraries, assetFolder);
				continue;
			}

			CompileObject lib;
			if (!GetFileCompilationData(entry, lib))
				continue;

			lib.folder = folder;
			libraries.push_back(lib);
		}
	}
}

OutputObject CompileCompObject(const string& path, const CompileObject& object, string& buildScript)
{
	string output = PathConcat(path, object.name) +
		COMPILE_EXTENSION(object.type);

	bool upToDate = FileIsUptoDate(object.input, output);
	if (!upToDate)
	{
		buildScript += ECHO(object.name + " is compiling...");

		string command = COMPILER;
		command += PATH_FORMAT(object.input) + ' ';

		switch (object.type)
		{
		case CPP:
			command += COMPILE_INCLUDE_DIRS;
			break;
		case ARM:
			break;
		}
		command += COMPILE_INCLUDE(mainDir);
		command += COMPILE_OUTPUT(output);
		command += COMPILE_FLAGS;

		buildScript += command + '\n';
	}
	else
		buildScript += ECHO_TAB(object.name + " is up to date");

	return { object.name, output, upToDate, object.folder };
}
OutputObject CompileCompStruct(const string& path, const CompileStructure& structure, string& buildScript, OutputObject* externMerge = nullptr)
{
	if (externMerge && externMerge->path.empty())
		externMerge = nullptr;

	buildScript += ECHO_SUBTITLE(structure.name + " is compiling...");

	vector<OutputObject> compiled;
	unsigned int upToDate = 0;
	for (size_t objIdx = 0; objIdx < structure.objects.size(); ++objIdx)
	{
		OutputObject output = CompileCompObject(path, structure.objects[objIdx], buildScript);
		if (OUTPUT_INVALID(output))
			continue;
		if (output.upToDate)
			++upToDate;
		compiled.push_back(output);
	}
	if (compiled.empty())
	{
		printf_s("%s has no compilable objects\n", structure.name.c_str());
		return OutputObject();
	}

	// If there is only 1 compilable object return the only output
	if (compiled.size() == 1 && !externMerge)
		return compiled[0]; 

	// If all the compilable object to be merged are up to date return an up to date output
	string output = PathConcat(path, structure.name) + COMPILE_EXTENSION(CPP);
	if (compiled.size() == upToDate)
		if (!externMerge || externMerge->upToDate)
			return { structure.name, output, true };

	string command = MERGER;
	command += MERGE_FLAGS;
	command += MERGE_OUTPUT(output);
	for (size_t compIdx = 0; compIdx < compiled.size(); ++compIdx)
		command += PATH_FORMAT(compiled[compIdx].path) + ' ';
	if (externMerge)
		command += PATH_FORMAT(externMerge->path);

	buildScript += command + '\n';
	return { structure.name, output, false };
}

void LinkCompOutputs(const string& path, const vector<OutputObject>& outputs, string& buildScript)
{
	for (size_t outIdx = 0; outIdx < outputs.size(); ++outIdx)
	{
		string fullPath = path;
		if (!outputs[outIdx].folder.empty())
		{
			fullPath = PathConcat(path, outputs[outIdx].folder);
			FolderCreate(fullPath);
		}

		string output = PathConcat(fullPath, outputs[outIdx].name) + LINK_EXTENSION;
		bool upToDate = FileIsUptoDate(outputs[outIdx].path, output) && outputs[outIdx].upToDate;
		if (!upToDate)
		{
			buildScript += ECHO_SUBTITLE(outputs[outIdx].name + " is linking...");
			string command = LINKER;
			command += LINK_INPUT(outputs[outIdx].path);
			command += LINK_OUTPUT(output);
			command += LINK_ESDB;
			command += LINK_FLAGS;

			buildScript += command + '\n';

			AddInstallLog(output, string());
		}
	}
}

bool BuildPatches(const string& path, const vector<CompileStructure>& patches, const CompileStructure& global, string& buildScript)
{
	buildScript += ECHO_TITLE("Building Patches...");

	OutputObject globalOut;
	if (!global.objects.empty())
		globalOut = CompileCompStruct(BUILD_DIR, global, buildScript, nullptr);
	else
		printf_s("No globals to compile!\n");

	vector<OutputObject> patchesOut;
	unsigned int upToDate = 0;
	for (size_t patchIdx = 0; patchIdx < patches.size(); ++patchIdx)
	{
		OutputObject output = CompileCompStruct(BUILD_DIR, patches[patchIdx], buildScript, &globalOut);
		if (OUTPUT_INVALID(output))
			continue;
		if (output.upToDate)
			++upToDate;
		patchesOut.push_back(output);
	}
	if (patchesOut.empty())
	{
		printf_s("No compilable data found in any patch!\n");
		return false;
	}
	if (patchesOut.size() == upToDate)
		buildScript += ECHO_SUBTITLE("All patches are up to date!");

	LinkCompOutputs(path, patchesOut, buildScript);

	return true;
}
bool BuildLibraries(const string& path, const vector<CompileObject>& libraries, string& buildScript)
{
	if (libraries.empty())
	{
		printf_s("No libraries to build!\n");
		return true;
	}
		
	buildScript += ECHO_TITLE("Building Libraries...");

	vector<OutputObject> libsOut;
	unsigned int upToDate = 0;
	for (size_t libIdx = 0; libIdx < libraries.size(); ++libIdx)
	{
		OutputObject output = CompileCompObject(BUILD_DIR, libraries[libIdx], buildScript);
		if (OUTPUT_INVALID(output))
			continue;
		if (output.upToDate)
			++upToDate;

		libsOut.push_back(output);
	}
	if (libsOut.empty())
	{
		printf_s("No compilable data found in any library!\n");
		return false;
	}
	if (libsOut.size() == upToDate)
		buildScript += ECHO_SUBTITLE("All libraries are up to date!");

	LinkCompOutputs(path, libsOut, buildScript);

	return true;
}

enum AssetsHandling
{
	ASK = 0,
	OVERRIDE = 1,
	KEEP = 2,
};
AssetsHandling assetsHandling = ASK;
void MoveData(const string& directory, const string& output, const string& folder = string())
{
	string path = PathConcat(directory, folder);

	vector<string> entries = FolderGetList(path);
	if (!entries.empty())
	{
		for (size_t entryIdx = 0; entryIdx < entries.size(); ++entryIdx)
		{
			const string& entry = entries[entryIdx];
			string assetFolder = entry.substr(directory.length() + 1);

			if (filesystem::is_directory(entry))
			{
				MoveData(directory, output, assetFolder);
				continue;
			}

			if (!FindData(whitelistAssets, assetFolder, assetsData))
				continue;

			string outputPath = PathConcat(output, folder);
			if (!FolderCreate(outputPath))
				continue;
			
			string outputFile = PathConcat(output, assetFolder);
			if (filesystem::exists(outputFile))
			{
				if (FileCompare(entry, outputFile))
					continue;
				else
				{
					switch (assetsHandling)
					{
					case ASK:
					{
						printf_s("The file %s has been modified, do you want to override it?\n", outputFile.c_str());
						printf_s("You select a default setting by using the \"-assets-override\" or \"-assets-keep\" options\n");
						printf_s("(Yes: Y) (No: N)\n");
						char response = getchar();
						if (response != 'y' && response != 'Y')
						{
							printf_s("    The file %s was kept\n", outputFile.c_str());
							continue;
						}
						break;
					}
					case OVERRIDE:
						printf_s("The file %s was overriden\n", outputFile.c_str());
						break;
					case KEEP:
						printf_s("The file %s was kept\n", outputFile.c_str());
						continue;
					}
				}
			}

			std::error_code ec;
			if (!filesystem::copy_file(entry, outputFile, filesystem::copy_options::overwrite_existing, ec))
			{
				printf_s("Error copying file %s to %s (%s)\n", entry.c_str(), outputFile.c_str(), ec.message().c_str());
				continue;
			}

			printf("Added file: %s \n", outputFile.c_str());
			AddInstallLog(outputFile, entry);
		}
	}
}

bool CheckIfInstalled()
{
	if (customBuild)
		return true;
	if (!filesystem::exists(BUILDER_DIR))
		return false;
	return true;
}

bool Install()
{
	if (!filesystem::exists(BUILD_SETTINGS_FILE))
	{
		printf_s("Couldn't install, the default build settings file is missing!");
		return false;
	}
	if (!filesystem::exists(SETTINGS_FILE))
	{
		printf_s("Couldn't install, the build project doesn't have a settings.h file!");
		return false;
	}
	if (!FolderCreate(BUILDER_DIR))
		return false;

	std::error_code ec;
	if (!filesystem::copy_file(BUILD_SETTINGS_FILE, BUILD_SETTINGS_PATH, filesystem::copy_options::overwrite_existing, ec))
	{
		printf_s("Couldn't install!");
		printf_s("    Error copying file %s to %s (%s)\n", BUILD_SETTINGS_FILE, BUILD_SETTINGS_PATH.c_str(), ec.message().c_str());
		return false;
	}

	printf_s("Installation complete!");
	return true;
}
bool Build() 
{
	if (!CheckIfInstalled())
	{
		printf_s("Couldn't build, it's not installed!");
		printf_s("    Use the -install command to correctly setup a build");
		return false;
	}

	LoadInstallLog(BUILD_INSTALL_LOG_PATH);

	if (!FolderCreate(BUILD_DIR))
		return false;

	printf_s("--- LOADING DATA ---\n");
	if (whitelistLibs && !LoadDataFile(SETTINGS_LIBRARIES, libraryData))
		return false;
	if (whitelistAssets && !LoadDataFile(SETTINGS_ASSETS, assetsData))
		return false;

	printf_s("--- COMPILE DATA ---\n");
	printf_s("- GLOBALS -\n");
	CompileStructure global;
	GetFolderCompilationData(GLOBAL_DIR, global);

	printf_s("- PATCHES -\n");
	vector<CompileStructure> patches;
	if (!GetPatchesCompilationData(PATCH_DIR, patches))
		return false;

	printf_s("- LIBRARIES -\n");
	vector<CompileObject> libraries;
	GetLibrariesCompilationData(LIB_DIR, libraries);

	printf_s("--- BUILD SCRIPT ---\n");
	string buildScript = "@ECHO OFF\n";
	string projectDir = AddScriptSettings(BUILD_SETTINGS_PATH, buildScript);
	if (projectDir.empty())
		return false;

	printf_s("- PATCHES -\n");
	if (!BuildPatches(PROJECT_PATCH_DIR(projectDir), patches, global, buildScript))
		return false;
	printf_s("- LIBRARIES -\n");
	if (!BuildLibraries(PROJECT_LIB_DIR(projectDir), libraries, buildScript))
		return false;

	printf_s("--- CREATING SCRIPT ---\n");
	string script = SCRIPT_PATH("build");
	if (!CreateScript(script, buildScript))
		return false;

	printf_s("--- MOVING DATA ---\n");
	MoveData(ASSETS_DIR, PROJECT_ASSETS_DIR(projectDir));

	SaveInstallLog(BUILD_INSTALL_LOG_PATH);

	printf_s("--- BUILD PROCESS ---\n");
	system((PATH_FORMAT(script)).c_str());
	return true;
}
bool Clear(bool log = false)
{
	if (!CheckIfInstalled())
	{
		printf_s("Couldn't clear, it's not installed!");
		printf_s("    Use the -install command to correctly setup a build");
		return false;
	}

	if (!PathRemove(BUILD_DIR))
		return false;

	if (log)
		printf_s("Cleared build data!\n");
	return true;
}
bool Uninstall()
{
	if (!CheckIfInstalled())
	{
		printf_s("Couldn't uninstall, it's not installed!");
		printf_s("    Use the -install command to correctly setup a build");
		return false;
	}

	LoadInstallLog(BUILD_INSTALL_LOG_PATH);

	for (size_t log = 0; log < installLog.size(); ++log)
	{
		const string& installedFile = installLog[log].first;
		if (filesystem::exists(installedFile))
		{
			if (!installLog[log].second.empty())
			{
				if (!FileCompare(installLog[log].second, installedFile))
				{
					printf_s("The file %s has been modified, do you want to remove it?\n", installedFile.c_str());
					printf_s("(Yes: Y) (No: N)\n");
					char response[64];
					scanf_s("%s", response, 64);
					if (response[0] != 'y' && response[0] != 'Y')
					{
						printf_s("Kept file: %s\n", installedFile.c_str());
						continue;
					}
				}
			}

			if (PathRemove(installedFile))
				printf("Removed file: %s\n", installedFile.c_str());
		}
	}
		
	installLog.clear();
	PathRemove(BUILD_INSTALL_LOG_PATH);

	if (!Clear())
		return false;

	if (!keepSettings)
		PathRemove(BUILDER_DIR);

	printf("Uninstallation complete!\n");
	return true;
}
bool Rebuild()
{
	keepSettings = true;
	if (!Uninstall())
		return false;
	if (!customBuild && !Install())
		return false;
	if (!Build())
		return false;
	return true;
}
void Help()
{
	printf_s("-install \"Patch Dir\" -> set up a build project\n");
	printf_s("-build \"Patch Dir\" -> build only the modified files in the patch\n");
	printf_s("-rebuild \"Patch Dir\" -> build the patch from scratch\n");
	printf_s("    -whitelist-libs -> ignore any folder in \"Libraries\" that is not specified in \"Libraries\\whitelist.txt\"\n");
	printf_s("    -whitelist-assets -> ignore any file in \"Assets\" that is not specified in \"Assets\\whitelist.txt\"\n");
	printf_s("    -whitelist-all -> activate all whitelist functionalities\n");
	printf_s("    -assets-override -> when a conflict appears when moving assets, the project asset gets automatically overriden\n");
	printf_s("    -assets-keep -> when a conflict appears when moving assets, the asset is not moved keeping the project asset\n");
	printf_s("-clear \"Patch Dir\" -> clear all build data (deletes \"build\" folder)\n");
	printf_s("-uninstall \"Patch Dir\" -> remove the patch completely from the CTRMap project\n");
	printf_s("    -keep-settings -> don't delete the build settings file when uninstalling\n");
	printf_s("\n-custom-build \"Custom Build Path\" -> can be added to any command to specify a non-default Builder folder where the build settings are stored\n");
	printf_s("\n-pause -> can be added to any command to pause the program before exiting\n");
}

#define INSTALL_COMMAND "-install"
#define BUILD_COMMAND "-build"
#define REBUILD_COMMAND "-rebuild"
	#define WHITELIST_ALL_COMMAND "-whitelist-all"
	#define WHITELIST_LIBS_COMMAND "-whitelist-libs"
	#define WHITELIST_ASSETS_COMMAND "-whitelist-assets"
	#define ASSETS_OVERRIDE_COMMAND "-assets-override"
	#define ASSETS_KEEP_COMMAND "-assets-keep"
#define CLEAR_COMMAND "-clear"
#define UNINSTALL_COMMAND "-uninstall"
	#define KEEP_SETTINGS_COMMAND "-keep-settings"
#define CUSTOM_BUILD_COMMAND "-custom-build"
#define PAUSE_COMMAND "-pause"

#define RETURN if (pause) { printf("\nPress Enter to continue...\n"); getchar(); } return 0
int main(int argc, char* argv[])
{
#if _DEBUG
	mainDir = "..\\Code\\PW2Code";
	builderDir = "..\\Editor\\Projects\\PW2";
	customBuild = (mainDir != builderDir);
	whitelistLibs = true;
	whitelistAssets = true;
	keepSettings = true;
	assetsHandling = ASK;
	Build();
#else
	if (argc > 1)
	{
		for (int arg = 2; arg < argc; ++arg)
		{
			if (strcmp(argv[arg], WHITELIST_ALL_COMMAND) == 0)
			{
				whitelistLibs = true;
				whitelistAssets = true;
			}
			else if (strcmp(argv[arg], WHITELIST_LIBS_COMMAND) == 0)
				whitelistLibs = true;
			else if (strcmp(argv[arg], WHITELIST_ASSETS_COMMAND) == 0)
				whitelistAssets = true;
			else if (strcmp(argv[arg], KEEP_SETTINGS_COMMAND) == 0)
				keepSettings = true;
			else if (strcmp(argv[arg], ASSETS_OVERRIDE_COMMAND) == 0)
				assetsHandling = OVERRIDE;
			else if (strcmp(argv[arg], ASSETS_KEEP_COMMAND) == 0)
				assetsHandling = KEEP;
			else if (strcmp(argv[arg], CUSTOM_BUILD_COMMAND) == 0)
			{
				++arg;
				if (arg >= argc || argv[arg][0] == '-')
				{
					printf_s("You need to specify a directory with the build settings for the custom build!\n");
					RETURN;
				}
				builderDir = argv[arg];
			}
			else if (strcmp(argv[arg], PAUSE_COMMAND) == 0)
				pause = true;
			else
			{
				mainDir = argv[arg];
				CleanLine(mainDir);
				PathCorrectSeparator(mainDir);
			}
		}

		if (mainDir.empty())
		{
			printf_s("You need to specify a directory to build!\n");
			RETURN;
		}

		if (builderDir.empty())
			builderDir = mainDir;
		else
			customBuild = (mainDir != builderDir);
			
		if (strcmp(argv[1], INSTALL_COMMAND) == 0)
			Install();
		else if (strcmp(argv[1], BUILD_COMMAND) == 0)
			Build();
		else if (strcmp(argv[1], REBUILD_COMMAND) == 0)
			Rebuild();
		else if (strcmp(argv[1], CLEAR_COMMAND) == 0)
			Clear(true);
		else if (strcmp(argv[1], UNINSTALL_COMMAND) == 0)
			Uninstall();
		else
			Help();
	}
	else
		Help();
#endif

	RETURN;
}