#include "sem_graph.h"
#include "dump_info.h"
#include "exec.h"
#include "mkcmd.h"
#include "module_store.h"
#include "flat_json_graph.h"
#include "json_subst.h"
#include "tools_miner.h"
#include "prop_names.h"

#include <devtools/ymake/compact_graph/query.h>
#include <devtools/ymake/compact_graph/dep_types.h>
#include <devtools/ymake/compact_graph/iter_direct_peerdir.h>
#include <devtools/ymake/managed_deps_iter.h>

#include <util/generic/stack.h>
#include <util/generic/overloaded.h>

template<>
struct NFlatJsonGraph::TNodePropertyTriat<TSingleCmd> {
    static void Serialize(NJsonWriter::TBuf& to, TSingleCmd value) {
        to.BeginObject();
        to.WriteKey("sem");
        std::visit(TOverloaded{
            [&](const TString& cmd) {
                to.UnsafeWriteValue(cmd);
            },
            [&](const TVector<TString>& cmd) {
                to.BeginList();
                for (auto&& arg: cmd)
                    to.WriteString(arg);
                to.EndList();
            }
        }, value.CmdStr);
        if (!value.Cwd.empty()) {
            to.WriteKey("cwd");
            to.WriteString(value.Cwd);
        }
        to.EndObject();
    }
};

namespace {

    class TDumpInfoSem : public TDumpInfoEx {
    public:
        TDumpInfoSem(TRestoreContext restoreContext, TConstDepNodeRef node)
            : RestoreContext{restoreContext}
            , Node{node}
        {}

        void SetExtraValues(TVars& vars) override {
            if (!IsModuleType(Node->NodeType)) {
                return;
            }

            const auto* mod = RestoreContext.Modules.Get(Node->ElemId);
            Y_ASSERT(mod);

            vars.SetValue("FAKE_MODULE", mod->IsFakeModule() ? "yes" : "no");

            for (size_t pos = 0; pos < mod->IncDirs.GetAllUsed().size(); ++pos) {
                const auto lang = static_cast<TLangId>(pos);
                auto& ownedAddincls = vars[mod->IncDirs.GetIncludeVarName(lang) + "_OWNED"];
                auto& globalAddincls = vars[mod->IncDirs.GetIncludeVarName(lang) + "_GLOBAL"];
                for (const auto& dir: mod->IncDirs.GetOwned(lang)) {
                    ownedAddincls.push_back({dir.GetTargetStr(), false, true});
                }
                for (const auto& dir: mod->IncDirs.GetGlobal(lang)) {
                    globalAddincls.push_back({dir.GetTargetStr(), false, true});
                }
            }

            for (const auto& dep: Node.Edges()) {
                if (!IsBuildCmdInclusion(dep)) {
                    continue;
                }
                const auto& varStr = RestoreContext.Graph.GetCmdName(dep.To()).GetStr();
                const TStringBuf varname = GetCmdName(varStr);
                const TStringBuf varval = GetCmdValue(varStr);
                vars[varname + TString("_RAW")].push_back(varval);
            }
        }

    private:
        TRestoreContext RestoreContext;
        TConstDepNodeRef Node;
    };

    class TSemFormatter: public TJsonCmdAcceptor {
    public:
        TVector<TSingleCmd>&& TakeCommands() && noexcept {return std::move(FormattedCommands);}

    protected:
        void OnCmdFinished(const TVector<TSingleCmd>& commands, TCommandInfo& cmdInfo[[maybe_unused]], const TVars& vars[[maybe_unused]]) override {
            FormattedCommands = commands;
        }

    private:
        TVector<TSingleCmd> FormattedCommands;
    };

    TVector<TSingleCmd> FormatCmd(
        const TRestoreContext& restoreContext,
        const TCommands& commands,
        TNodeId nodeId,
        TNodeId modId,
        TDumpInfoSem& semInfoProvider
    ) {
        TSemFormatter formatter{};
        TMakeCommand mkcmd{restoreContext, commands, nullptr, &restoreContext.Conf.CommandConf};
        mkcmd.CmdInfo.MkCmdAcceptor = formatter.GetAcceptor();
        mkcmd.GetFromGraph(nodeId, modId, ECmdFormat::ECF_Json, &semInfoProvider);
        return std::move(formatter).TakeCommands();
    }

    class SemGraphRenderVisitor: public TManagedPeerConstVisitor<> {
    private:
        struct ModInfo {
            TNodeId ModNode;
            ui32 GlobalLibId;
        };

    public:
        using TBase = TManagedPeerConstVisitor<>;
        using typename TBase::TState;

        SemGraphRenderVisitor(
            const TRestoreContext& restoreContext,
            const TCommands& commands,
            THashSet<TNodeId> startDirs,
            const THashSet<TTarget>& modStartTargets,
            IOutputStream& out
        )
            : TBase{restoreContext}
            , Commands{commands}
            , JsonWriter{out}
            , StartDirs{std::move(startDirs)}
            , ModStartTargets{modStartTargets}
        {}

        bool AcceptDep(TState& state) {
            if (!TBase::AcceptDep(state)) {
                return false;
            }
            const auto& dep = state.NextDep();
            if (IsPropertyDep(dep)) {
                return false;
            }
            if (!state.HasIncomingDep() && IsDirToModuleDep(dep) && !ModStartTargets.contains(dep.To().Id())) {
                return false;
            }
            return !IsIncludeFileDep(dep) && *dep != EDT_OutTogetherBack;
        }

        bool Enter(TState& state) {
            if (!TBase::Enter(state)) {
                return false;
            }
            const auto& lists = RestoreContext.Modules.GetNodeListStore();
            TVector<ui32> tests;
            const auto& topNode = state.TopNode();
            if (IsModuleType(topNode->NodeType)) {
                const TModule* mod = RestoreContext.Modules.Get(topNode->ElemId);
                if (mod->IsDependencyManagementApplied()) {
                    const auto& modListIds = RestoreContext.Modules.GetModuleNodeIds(mod->GetId());
                    const auto& managedPeersClosure = lists.GetList(modListIds.UniqPeers).Data();
                    if (!managedPeersClosure.empty()) {
                        // Iterate all peers closure before add node ids, else peers of contrib peers will not add to sem graph,
                        // and as result node ids below will to point to absent nodes
                        IterateAll(RestoreContext.Graph, managedPeersClosure, state, *this);
                    }
                }
                ModulesStack.push({
                    .ModNode = topNode.Id(),
                    .GlobalLibId = mod->GetAttrs().UseGlobalCmd ? mod->GetGlobalLibId() : 0
                });
                if (RestoreContext.Conf.ShouldTraverseDepsTests()) {
                    tests = RenderTests(
                        RestoreContext.Graph.Names().FileConf.ConstructLink(ELinkType::ELT_MKF, mod->GetMakefile())
                    );
                }
            }

            if (UseFileId(topNode->NodeType)) {
                auto node = JsonWriter.AddNode(topNode);
                if (IsOutputType(topNode->NodeType) && !AnyOf(topNode.Edges(), [](const auto& dep) {return *dep == EDT_OutTogether;})) {
                    Y_ASSERT(!ModulesStack.empty());
                    const auto& modinfo = ModulesStack.top();
                    TDumpInfoSem semVarsProvider{
                        RestoreContext,
                        modinfo.GlobalLibId == topNode->ElemId ? RestoreContext.Graph[modinfo.ModNode] : topNode
                    };
                    const auto sem = FormatCmd(RestoreContext, Commands, topNode.Id(), ModulesStack.top().ModNode, semVarsProvider);
                    node.AddProp("semantics", sem);
                    auto mod = RestoreContext.Modules.Get(topNode->ElemId);
                    if (mod && mod->IsDependencyManagementApplied()) {
                        const auto& modListIds = RestoreContext.Modules.GetModuleNodeIds(mod->GetId());
                        const auto& managedPeersClosure = lists.GetList(modListIds.UniqPeers).Data();
                        if (!managedPeersClosure.empty()) {
                            TVector<ui32> elemIds;
                            elemIds.reserve(managedPeersClosure.size());
                            for (const auto nodeId: managedPeersClosure) {
                                elemIds.emplace_back(RestoreContext.Graph.Get(nodeId)->ElemId);
                            }
                            node.AddProp("ManagedPeersClosure", std::move(elemIds));
                        }
                    }
                    if (mod && mod->IsSemIgnore()) {
                        return false;
                    }

                    if (AnyOf(sem, [](const TSingleCmd& cmd) {
                        static const TSingleCmd::TCmdStr ignored = "[\"IGNORED\"]";
                        return cmd.CmdStr == ignored;
                    })) {
                        return false;
                    }

                    const auto tools = ToolMiner.MineTools(topNode);
                    if (!tools.empty()) {
                        node.AddProp("Tools", tools);
                    }
                    if (RestoreContext.Conf.ShouldTraverseDepsTests() && !tests.empty()) {
                        node.AddProp("Tests", tests);
                    }
                }
                if (topNode->NodeType == EMNT_Directory && StartDirs.contains(topNode.Id())) {
                    node.AddProp("Tag", "StartDir");
                }
            }

            return true;
        }

        void Left(TState& state) {
            TBase::Left(state);
            const auto& dep = state.NextDep();
            if (!ModulesStack.empty() && dep.To().Id() == ModulesStack.top().ModNode) {
                ModulesStack.pop();
            }
            if (UseFileId(dep.From()->NodeType) && UseFileId(dep.To()->NodeType) && AcceptDep(state)) {
                auto node = JsonWriter.AddLink(dep);
                AddExcludeProperty(node, dep);
                AddIsClosureProperty(node, dep);
            }
        }

    private:
        TVector<ui32> RenderTests(TFileView mkf) {
            TVector<ui32> res;
            const auto node = RestoreContext.Graph.GetFileNode(mkf);
            for (const auto& dep: node.Edges()) {
                if (!IsMakeFilePropertyDep(dep.From()->NodeType, *dep, dep.To()->NodeType) || GetCmdName(RestoreContext.Graph.GetCmdName(dep.To()).GetStr()) != NProps::TEST_RECURSES) {
                    continue;
                }
                for (const auto &testdep: dep.To().Edges()) {
                    if (IsSearchDirDep(testdep)) {
                        res.push_back(testdep.To()->ElemId);
                    }
                }
            }
            return res;
        }

        void AddExcludeProperty(NFlatJsonGraph::TNodeWriter node, TConstDepRef dep) {
            auto excludeNodeIds = ComputeExcludeNodeIds(dep);
            if (!excludeNodeIds.empty()) {
                // To dependences attribute name after ':' add info about type ([] - array) and element type (NodeId)
                // P.S. In sem-graph Id of node is ElemId from dep-graph, that is why typename of element is "NodeId"
                node.AddProp("Excludes:[NodeId]", TDepGraph::NodeToElemIds(RestoreContext.Graph, excludeNodeIds));
            }
        }

        void AddIsClosureProperty(NFlatJsonGraph::TNodeWriter node, TConstDepRef dep) {
            auto isClosure = ComputeIsClosure(dep);
            if (isClosure) {
                node.AddProp("IsClosure:bool", isClosure);
            }
        }

        THashSet<TNodeId> ComputeExcludeNodeIds(TConstDepRef dep) {
            if (!IsDirectPeerdirDep(dep)) {
                return {};
            }
            const TModule* fromMod = RestoreContext.Modules.Get(dep.From()->ElemId);
            const TModule* toMod = RestoreContext.Modules.Get(dep.To()->ElemId);
            if (!fromMod->IsDependencyManagementApplied() || !toMod->IsDependencyManagementApplied()) {
                // Excludes can compute only if both modules under DM
                return {};
            }
            const auto& lists = RestoreContext.Modules.GetNodeListStore();
            auto toManagedPeersClosureListId = RestoreContext.Modules.GetModuleNodeIds(toMod->GetId()).UniqPeers;
            const auto& toManagedPeersClosure = lists.GetList(toManagedPeersClosureListId).Data();
            if (toManagedPeersClosure.empty()) {
                // Empty closure, nothing can be excluded
                return {};
            }
            THashSet<TNodeId> excludeNodeIds(toManagedPeersClosure.begin(), toManagedPeersClosure.end());
            auto fromManagedPeersClosureListId = RestoreContext.Modules.GetModuleNodeIds(fromMod->GetId()).UniqPeers;
            const auto& fromManagedPeersClosure = lists.GetList(fromManagedPeersClosureListId).Data();
            // Remove from TO (library) managed peers closure all FROM (program) managed peers closure ids
            // As result, after subtracting sets, we found all nodes for exclude by this dependency
            for (auto nodeId : fromManagedPeersClosure) {
                if (auto nodeIt = excludeNodeIds.find(nodeId); nodeIt != excludeNodeIds.end()) {
                    excludeNodeIds.erase(nodeIt);
                }
            }
            return excludeNodeIds;
        }

        bool ComputeIsClosure(TConstDepRef dep) {
            if (!IsDirectPeerdirDep(dep)) {
                // Managed peers closure added to sem graph as direct peerdirs
                return false;
            }
            const TModule* fromMod = RestoreContext.Modules.Get(dep.From()->ElemId);
            if (!fromMod->IsDependencyManagementApplied()) {
                // IsClosure can compute only if FROM module under DM
                return false;
            }
            const auto& lists = RestoreContext.Modules.GetNodeListStore();
            auto fromManagedDirectPeersListId = RestoreContext.Modules.GetModuleNodeIds(fromMod->GetId()).ManagedDirectPeers;
            const auto& fromManagedDirectPeersList = lists.GetList(fromManagedDirectPeersListId).Data();
            // Absent in direct peers, but is IsDirectPeerdirDep (see above) => is closure
            return Find(fromManagedDirectPeersList, dep.To().Id()) == fromManagedDirectPeersList.end();
        }

    private:
        const TCommands& Commands;
        NFlatJsonGraph::TWriter JsonWriter;
        TStack<ModInfo> ModulesStack;
        THashSet<TNodeId> StartDirs;
        const THashSet<TTarget>& ModStartTargets;
        TToolMiner ToolMiner;
    };

}

void RenderSemGraph(
    IOutputStream& out,
    TRestoreContext restoreContext,
    const TCommands& commands,
    TTraverseStartsContext startsContext
) {
    THashSet<TNodeId> startDirs;
    for (const auto& tgt : startsContext.StartTargets) {
        if (!tgt.IsNonDirTarget && !tgt.IsDepTestTarget) {
            startDirs.insert(tgt.Id);
        }
    }
    SemGraphRenderVisitor visitor{restoreContext, commands, std::move(startDirs), startsContext.ModuleStartTargets, out};
    IterateAll(restoreContext.Graph, startsContext.StartTargets, visitor, [](const TTarget& t) -> bool { return !t.IsModuleTarget; });
}
