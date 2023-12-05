#include "libbfx.cpp"
#include <cstdlib>
#include "path.cpp"

#define VERSION "0.6"

fs::path exec_dir = path::getExecutableDir();
fs::path include_dir = exec_dir / "include";
fs::path plugins_dir = exec_dir / "plugins";
fs::path plugins_bin_dir = plugins_dir / "bin";
fs::path plugins_src_dir = plugins_dir / "src";
fs::path plugin_list_file_path = plugins_dir / "list.txt";
fs::path load_plugins_path = include_dir / "_load_plugins.hpp";

struct plugin_info
{
    std::string name;
    std::string bin;
    std::string src;

    bool overwrite = false;

    plugin_info(std::string plugin)
    {
        if (plugin[plugin.length() - 1] == '+')
        {
            plugin.pop_back();
            this->overwrite = true;
        }

        this->name = plugin;
        this->src = plugin + ".cpp";
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        this->bin = plugin + ".dll";
#else
        this->bin = plugin + ".so";
#endif
    }
};

void show_version()
{
    std::cout << "BrainFuck version " << VERSION << " \nCopyright (c) 2023 Mahmood Jamshidian (MJScript)\nMIT License\n\nWritten by Mahmood Jamshidian (MJScript)\n\nread more at https://github.com/MahmoodJamshidian/BrainFuck\n";
}

void show_help(char const *argv[])
{
    std::string app_name = argv[0];
    app_name = app_name.substr(app_name.find_last_of("/") + 1);
    std::cout << "Usage: " << app_name << " <source> [options]\nOptions:\n  -h --help\t\tShow help\n  -v --version\t\tShow version\n  (The following options are related to the build)\n  -b --build\t\tBuild executable file\n  -o <output file>\tThe output path of the executable file (source path by default)\n  -c <c++ compiler>\tSet c++ compiler (g++ by default)\n  --option <option>\tSet c++ compiler options\n  -np --no-plugin\tWithout using plugins" << std::endl;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        show_help(argv);
        return Exception;
    }
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
    {
        show_version();
        return 0;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        show_help(argv);
        return 0;
    }
    char const *filename = argv[1];
    const char *ext = std::string(filename).substr(std::string(filename).find_last_of(".") + 1).c_str();
    if (strcmp(ext, "bf") != 0 && strcmp(ext, "bfx") != 0)
    {
        __tb.raise(Exception, "invalid file extension");
        return Exception;
    }
    bool is_build = false;
    bool no_plugin = false;
    std::string output = "";
    std::string compiler = "";
    std::string options = "";

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-b") == 0)
        {
            if (is_build)
            {
                __tb.raise(Exception, string_format("repeat '%s' switch", argv[i]));
                return Exception;
            }
            is_build = true;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (output != "")
            {
                __tb.raise(Exception, string_format("repeated '%s' switch", argv[i]));
                return Exception;
            }
            if (++i<argc)
            {
                output = argv[i];
            }
            else
            {
                __tb.raise(Exception, string_format("'%s' switch was ignored", argv[i-1]));
                return Exception;
            }
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            if (compiler != "")
            {
                __tb.raise(Exception, string_format("repeated '%s' switch", argv[i]));
                return Exception;
            }
            if (++i<argc)
            {
                compiler = argv[i];
            }
            else
            {
                __tb.raise(Exception, string_format("'%s' switch was ignored", argv[i-1]));
                return Exception;
            }
        }
        else if (strcmp(argv[i], "--option") == 0)
        {
            if (++i<argc)
            {
                options += " ";
                options += argv[i];
            }
            else
            {
                __tb.raise(Exception, string_format("'%s' switch was ignored", argv[i-1]));
                return Exception;
            }
        }
        else if (strcmp(argv[i], "-np") == 0 || strcmp(argv[i], "--no-plugin") == 0)
        {
            if (no_plugin)
            {
                __tb.raise(Exception, string_format("repeated '%s' switch", argv[i]));
                return Exception;
            }

            no_plugin = true;
        }
        else
        {
            __tb.raise(Exception, string_format("invalid command '%s'", argv[i]));
            return Exception;
        }
    }
    if (options == "")
        options = " ";
    else if (!is_build)
    {
        __tb.raise(Exception, "switch '--option' in run mode");
        return Exception;
    }
    if (!is_build && output != "")
    {
        __tb.raise(Exception, "switch '-o' in run mode");
        return Exception;
    }
    if (is_build && output == "")
    {
        output = std::string(filename).substr(0, std::string(filename).find_last_of("."));
    }
    if (compiler == "")
        compiler = "g++";
    
    std::vector<plugin_info> plugins;
    
    if (!no_plugin)
    {
        std::ifstream plugin_list_file;
        plugin_list_file.open(plugin_list_file_path, std::ifstream::in);

        if(plugin_list_file.fail())
        {
            __tb.raise(IOError, string_format("can't open plugin list file ('%s'): %s", plugin_list_file_path.c_str(), strerror(errno)));
            return IOError;
        }

        std::string lin;

        while (plugin_list_file >> lin)
        {
            if (lin[0] != '#')
            {
                plugins.push_back(lin);
            }
        }

        plugin_list_file.close();

        for (uint64_t ind = 0; ind < plugins.size(); ind++)
        {
            load_plugin((plugins_bin_dir / plugins[ind].bin).string().c_str(), plugins[ind].overwrite);
        }
    }
    
    std::ifstream file;
    file.open(filename, std::ifstream::in | std::ifstream::ate);

    if(file.fail())
    {
        __tb.raise(IOError, string_format("can't open '%s': %s", filename, strerror(errno)));
        return IOError;
    }
    char *code = (char *)malloc(file.tellg() * sizeof(char) + 1);
    file.seekg(0);
    char c;
    size_t index = 0;
    while (true)
    {
        file.read(&c, 1);
        if(!file.eof())
        {
            if(c != '\r')
            {
                code[index] = c;
            }
            index++;
        }else{
            code[index] = '\0';
            break;
        }
    }
    file.close();

    Structure main_struct((const char *)code);
    Program program(&main_struct);
    if (!is_build)
        program.run();
    else
    {
        std::string __bsrc = program.build();

        fs::path build_file_path = fs::temp_directory_path() / string_format("bfx-%i.cpp", rand());

        std::ofstream build_file(build_file_path);
        
        if (build_file.fail())
        {
            __tb.raise(IOError, string_format("can't wtite build file in %s", build_file_path.c_str()));
        }

        build_file << __bsrc;
        build_file.close();

        std::ofstream load_plugins_file(load_plugins_path.c_str());

        if (!load_plugins_file)
        {
            remove(build_file_path);

            __tb.raise(IOError, string_format("can't wtite load plugins file in %s", load_plugins_path.c_str()));
        }

        std::string load_plugins_content;

        for (uint64_t ind = 0; ind < plugins.size(); ind++)
        {
            load_plugins_content += string_format("#include \"%s\"\n", plugins[ind].src.c_str());
        }

        load_plugins_file << load_plugins_content;
        load_plugins_file.close();
        
        if (system(string_format("%s -I\"%s\" -I\"%s\" -D BFX_BUILD -D BFX_LOAD_PLUGINS \"%s\" -o \"%s\" %s", compiler.c_str(), include_dir.string().c_str(), plugins_src_dir.string().c_str(), build_file_path.string().c_str(), output.c_str(), options.c_str())))
        {
            remove(build_file_path);
            remove(load_plugins_path);

            __tb.raise(CompileError, "a compilation error occurred");
            return CompileError;
        }

        remove(build_file_path);
        remove(load_plugins_path);
    }
    return 0;
}