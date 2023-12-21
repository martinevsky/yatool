#include "json_visitor_new.h"

#include "module_restorer.h"
#include "parser_manager.h"

TJSONVisitorNew::TJSONVisitorNew(const TRestoreContext& restoreContext, TCommands& commands, const TVector<TTarget>& startDirs, bool newUids)
    : TBase{restoreContext, TDependencyFilter{TDependencyFilter::SkipRecurses}}
    , Commands(commands)
    , NewUids(newUids)
    , Edge(restoreContext.Graph.GetInvalidEdge())
    , CurrNode(restoreContext.Graph.GetInvalidNode())
{
    Loops.FindLoops(RestoreContext.Graph, startDirs, false);
}


bool TJSONVisitorNew::AcceptDep(TState& state) {
    return TBase::AcceptDep(state);
}

bool TJSONVisitorNew::Enter(TState& state) {
    bool fresh = TBase::Enter(state);
    if (fresh)
        CurEnt->InitUids(NewUids);

    if (NewUids) {
        UpdateReferences(state);

        ++CurrData->NewUids()->EnterDepth;

        if (fresh && !CurrData->Completed) {
            Y_ASSERT(!CurrData->NewUids()->Finished);
            PrepareCurrent(state);
        }
    }

    return fresh;
}

void TJSONVisitorNew::Leave(TState& state) {
    if (NewUids) {
        UpdateReferences(state);

        TNodeId chldLoop = CurrData->LoopId;
        bool inSameLoop = chldLoop && PrntData && chldLoop == PrntData->LoopId;

        --CurrData->NewUids()->EnterDepth;
        if (CurrData->NewUids()->EnterDepth == 0 && !CurrData->NewUids()->Finished) {
            FinishCurrent(state);
            CurrData->NewUids()->Finished = true;
            if (chldLoop && !inSameLoop) {
                ComputeLoopHash(chldLoop);
            }
        }

        if (PrntState != nullptr && !PrntData->NewUids()->Finished && CurrData->NewUids()->Finished && !inSameLoop) {
            PassToParent(state);
        }
    }

    TBase::Leave(state);
}

void TJSONVisitorNew::Left(TState& state) {
    TBase::Left(state);
}

void TJSONVisitorNew::PrepareCurrent(TState& state) {
    Y_UNUSED(state);

    TNodeDebugOnly nodeDebug{*Graph, CurrNode.Id()};
    Y_ASSERT(!CurrState->Hash);
    CurrState->Hash = new TJsonMd5New(nodeDebug);

    if (IsModuleType(CurrNode->NodeType) && !CurrState->Module) {
        CurrState->Module = RestoreContext.Modules.Get(CurrNode->ElemId);
        Y_ENSURE(CurrState->Module != nullptr);
    }
}

void TJSONVisitorNew::FinishCurrent(TState& state) {
    if (CurrNode->NodeType == EMNT_BuildCommand) {
        auto name = CurrState->GetCmdName();
        if (!name.IsNewFormat()) {
            auto str = name.GetStr();
            TStringBuf val = GetCmdValue(str);
            TStringBuf cmdName = GetCmdName(str);
            if (cmdName.EndsWith("__NO_UID__")) {
                YDIAG(V) << "Command not accounted for in uid: " << cmdName << ", value: " << val << Endl;
            } else {
                UpdateCurrent(state, val, "Include command value to structure uid");
            }
        } else {
            auto expr = Commands.Get(Commands.IdByElemId(name.GetElemId()));
            Y_ASSERT(expr);
            Commands.StreamCmdRepr(*expr, [&](auto data, auto size) {
                CurrState->Hash->New()->StructureMd5Update({data, size}, "new_cmd");
            });
        }
        CurrState->Hash->New()->StructureMd5Update(RestoreContext.Conf.GetUidsSalt(), "FAKEID");
    }

    // include content hash to content uid
    if (CurrNode->NodeType == EMNT_File) {
        auto elemId = CurrNode->ElemId;
        const TFileData& fileData = Graph->Names().FileConf.GetFileDataById(elemId);
        Y_ASSERT(fileData.HashSum != TMd5Sig());
        TString value = Md5SignatureAsBase64(fileData.HashSum);
        CurrState->Hash->New()->ContentMd5Update(value, "Update content hash by current file hash sum");
        CurrState->Hash->New()->IncludeContentMd5Update(value, "Update include content hash by current file hash sum");
    }

    if (CurrNode->NodeType == EMNT_NonParsedFile) {
        TString nodeName = Graph->ToString(CurrNode);
        TMd5Value md5{TNodeDebugOnly{*Graph, CurrNode.Id()}, "TJSONVisitorNew::Enter::<md5#2>"sv};
        md5.Update(nodeName, "TJSONVisitorNew::Enter::<nodeName>"sv);
        CurrState->Hash->New()->ContentMd5Update(nodeName, "Update content hash by current file name");
        CurrState->Hash->New()->IncludeContentMd5Update(nodeName, "Update include content hash by current file name");
    }

    // include extra output names to parent entry
    for (auto dep : CurrNode.Edges()) {
        if (*dep == EDT_OutTogetherBack) {
            TString depName = Graph->ToString(dep.To());
            UpdateCurrent(state, depName, "Include extra output name to structure uid");
        }
    }

    // include owners and module tag
    if (IsModuleType(CurrNode->NodeType)) {
        auto tag = CurrState->Module->GetTag();
        if (tag) {
            UpdateCurrent(state, tag, "Include module tag to structure uid");
            if (CurrState->Module->IsFromMultimodule()) {
                // In fact tag is only emitted into node for multimodules, so make distinction
                UpdateCurrent(state, "mm", "Include multimodule tag to structure uid");
            }
        }

        // include managed peers closure to structure hash
        if (CurrState->Module->IsDependencyManagementApplied() && CurrData->NodeDeps) {
            for (TNodeId nodeId : *CurrData->NodeDeps) {
                const auto depNode = RestoreContext.Graph.Get(nodeId);
                const auto name = RestoreContext.Graph.GetNameFast(depNode);
                UpdateCurrent(state, name, "Include managed peer name to structure hash");
            }
        }
    }

    // Node name will be in $AUTO_INPUT for the consumer of this node.
    if (IsFileType(CurrNode->NodeType)) {
        CurrState->Hash->New()->IncludeStructureMd5Update(Graph->ToString(CurrNode), "Node name for $AUTO_INPUT");
    }

    // There is no JSON node for current DepGraph node
    if (CurrData->HasBuildCmd) {
        AddAddincls(state);

        AddGlobalVars(state);

        // include main output name to cur entry
        if (IsFileType(CurrNode->NodeType)) {
            TString nodeName = Graph->ToString(CurrNode);
            UpdateCurrent(state, nodeName, "Include node name to current structure hash");
        }
    }

    if (UseFileId(CurrNode->NodeType)) {
        CurrData->NewUids()->SetContentUid(CurrState->Hash->New()->GetContentMd5());
        CurrData->NewUids()->SetIncludeContentUid(CurrState->Hash->New()->GetIncludeContentMd5());
    }

    CurrData->NewUids()->SetStructureUid(CurrState->Hash->New()->GetStructureMd5());
    CurrData->NewUids()->SetIncludeStructureUid(CurrState->Hash->New()->GetIncludeStructureMd5());
}

void TJSONVisitorNew::PassToParent(TState& state) {
    // include inputs name
    bool isBuildFromDep = *Edge == EDT_BuildFrom && IsFileType(CurrNode->NodeType);
    if (isBuildFromDep && CurrNode->NodeType != EMNT_File) {
        TString nodeName = Graph->ToString(CurrNode);
        UpdateParent(state, nodeName, "Include input name to parent structure hash");
    }

    // include build command
    if (IsBuildCommandDep(Edge) || IsInnerCommandDep(Edge)) {
        UpdateParent(state, CurrData->NewUids()->GetStructureUid(), "Include build cmd name to parent structure hash");
    }

    // tool dep
    if (CurrNode->NodeType == EMNT_Program && IsDirectToolDep(Edge)) {
        TString nodeName = Graph->ToString(CurrNode);
        UpdateParent(state, nodeName, "Include tool node name to parent structure hash");
    }

    // late globs
    if (*Edge == EDT_BuildFrom && CurrNode->NodeType == EMNT_BuildCommand) {
        UpdateParent(state, CurrData->NewUids()->GetStructureUid(), "Include late glob hash to parent structure hash");
    }

    if (*Edge == EDT_Property && IsFileType(CurrNode->NodeType)) {
        TString nodeName = Graph->ToString(CurrNode);
        UpdateParent(state, nodeName, "Include file names to late glob structure hash");
    }

    if (IsFileType(CurrNode->NodeType) && IsFileType(PrntState->Node()->NodeType)) {
        bool isModuleDep = IsDirectPeerdirDep(Edge) && PrntState->Module->PassPeers();
        if (*Edge == EDT_Include || isModuleDep) {
            bool isDMModules = false;
            if (isModuleDep) {
                if (!CurrState->Module) {
                    CurrState->Module = RestoreContext.Modules.Get(CurrNode->ElemId);
                    Y_ENSURE(CurrState->Module != nullptr);
                }
                isDMModules = CurrState->Module->GetAttrs().RequireDepManagement && PrntState->Module->GetAttrs().RequireDepManagement;
            }
            if (!isDMModules) {
                PrntState->Hash->New()->IncludeStructureMd5Update(CurrData->NewUids()->GetIncludeStructureUid(), "Pass structure for consumer by EDT_Include");
            }
        }

        if (*Edge == EDT_BuildFrom) {
            PrntState->Hash->New()->StructureMd5Update(CurrData->NewUids()->GetIncludeStructureUid(), "Pass structure for consumer by EDT_BuildFrom");
        }
    }

    if (IsGlobalSrcDep(Edge)) {
        TString globalSrcName = Graph->ToString(CurrNode);
        PrntState->Hash->New()->IncludeStructureMd5Update(globalSrcName, "Add GlobalSrc name"sv);

        PrntState->Hash->New()->IncludeStructureMd5Update(CurrData->NewUids()->GetIncludeStructureUid(), "Add IncludeStructure from GlobalSrc"sv);
    }

    if (IsIncludeFileDep(Edge)) {
        PrntState->Hash->New()->IncludeContentMd5Update(CurrData->NewUids()->GetIncludeContentUid(), "Pass IncludeContent by EDT_Include"sv);
    }

    if (*Edge == EDT_BuildFrom && IsSrcFileType(CurrNode->NodeType)) {
        if (CurrNode->NodeType != EMNT_NonParsedFile) {
            PrntState->Hash->New()->ContentMd5Update(CurrData->NewUids()->GetContentUid(), "Update parent content UID with content"sv);
        }
        PrntState->Hash->New()->ContentMd5Update(CurrData->NewUids()->GetIncludeContentUid(), "Update parent content UID with include content"sv);
    }
}

void TJSONVisitorNew::UpdateParent(TState& state, TStringBuf value, TStringBuf description) {
    const auto& parentState = *state.Parent();
    const auto& graph = TDepGraph::Graph(state.TopNode());

    YDIAG(Dev) << description << ": " << value << " " << graph.ToString(parentState.Node()) << Endl;
    parentState.Hash->New()->StructureMd5Update(value, value);
}

void TJSONVisitorNew::UpdateParent(TState& state, const TMd5SigValue& value, TStringBuf description) {
    const auto& parentState = *state.Parent();
    const auto& graph = TDepGraph::Graph(state.TopNode());

    TString nodeName = graph.ToString(state.TopNode());

    YDIAG(Dev) << description << ": " << nodeName << " " << graph.ToString(parentState.Node()) << Endl;
    parentState.Hash->New()->StructureMd5Update(value, nodeName);
}

void TJSONVisitorNew::UpdateCurrent(TState& state, TStringBuf value, TStringBuf description) {
    const auto& graph = TDepGraph::Graph(state.TopNode());

    YDIAG(Dev) << description << ": " << value << " " << graph.ToString(state.TopNode()) << Endl;
    state.Top().Hash->New()->StructureMd5Update(value, value);
}

void TJSONVisitorNew::AddAddincls(TState& state) {
    const auto moduleIt = FindModule(state);
    bool hasModule = moduleIt != state.end();
    if (!hasModule || !moduleIt->Module || !CurrData->UsedReservedVars) {
        return;
    }

    const auto& includesMap = moduleIt->Module->IncDirs.GetAll();
    const auto& usedVars = *CurrData->UsedReservedVars;
    for (size_t lang = 0; lang < NLanguages::LanguagesCount(); lang++) {
        auto&& includeVarName = TModuleIncDirs::GetIncludeVarName(static_cast<TLangId>(lang));
        if (!usedVars.contains(includeVarName)) {
            continue;
        }
        moduleIt->Module->IncDirs.MarkLanguageAsUsed(static_cast<TLangId>(lang));
        const auto includesIt = includesMap.find(static_cast<TLangId>(lang));
        if (includesIt == includesMap.end()) {
            continue;
        }
        for (const auto& dir : includesIt->second.Get()) {
            TStringBuf dirStr = dir.GetTargetStr();
            UpdateCurrent(state, dirStr, "Include addincl dir to current structure hash");
        }
    }
}

void TJSONVisitorNew::AddGlobalVars(TState& state) {
    const auto moduleIt = FindModule(state);
    bool hasModule = moduleIt != state.end();
    if (!hasModule || !moduleIt->Module || !CurrData->UsedReservedVars) {
        return;
    }
    for (const auto& varStr : RestoreContext.Modules.GetGlobalVars(moduleIt->Module->GetId()).GetVars()) {
        if (CurrData->UsedReservedVars->contains(varStr.first)) {
            for (const auto& varItem : varStr.second) {
                TString value = FormatProperty(varStr.first, varItem.Name);
                UpdateCurrent(state, value, "Include global var to current structure hash");
            }
        }
    }
}

void TJSONVisitorNew::ComputeLoopHash(TNodeId loopId) {
    const TGraphLoop& loop = Loops[loopId];

    TJsonMultiMd5 structureLoopHash(loopId, Graph->Names(), loop.size());
    TJsonMultiMd5 includeStructureLoopHash(loopId, Graph->Names(), loop.size());
    TJsonMultiMd5 contentLoopHash(loopId, Graph->Names(), loop.size());
    TJsonMultiMd5 includeContentLoopHash(loopId, Graph->Names(), loop.size());

    for (const auto& nodeId : loop) {
        const auto nodeDataIt = Nodes.find(nodeId);
        if (nodeDataIt == Nodes.end()) {
            // Loops now contains directories for peerdirs while this visitor
            // skips such directories by direct module-to-module edges
            continue;
        }

        const TJSONEntryStats& nodeData = nodeDataIt->second;
        TString nodeName = Graph->ToString(Graph->Get(nodeId));
        structureLoopHash.AddSign(nodeData.NewUids()->GetStructureUid(), nodeName, true);
        includeStructureLoopHash.AddSign(nodeData.NewUids()->GetIncludeStructureUid(), nodeName, true);
        contentLoopHash.AddSign(nodeData.NewUids()->GetContentUid(), nodeName, true);
        includeContentLoopHash.AddSign(nodeData.NewUids()->GetIncludeContentUid(), nodeName, true);
    }

    TMd5SigValue structureLoopUid{"TJSONVisitorNew::StructureLoopUID"sv};
    TMd5SigValue includeStructureLoopUid{"TJSONVisitorNew::IncludeStructureLoopUID"sv};
    TMd5SigValue contentLoopUid{"TJSONVisitorNew::ContentLoopUID"sv};
    TMd5SigValue includeContentLoopUid{"TJSONVisitorNew::IncludeContentLoopUID"sv};
    structureLoopHash.CalcFinalSign(structureLoopUid);
    includeStructureLoopHash.CalcFinalSign(includeStructureLoopUid);
    contentLoopHash.CalcFinalSign(contentLoopUid);
    includeContentLoopHash.CalcFinalSign(includeContentLoopUid);

    for (const auto& node : loop) {
        const auto nodeDataIt = Nodes.find(node);
        if (nodeDataIt == Nodes.end()) {
            continue;
        }

        TJSONEntryStats& nodeData = nodeDataIt->second;
        nodeData.NewUids()->SetIncludeStructureUid(includeStructureLoopUid);
        nodeData.NewUids()->SetContentUid(contentLoopUid);
        nodeData.NewUids()->SetIncludeContentUid(includeContentLoopUid);

        if (!nodeData.HasBuildCmd) {
            nodeData.NewUids()->SetStructureUid(structureLoopUid);

        } else {
            TMd5Value structureHash{"Loop structure hash with name"sv};
            structureHash.Update(structureLoopUid, "Common loop structure hash"sv);
            TString mainOutputName = Graph->ToString(Graph->Get(node));
            structureHash.Update(mainOutputName, "Main output name"sv);

            TMd5SigValue structureUid{"Loop structure uid with name"sv};
            structureUid.MoveFrom(std::move(structureHash));

            nodeData.NewUids()->SetStructureUid(structureUid);
        }
    }
}

void TJSONVisitorNew::UpdateReferences(TState& state) {
    CurrState = &state.Top();
    CurrData = CurEnt;

    if (state.HasIncomingDep()) {
        PrntState = &*state.Parent();
        PrntData = reinterpret_cast<TNodeData*>(PrntState->Cookie);
    } else {
        PrntState = nullptr;
        PrntData = nullptr;
    }

    CurrNode.~TNodeRefBase();
    new (&CurrNode) TDepGraph::TConstNodeRef(state.TopNode());

    Edge.~TEdgeRefBase();
    new (&Edge) TDepGraph::TConstEdgeRef(state.IncomingDep());

    const auto node = CurrState->Node();
    Graph = &TDepGraph::Graph(node);
}
