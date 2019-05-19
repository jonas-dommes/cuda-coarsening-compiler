// ============================================================================
// Copyright (c) Richard Rohac, 2019, All rights reserved.
// ============================================================================
// CUDA Coarsening Transformation pass
// -> Based on Alberto's Magni OpenCL coarsening pass algorithm
//    available at https://github.com/HariSeldon/coarsening_pass
// ============================================================================

#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Analysis/PostDominators.h>

#include "Common.h"
#include "CUDACoarsening.h"
#include "Util.h"
#include "RegionBounds.h"
#include "DivergentRegion.h"
#include "DivergenceAnalysisPass.h"
#include "GridAnalysisPass.h"

#include <utility>

void CUDACoarseningPass::replicateRegion(DivergentRegion *region)
{
    assert(m_domT->dominates(region->getHeader(), region->getExiting()) &&
           "Header does not dominate Exiting");
    assert(m_postDomT->dominates(region->getExiting(), region->getHeader()) &&
         "Exiting does not post dominate Header");

    replicateRegionClassic(region);
}

void CUDACoarseningPass::replicateRegionClassic(DivergentRegion *region)
{
    CoarseningMap aliveMap;
    initAliveMap(region, aliveMap);
    replicateRegionImpl(region, aliveMap);
    updatePlaceholdersWithAlive(aliveMap);
}

void CUDACoarseningPass::initAliveMap(DivergentRegion *region,
                                      CoarseningMap&   aliveMap)
{
    InstVector &aliveInsts = region->getAlive();
    for (auto &inst : aliveInsts) {
        aliveMap.insert(std::pair<Instruction *, InstVector>(inst,
                                                             InstVector()));
    }
}

void CUDACoarseningPass::updateAliveMap(CoarseningMap& aliveMap,
                                        Map&           regionMap)
{
  for (auto &mapIter : aliveMap) {
    InstVector &coarsenedInsts = mapIter.second;
    Value *value = regionMap[mapIter.first];
    assert(value != nullptr && "Missing alive value in region map");
    coarsenedInsts.push_back(dyn_cast<Instruction>(value));
  }
}

void CUDACoarseningPass::updatePlaceholdersWithAlive(CoarseningMap& aliveMap) {
  // Force the addition of the alive values to the coarsening map. 
  for (auto mapIter : aliveMap) {
    Instruction *alive = mapIter.first;
    InstVector &coarsenedInsts = mapIter.second;

    auto cIter = m_coarseningMap.find(alive); 
    if(cIter == m_coarseningMap.end()) {
      m_coarseningMap.insert(std::pair<Instruction *, InstVector>(alive, coarsenedInsts)); 
    }
  }
  
  for (auto &mapIter : aliveMap) {
    Instruction *alive = mapIter.first;
    InstVector &coarsenedInsts = mapIter.second;

    updatePlaceholderMap(alive, coarsenedInsts);
  }
}

void CUDACoarseningPass::replicateRegionImpl(DivergentRegion *region,
                                             CoarseningMap&   aliveMap)
{
    BasicBlock *pred = getPredecessor(region, m_loopInfo);
    BasicBlock *topInsertionPoint = pred; // region->getExiting()
    BasicBlock *bottomInsertionPoint = region->getHeader(); //getExit(*region);
    // Replicate the region.
    for (unsigned int index = 0; index < m_factor - 1; ++index) {

        Map valueMap;
        DivergentRegion *newRegion =
            region->clone(".cf" + Twine(index + 2), m_domT, valueMap);

        std::vector<std::pair<Instruction *, InstVector> > toAdd;

        // BUGFIX: Also extend coarsening map with new aliases.
        std::for_each(m_coarseningMap.begin(),
                      m_coarseningMap.end(),
          [&](std::pair<Instruction*, InstVector> p) {
              std::string name = p.first->getName();
              name.append(".cf" + std::to_string(index + 2));

              for (BasicBlock *BB : *newRegion) {
                for (Instruction& I : *BB) {
                  if (I.getName() == name) {
                    InstVector tmp;
                  
                    for (Instruction *pI : p.second) {
                      std::string _name = pI->getName();
                      _name.append(".cf" + std::to_string(index + 2));

                      for (BasicBlock *pBB : *newRegion) {
                        for (Instruction& cI : *pBB) {
                          if (cI.getName() == _name) {
                            tmp.push_back(&cI);
                          }
                        }
                      }
                    }
                    errs() << "Adding mapping from ";
                    I.dump();
                    for (Instruction *k : tmp) {
                      k->dump();
                    }
                    errs() << "====\n";
                    toAdd.push_back(std::pair<Instruction*, InstVector>(&I, tmp));
                  }
                }
              }
          });

        for (auto& ta : toAdd) {
          m_coarseningMap.insert(ta);
        }

        for (auto& sh : m_coarseningMap) {
          sh.first->dump();
        }
        
        applyCoarseningMap(*newRegion, index);

        // Connect the region to the CFG.
        Util::changeBlockTarget(topInsertionPoint, newRegion->getHeader());
        Util::changeBlockTarget(newRegion->getExiting(), bottomInsertionPoint);

        // Update the phi nodes of the newly inserted header.
        //Util::remapBlocksInPHIs(newRegion->getHeader(), pred, topInsertionPoint);
        // Update the phi nodes in the exit block.
       // Util::remapBlocksInPHIs(bottomInsertionPoint, topInsertionPoint,
       //                         newRegion->getExiting());

        //topInsertionPoint = newRegion->getHeader();
        bottomInsertionPoint = newRegion->getHeader(); //getExit(*newRegion);

        delete newRegion;
        updateAliveMap(aliveMap, valueMap);
    }
}