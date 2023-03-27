#include "brainfuck.hpp"

void show_version()
{
    std::cout << "BrainFuck version 0.4\nCopyright (c) 2023 Mahmood Jamshidian (MJScript)\nMIT License\n\nWritten by Mahmood Jamshidian (MJScript)\n\nread more at https://github.com/MahmoodJamshidian/BrainFuck\n";
}

void show_help(char const *argv[])
{
    std::string app_name = argv[0];
    app_name = app_name.substr(app_name.find_last_of("/") + 1);
    std::cout << "Usage: " << app_name << " <source> [options]\nOptions:\n  -h --help\t\tShow help\n  -v --version\t\tShow version\n  (The following options are related to the build)\n  -b --build\t\tBuild executable file\n  -o <output file>\tThe output path of the executable file (source path by default)\n  -c <c++ compiler>\tSet c++ compiler (g++ by default)\n  --option <option>\tSet c++ compiler options" << std::endl;
}

int main(int argc, char const *argv[])
{
    __tb.throw_exc = false;
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
    if (strcmp(ext, "bf") != 0 && strcmp(ext, "b") != 0)
    {
        __tb.raise(Exception, "invalid file extension");
        return Exception;
    }
    bool is_build = false;
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
    std::ifstream file;
    file.open(filename, std::ifstream::in | std::ifstream::ate);

    if(file.fail()){
        __tb.raise(IOError, string_format("can't open '%s': %s", filename, strerror(errno)));
        return IOError;
    }
    char *code = (char *)malloc(file.tellg() * sizeof(char));
    file.seekg(0);
    char c;
    size_t index = 0;
    while (true)
    {
        file.read(&c, 1);
        if(!file.eof()){
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
    for(int _ = 0; _ < INITIAL_REGISTRY_ARGS; _++)
        reg.nreg.push_back(0);
    Structure main_struct((const char *)code);
    Environment program({{0, strlen(code), &main_struct, false, main_struct.tree}});
    if (!is_build)
        program.run();
    else
    {
        std::string __bsrc = program.build();
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        std::string bfile_addr = std::filesystem::temp_directory_path().string() + "bf-temp.cpp";
        std::ofstream bfile(bfile_addr.c_str());
        bool is_bfile_writable = (bool)bfile;
        bfile.write(__bsrc.c_str(), __bsrc.length());
        bfile.close();
        if (!is_bfile_writable)
        {
            __tb.raise(IOError, string_format("can't wtite build file in %s", bfile_addr));
            return IOError;
        }
        std::string cmd = compiler + " -Wall -I" + path::getExecutableDir() + "include -o " + output + options + ' ' + bfile_addr;
#else
        std::string cmd = compiler + " -Wall -I" + path::getExecutableDir() + "include -o " + output + options + " -xc++ - << EOF\n" + __bsrc + "\nEOF";
#endif
        bool is_worked = !system(cmd.c_str());
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        remove(bfile_addr.c_str());
#endif
        if (!is_worked)
        {
            __tb.raise(CompileError, "a compilation error occurred");
            return CompileError;
        }
    }
    return 0;
}