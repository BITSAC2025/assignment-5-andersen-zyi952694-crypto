/**
 * Andersen.cpp
 * @author kisslune
 */

#include "A5Header.h"

using namespace llvm;
using namespace std;

int main(int argc, char** argv)
{
    auto moduleNameVec =
            OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                     "[options] <input-bitcode...>");

    SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);

    SVF::SVFIRBuilder builder;
    auto pag = builder.build();
    auto consg = new SVF::ConstraintGraph(pag);
    consg->dump();

    Andersen andersen(consg);

    // TODO: complete the following method
    andersen.runPointerAnalysis();

    andersen.dumpResult();
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
	return 0;
}


void Andersen::runPointerAnalysis()
{
    // TODO: complete this method. Point-to set and worklist are defined in A5Header.h
    //  The implementation of constraint graph is provided in the SVF library
    WorkList<SVF::ConstraintNode *> worklist;

    // Initialize worklist with all constraint nodes
    for (auto it = consg->begin(); it != consg->end(); ++it)
    {
        worklist.push(it->second);
    }

    // Initialize points-to sets
    for (auto it = consg->begin(); it != consg->end(); ++it)
    {
        SVF::ConstraintNode *node = it->second;
        pts[node->getId()] = std::set<unsigned>();
    }

    // Process address constraints first
    for (auto it = consg->begin(); it != consg->end(); ++it)
    {
        SVF::ConstraintNode *node = it->second;
        for (SVF::ConstraintEdge *edge : node->getAddrInEdges())
        {
            if (edge->getEdgeKind() == SVF::ConstraintEdge::Addr)
            {
                SVF::ConstraintNode *srcNode = edge->getSrcNode();
                SVF::ConstraintNode *dstNode = edge->getDstNode();
                pts[dstNode->getId()].insert(srcNode->getId());
                worklist.push(dstNode);
            }
        }
    }

    // Main worklist algorithm
    while (!worklist.empty())
    {
        SVF::ConstraintNode *node = worklist.pop();

        // Process copy constraints
        for (SVF::ConstraintEdge *edge : node->getCopyOutEdges())
        {
            if (edge->getEdgeKind() == SVF::ConstraintEdge::Copy)
            {
                SVF::ConstraintNode *dstNode = edge->getDstNode();
                bool changed = false;

                // Propagate points-to information
                for (unsigned pointee : pts[node->getId()])
                {
                    if (pts[dstNode->getId()].insert(pointee).second)
                    {
                        changed = true;
                    }
                }

                if (changed)
                {
                    worklist.push(dstNode);
                }
            }
        }

        // Process load constraints
        for (SVF::ConstraintEdge *edge : node->getLoadOutEdges())
        {
            if (edge->getEdgeKind() == SVF::ConstraintEdge::Load)
            {
                SVF::ConstraintNode *dstNode = edge->getDstNode();
                bool changed = false;

                for (unsigned pointee : pts[node->getId()])
                {
                    // For each object that node points to, propagate its points-to set
                    SVF::ConstraintNode *objNode = consg->getConstraintNode(pointee);
                    if (objNode)
                    {
                        for (unsigned indirectPointee : pts[objNode->getId()])
                        {
                            if (pts[dstNode->getId()].insert(indirectPointee).second)
                            {
                                changed = true;
                            }
                        }
                    }
                }

                if (changed)
                {
                    worklist.push(dstNode);
                }
            }
        }

        // Process store constraints
        for (SVF::ConstraintEdge *edge : node->getStoreOutEdges())
        {
            if (edge->getEdgeKind() == SVF::ConstraintEdge::Store)
            {
                SVF::ConstraintNode *dstNode = edge->getDstNode();
                bool changed = false;

                for (unsigned pointee : pts[node->getId()])
                {
                    // For each object that node points to, add dstNode's points-to set to it
                    SVF::ConstraintNode *objNode = consg->getConstraintNode(pointee);
                    if (objNode)
                    {
                        for (unsigned dstPointee : pts[dstNode->getId()])
                        {
                            if (pts[objNode->getId()].insert(dstPointee).second)
                            {
                                changed = true;
                                worklist.push(objNode);
                            }
                        }
                    }
                }
            }
        }

        // Process GEP constraints (if needed)
        for (SVF::ConstraintEdge *edge : node->getGepOutEdges())
        {
            if (edge->getEdgeKind() == SVF::ConstraintEdge::NormalGep ||
                edge->getEdgeKind() == SVF::ConstraintEdge::VariantGep)
            {
                // Handle GEP constraints - simplified version
                SVF::ConstraintNode *dstNode = edge->getDstNode();
                bool changed = false;

                for (unsigned pointee : pts[node->getId()])
                {
                    // For field-sensitive analysis, we would adjust the offset
                    // Here we use a simplified approach that ignores offsets
                    if (pts[dstNode->getId()].insert(pointee).second)
                    {
                        changed = true;
                    }
                }

                if (changed)
                {
                    worklist.push(dstNode);
                }
            }
        }
    }
}
void Andersen::dumpResult()
{
    using namespace std;
    using namespace SVF;

    std::cout << "========== Andersen Points-to Results ==========" << std::endl;

    for (const auto &entry : pts)
    {
        unsigned varID = entry.first;
        const std::set<unsigned> &pointees = entry.second;

        std::cout << "Var " << varID << " -> { ";
        for (auto it = pointees.begin(); it != pointees.end(); ++it)
        {
            if (it != pointees.begin()) std::cout << ", ";
            std::cout << *it;
        }
        std::cout << " }" << std::endl;
    }

    std::cout << "===============================================" << std::endl;
}
