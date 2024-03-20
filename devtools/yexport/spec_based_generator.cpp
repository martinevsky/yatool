#include "spec_based_generator.h"
#include "yexport_spec.h"

#include <spdlog/spdlog.h>

namespace NYexport {

namespace {

    class TPrefixedFileSystem: public jinja2::IFilesystemHandler {
    public:
        TPrefixedFileSystem(const std::string& prefix)
            : Prefix_(prefix)
        {
        }

        jinja2::CharFileStreamPtr OpenStream(const std::string& name) const override {
            std::string realName(CutPrefix(name));
            return Filesystem_.OpenStream(realName);
        };
        jinja2::WCharFileStreamPtr OpenWStream(const std::string& name) const override {
            std::string realName(CutPrefix(name));
            return Filesystem_.OpenWStream(realName);
        };
        std::optional<std::chrono::system_clock::time_point> GetLastModificationDate(const std::string& name) const override {
            std::string realName(CutPrefix(name));
            return Filesystem_.GetLastModificationDate(realName);
        };

        void SetRootFolder(const fs::path& path) {
            Filesystem_.SetRootFolder(path);
        }

    private:
        std::string_view CutPrefix(const std::string& name) const {
            std::string_view sv = name;
            if (sv.find(Prefix_) == 0) {
                sv.remove_prefix(Prefix_.size());
            }
            return sv;
        }

        jinja2::RealFileSystem Filesystem_;
        const std::string Prefix_;
    };
}

const TGeneratorSpec& TSpecBasedGenerator::GetGeneratorSpec() const {
    return GeneratorSpec;
}

const fs::path& TSpecBasedGenerator::GetGeneratorDir() const {
    return GeneratorDir;
}

void TSpecBasedGenerator::OnAttribute(const TAttribute& attribute) {
    for (size_t i = 0; i < attribute.Size(); ++i) {
        std::string attr(attribute.GetFirstParts(i));
        UsedAttributes.emplace(attr);
        auto rules = GeneratorSpec.GetAttrRules(attr);
        UsedRules.insert(rules.begin(), rules.end());
    }
}

void TSpecBasedGenerator::OnPlatform(const std::string_view& platform) {
    auto rules = GeneratorSpec.GetPlatformRules(platform);
    UsedRules.insert(rules.begin(), rules.end());
}

void TSpecBasedGenerator::ApplyRules(TTargetAttributes& jinjaTemplate) const {
    for (const auto& rule : UsedRules) {
        for (const auto& [attr, values] : rule->AddValues) {
            jinjaTemplate.AppendAttrValue(attr, values);
        }
    }
}

jinja2::TemplateEnv* TSpecBasedGenerator::GetJinjaEnv() const {
    return JinjaEnv.get();
}

void TSpecBasedGenerator::SetupJinjaEnv() {
    JinjaEnv = std::make_unique<jinja2::TemplateEnv>();
    SourceTemplateFs = std::make_shared<jinja2::RealFileSystem>();
    auto GeneratorTemplateFs = std::make_shared<TPrefixedFileSystem>(GENERATOR_TEMPLATES_PREFIX);
    GeneratorTemplateFs->SetRootFolder(GeneratorDir);
    SourceTemplateFs->SetRootFolder(ArcadiaRoot);

    // Order matters. Jinja should search files with generator/ prefix in generator directory, and only if there is no such file fallback to source directory
    JinjaEnv->AddFilesystemHandler(GENERATOR_TEMPLATES_PREFIX, GeneratorTemplateFs);
    JinjaEnv->AddFilesystemHandler({}, SourceTemplateFs);
}
void TSpecBasedGenerator::SetCurrentDirectory(const fs::path& dir) const {
    YEXPORT_VERIFY(JinjaEnv, "Cannot set current directory to " << dir << " before setting up jinja enviroment");
    SourceTemplateFs->SetRootFolder(dir.c_str());
}


const TNodeSemantics& TSpecBasedGenerator::ApplyReplacement(TPathView path, const TNodeSemantics& inputSem) const {
    return TargetReplacements_.ApplyReplacement(path, inputSem);
}

TYexportSpec TSpecBasedGenerator::ReadYexportSpec(fs::path configDir) {
    if (!configDir.empty()) {
        auto yexportToml = configDir / YEXPORT_FILE;
        if (fs::exists(yexportToml)) {
            LoadTargetReplacements(yexportToml, TargetReplacements_);
            return ::NYexport::ReadYexportSpec(yexportToml);
        }
    }
    return {};
}

fs::path TSpecBasedGenerator::PathByCopyLocation(ECopyLocation location) const {
    switch (location) {
        case ECopyLocation::GeneratorRoot:
            return GeneratorDir;
        case ECopyLocation::SourceRoot:
            return ArcadiaRoot;
        default:
            YEXPORT_THROW("Unknown copy location");
    }
}

TCopySpec TSpecBasedGenerator::CollectFilesToCopy() const {
    TCopySpec result;

    for (auto rule : UsedRules) {
        result.Append(rule->Copy);
    }
    result.Append(GeneratorSpec.Root.Copy);
    return result;
}

void TSpecBasedGenerator::CopyFilesAndResources() {
    for (const auto& [location, files] : CollectFilesToCopy().Items) {
        auto dir = PathByCopyLocation(location);
        for (const auto& file : files) {
            ExportFileManager->Copy(dir / file, file);
        }
    }
}
}
