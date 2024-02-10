#pragma once

#include "../std_helpers.h"

#include <util/generic/vector.h>
#include <filesystem>

struct TGeneratorArgs;

struct TLoggingOpts {
    bool EnableEvlog = false;
    bool EnableStderr = true;
    fs::path EvLogFilePath;
};

struct TOpts {
    fs::path ArcadiaRoot;
    fs::path ExportRoot;
    fs::path ConfigDir;
    std::string ProjectName;
    TVector<fs::path> SemGraphs;
    TVector<std::string> Platforms;
    std::string Generator;
    bool CleanIgnored = false;
    bool ReportIgnored = false;

    TLoggingOpts LoggingOpts;

    fs::path PyDepsDump;
    std::string PyVer = "py3";

    /// Output a list of generators.
    bool List = false;

    static TOpts Parse(int argc, char** argv);
};
