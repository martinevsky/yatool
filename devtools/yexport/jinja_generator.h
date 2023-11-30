#pragma once

#include "generator_spec.h"
#include "sem_graph.h"
#include "spec_based_generator.h"
#include "yexport_generator.h"

#include <devtools/yexport/path_hash.h>
#include <devtools/ymake/dependency_management.h>

#include <contrib/libs/jinja2cpp/include/jinja2cpp/filesystem_handler.h>
#include <contrib/libs/jinja2cpp/include/jinja2cpp/template.h>
#include <contrib/libs/jinja2cpp/include/jinja2cpp/template_env.h>
#include <library/cpp/json/json_reader.h>

#include <util/generic/hash.h>
#include <util/generic/vector.h>

#include <filesystem>

namespace fs = std::filesystem;

struct TJinjaTarget {
    std::string Macro;
    std::string Name;
    bool isTest;
    TVector<std::string> MacroArgs;
    TMap<std::string, TVector<std::string>> Attributes;
    TMap<std::string, TVector<TNodeId>> LibExcludes;
};

struct TJinjaList {
    TVector<TJinjaTarget*> Targets;
};

using TSubdirsTable = THashMap<fs::path, TJinjaList>;
using TSubdirsTableElem = THashMap<fs::path, TJinjaList>::value_type;

class TJinjaGenerator : public TSpecBasedGenerator {
public:
    class TBuilder;

    static THolder<TJinjaGenerator> Load(const fs::path& arcadiaRoot, const std::string& generator, const fs::path& configDir = "");

    void SetProjectName(const std::string& name) override { ProjectName = name; }
    void LoadSemGraph(const std::string& platform, const fs::path& semGraph) override;
    void Render(const std::filesystem::path& exportRoot, ECleanIgnored cleanIgnored = ECleanIgnored::Disabled) override;

    void AnalizeSemGraph(const TVector<TNodeId>& startDirs, const TSemGraph& graph);
    THashMap<fs::path, TVector<TJinjaTarget>> GetSubdirsTargets() const;
    void SetSpec(const TGeneratorSpec& spec) { GeneratorSpec = spec; };
    const TNodeSemantics& ApplyReplacement(TPathView path, const TNodeSemantics& inputSem) const {
        return TargetReplacements_.ApplyReplacement(path, inputSem);
    }
private:

    void AddStrToParams(const std::string& attrMacro, const TVector<std::string>& values, jinja2::ValuesMap& params, const std::string& renderPath);
    void AddBoolToParams(const std::string& attrMacro, const TVector<std::string>& values, jinja2::ValuesMap& params, const std::string& renderPath);
    void AddFlagToParams(const std::string& attrMacro, const TVector<std::string>& values, jinja2::ValuesMap& params, const std::string& renderPath);
    void AddListToParams(const std::string& attrMacro, const TVector<std::string>& values, jinja2::ValuesMap& params);
    void AddSortedSetToParams(const std::string& attrMacro, const TVector<std::string>& values, jinja2::ValuesMap& params);
    void AddSetToParams(const std::string& attrMacro, const TVector<std::string>& values, jinja2::ValuesMap& params);
    void AddValuesToParams(const std::string& attrMacro, const EAttrTypes attrType, const TVector<std::string>& values, jinja2::ValuesMap& params, const std::string& renderPath);

    EAttrTypes GetAttrTypeFromSpec(const std::string& attrName, const std::string& attrMacro);

    bool IsExcludeInLibraryClasspath(const std::string& library, const std::string& exclude);
    void AddExcludesToTarget(const TJinjaTarget* target, jinja2::ValuesMap& targetMap, const std::string& renderPath);

    void RenderSubdir(const fs::path& root, const fs::path& subdir, const TJinjaList& data);

    std::string ProjectName;
    fs::path ArcadiaRoot;

    std::shared_ptr<jinja2::RealFileSystem> TemplateFs = std::make_shared<jinja2::RealFileSystem>();
    std::unique_ptr<jinja2::TemplateEnv> JinjaEnv = std::make_unique<jinja2::TemplateEnv>();
    std::vector<jinja2::Template> Templates;
    THashMap<std::string, std::vector<jinja2::Template>> TargetTemplates;

    TSubdirsTable Subdirs;
    TVector<TSubdirsTableElem*> SubdirsOrder;
    TDeque<TJinjaTarget> Targets;
    THashMap<std::string, TVector<std::string>> RootAttrs;

    THashMap<std::string, THashSet<std::string>> ClasspathByNodePath;
    THashMap<std::string, std::string> NodePathByLibrary;

    THashMap<TNodeId, NDetail::TPeersClosure> NodeClosures;
    THashMap<TNodeId, std::string> NodePaths;
    THashMap<TNodeId, std::string> NodeCoords;
    THashMap<std::string, TNodeId> NodeIds;
    THashMap<std::string, std::string> LibraryExcludes;
};