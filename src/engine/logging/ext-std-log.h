#include <iostream>

class ExtendedStdLog {
public:
enum class ExtLogLevel {
    Error,
    Info,
    Debug,
    Verbose,
    Network,
};

    static void SetLogLevel() {};
    template<typename... Args>
    static void LogStuff(ExtLogLevel level, Args&&... args) 
    {
        if (Enabled(level)) 
            (std::cout << ... << args) << '\n';
    };
private:
    static ExtLogLevel logLvl_;

    static bool Enabled(ExtLogLevel level)
    {
        return true;
    };
};
