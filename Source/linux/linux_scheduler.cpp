#include "linux_scheduler.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

static uint64_t __calcDelta(uint64_t deltaExec, uint64_t weight,
                            uint64_t inverseWeight) {
  uint64_t fairnessCt = weight * inverseWeight;

  return (deltaExec * fairnessCt) >>
         os_simulation_linux_scheduler_math::WMULT_SHIFT;
}

/**
 * @brief Converts a 1-10 priority scale to Linux's -20 to 19 nice value scale.
 * * This function uses linear interpolation to map a custom priority range
 * [1, 10] to the standard Linux Completely Fair Scheduler (CFS) nice
 * value range [-20, 19].
 * * The mapping assumes:
 * - Priority 1 (highest) maps to Nice -20 (maximum CPU share)
 * - Priority 10 (lowest) maps to Nice 19 (minimum CPU share)
 * * The mathematical formula used is: N = -20 + ((P - 1) * 39) / 9
 * * @param priority The raw priority value, expected to be in the range [1,
 * 10]. Values outside this range are safely clamped.
 * @return int The corresponding Linux nice value in the range [-20, 19].
 */
static int __convPriorityToNice(int priority) {
  return -20 + (std::clamp(priority, 1, 10) - 1) * 39 / 9;
}

namespace os_simulation_linux_scheduler {
// ===== STATIC METHODS =====

uint64_t LinuxCfsScheduler::calcDelta(uint64_t deltaExec, uint64_t weight,
                                      uint64_t inverseWeight) {
  if (weight != os_simulation_linux_scheduler_math::NICE_0_LOAD) [[unlikely]] {
    deltaExec =
        __calcDelta(deltaExec, os_simulation_linux_scheduler_math::NICE_0_LOAD,
                    inverseWeight);
  }

  return deltaExec;
}

// ===== LINUXCFS SCHEDULER METHODS =====

CfsNode::CfsNode(const os_simulation_process::Process &rProcess)
    : mProcess(rProcess) {
  int niceValue = __convPriorityToNice(rProcess.getPriority());
  int arrIndex = std::clamp(niceValue + 20, 0, 39);

  mWeight = os_simulation_linux_scheduler_math::SCHED_PRIO_TO_WEIGHT[arrIndex];
  mInverseWeight =
      os_simulation_linux_scheduler_math::SCHED_PRIO_TO_WMULT[arrIndex];
}

void LinuxCfsScheduler::addProcess(
    const os_simulation_process::Process &rProcess) {
  mAllNodes.emplace_back(rProcess);

  mAllNodes.back().mVruntime = mAllNodes.back().mInverseWeight;
}

void LinuxCfsScheduler::readyProcess(uint16_t processId) {
  for (auto &rNode : mAllNodes) {
    if (rNode.mProcess.getId() == processId) {
      rNode.mProcess.markReady();
      mRunQueue.insert(&rNode, rNode.mVruntime);

      return;
    }
  }
}

uint64_t LinuxCfsScheduler::getPreemptionDelay(
    os_simulation_process::Process *pProcess) {
  uint64_t totalWeight = 0;
  size_t runnableCount = 0;

  for (const auto &node : mAllNodes) {
    if (node.mProcess.getState() ==
            os_simulation_process::ProcessState::READY ||
        node.mProcess.getState() ==
            os_simulation_process::ProcessState::RUNNING ||
        (pProcess && node.mProcess.getId() == pProcess->getId())) {
      totalWeight += node.mWeight;
      runnableCount++;
    }
  }

  if (runnableCount == 0) {
    return 0;
  }

  if (runnableCount == 1) {
    return pProcess->getRemainingTime();
  }

  const uint64_t TARGET_LATENCY = 20;
  const uint64_t MIN_GRANULARITY = 4;

  uint64_t taskWeight = os_simulation_linux_scheduler_math::NICE_0_LOAD;

  for (const auto &node : mAllNodes) {
    if (pProcess && node.mProcess.getId() == pProcess->getId()) {
      taskWeight = node.mWeight;

      break;
    }
  }

  uint64_t slice = (TARGET_LATENCY * taskWeight) / totalWeight;

  return std::max(slice, MIN_GRANULARITY);
}

os_simulation_process::Process *LinuxCfsScheduler::getProcess(
    uint16_t processId) {
  for (auto &rNode : mAllNodes) {
    if (rNode.mProcess.getId() == processId) {
      return &rNode.mProcess;
    }
  }

  return nullptr;
}

os_simulation_process::Process *LinuxCfsScheduler::getNextProcessToRun() {
  CfsNode *pNextNode = mRunQueue.extractMin();

  if (pNextNode != nullptr) {
    return &(pNextNode->mProcess);
  }

  return nullptr;
}

void LinuxCfsScheduler::updateProcessExecution(
    os_simulation_process::Process *pProcess, uint64_t executedTicks,
    uint64_t currentTime) {
  for (auto &rNode : mAllNodes) {
    if (rNode.mProcess.getId() == pProcess->getId()) {
      pProcess->executeTicks(executedTicks, currentTime);

      // 1 simulated tick approximates 1,000,000 nanoseconds for standard CFS
      // scaling
      uint64_t deltaExecNs = executedTicks * 1'000'000;

      rNode.mVruntime +=
          calcDelta(deltaExecNs, rNode.mWeight, rNode.mInverseWeight);

      return;
    }
  }
}

void LinuxCfsScheduler::preemptProcess(
    os_simulation_process::Process *pProcess) {
  for (auto &rNode : mAllNodes) {
    if (rNode.mProcess.getId() == pProcess->getId()) {
      pProcess->preempt();
      mRunQueue.insert(&rNode, rNode.mVruntime);

      return;
    }
  }
}

bool LinuxCfsScheduler::isFinished() const {
  for (const auto &rNode : mAllNodes) {
    if (!rNode.mProcess.isFinished()) {
      return false;
    }
  }

  return true;
}

const os_simulation_process::Process &LinuxCfsScheduler::getProcessConst(
    uint16_t processId) const {
  for (const auto &rNode : mAllNodes) {
    if (rNode.mProcess.getId() == processId) {
      return rNode.mProcess;
    }
  }

  throw std::runtime_error("Process with ID " + std::to_string(processId) +
                           " not found in scheduler");
}

// ===== RED-BLACK TREE METHODS =====

RedBlackTree::RedBlackTree() {
  mNil = new RbNode();
  mNil->mColor = RbColor::BLACK;
  mRoot = mNil;
}

RedBlackTree::~RedBlackTree() {
  destroyTree(mRoot);
  delete mNil;
}

void RedBlackTree::destroyTree(RbNode *pNode) {
  if (pNode != mNil) {
    destroyTree(pNode->pLeft);
    destroyTree(pNode->pRight);
    delete pNode;
  }
}

void RedBlackTree::rotateLeft(RbNode *pNode) {
  RbNode *pRightChild = pNode->pRight;
  pNode->pRight = pRightChild->pLeft;

  if (pRightChild->pLeft != mNil) {
    pRightChild->pLeft->pParent = pNode;
  }

  pRightChild->pParent = pNode->pParent;

  if (pNode->pParent == mNil) {
    mRoot = pRightChild;
  } else if (pNode == pNode->pParent->pLeft) {
    pNode->pParent->pLeft = pRightChild;
  } else {
    pNode->pParent->pRight = pRightChild;
  }

  pRightChild->pLeft = pNode;
  pNode->pParent = pRightChild;
}

void RedBlackTree::rotateRight(RbNode *pNode) {
  RbNode *pLeftChild = pNode->pLeft;
  pNode->pLeft = pLeftChild->pRight;

  if (pLeftChild->pRight != mNil) {
    pLeftChild->pRight->pParent = pNode;
  }

  pLeftChild->pParent = pNode->pParent;

  if (pNode->pParent == mNil) {
    mRoot = pLeftChild;
  } else if (pNode == pNode->pParent->pRight) {
    pNode->pParent->pRight = pLeftChild;
  } else {
    pNode->pParent->pLeft = pLeftChild;
  }

  pLeftChild->pRight = pNode;
  pNode->pParent = pLeftChild;
}

void RedBlackTree::insert(CfsNode *pCfsNode, uint64_t vruntime) {
  RbNode *pNewNode = new RbNode();
  pNewNode->pCfsNode = pCfsNode;
  pNewNode->mVruntime = vruntime;
  pNewNode->pLeft = mNil;
  pNewNode->pRight = mNil;
  pNewNode->mColor = RbColor::RED;

  RbNode *pParent = mNil;
  RbNode *pCurrent = mRoot;

  while (pCurrent != mNil) {
    pParent = pCurrent;
    if (pNewNode->mVruntime < pCurrent->mVruntime) {
      pCurrent = pCurrent->pLeft;
    } else {
      pCurrent = pCurrent->pRight;
    }
  }

  pNewNode->pParent = pParent;

  if (pParent == mNil) {
    mRoot = pNewNode;
  } else if (pNewNode->mVruntime < pParent->mVruntime) {
    pParent->pLeft = pNewNode;
  } else {
    pParent->pRight = pNewNode;
  }

  insertFixup(pNewNode);
}

void RedBlackTree::insertFixup(RbNode *pNode) {
  while (pNode->pParent->mColor == RbColor::RED) {
    if (pNode->pParent == pNode->pParent->pParent->pLeft) {
      RbNode *pUncle = pNode->pParent->pParent->pRight;

      if (pUncle->mColor == RbColor::RED) {
        pNode->pParent->mColor = RbColor::BLACK;
        pUncle->mColor = RbColor::BLACK;
        pNode->pParent->pParent->mColor = RbColor::RED;
        pNode = pNode->pParent->pParent;
      } else {
        if (pNode == pNode->pParent->pRight) {
          pNode = pNode->pParent;
          rotateLeft(pNode);
        }
        pNode->pParent->mColor = RbColor::BLACK;
        pNode->pParent->pParent->mColor = RbColor::RED;
        rotateRight(pNode->pParent->pParent);
      }
    } else {
      RbNode *pUncle = pNode->pParent->pParent->pLeft;

      if (pUncle->mColor == RbColor::RED) {
        pNode->pParent->mColor = RbColor::BLACK;
        pUncle->mColor = RbColor::BLACK;
        pNode->pParent->pParent->mColor = RbColor::RED;
        pNode = pNode->pParent->pParent;
      } else {
        if (pNode == pNode->pParent->pLeft) {
          pNode = pNode->pParent;
          rotateRight(pNode);
        }
        pNode->pParent->mColor = RbColor::BLACK;
        pNode->pParent->pParent->mColor = RbColor::RED;
        rotateLeft(pNode->pParent->pParent);
      }
    }
  }
  mRoot->mColor = RbColor::BLACK;
}

void RedBlackTree::transplant(RbNode *pTarget, RbNode *pReplacement) {
  if (pTarget->pParent == mNil) {
    mRoot = pReplacement;
  } else if (pTarget == pTarget->pParent->pLeft) {
    pTarget->pParent->pLeft = pReplacement;
  } else {
    pTarget->pParent->pRight = pReplacement;
  }
  pReplacement->pParent = pTarget->pParent;
}

RbNode *RedBlackTree::getMinimum(RbNode *pNode) const {
  while (pNode->pLeft != mNil) {
    pNode = pNode->pLeft;
  }
  return pNode;
}

CfsNode *RedBlackTree::extractMin() {
  if (mRoot == mNil) {
    return nullptr;
  }

  RbNode *pMinNode = getMinimum(mRoot);
  CfsNode *pExtractedCfsNode = pMinNode->pCfsNode;

  RbNode *pNodeX;
  RbNode *pNodeY = pMinNode;
  RbColor originalColor = pNodeY->mColor;

  if (pMinNode->pLeft == mNil) {
    pNodeX = pMinNode->pRight;
    transplant(pMinNode, pMinNode->pRight);
  } else if (pMinNode->pRight == mNil) {
    pNodeX = pMinNode->pLeft;
    transplant(pMinNode, pMinNode->pLeft);
  } else {
    pNodeY = getMinimum(pMinNode->pRight);
    originalColor = pNodeY->mColor;
    pNodeX = pNodeY->pRight;

    if (pNodeY->pParent == pMinNode) {
      pNodeX->pParent = pNodeY;
    } else {
      transplant(pNodeY, pNodeY->pRight);
      pNodeY->pRight = pMinNode->pRight;
      pNodeY->pRight->pParent = pNodeY;
    }

    transplant(pMinNode, pNodeY);
    pNodeY->pLeft = pMinNode->pLeft;
    pNodeY->pLeft->pParent = pNodeY;
    pNodeY->mColor = pMinNode->mColor;
  }

  if (originalColor == RbColor::BLACK) {
    removeFixup(pNodeX);
  }

  delete pMinNode;
  return pExtractedCfsNode;
}

void RedBlackTree::removeFixup(RbNode *pNode) {
  while (pNode != mRoot && pNode->mColor == RbColor::BLACK) {
    if (pNode == pNode->pParent->pLeft) {
      RbNode *pSibling = pNode->pParent->pRight;

      if (pSibling->mColor == RbColor::RED) {
        pSibling->mColor = RbColor::BLACK;
        pNode->pParent->mColor = RbColor::RED;

        rotateLeft(pNode->pParent);

        pSibling = pNode->pParent->pRight;
      }

      if (pSibling->pLeft->mColor == RbColor::BLACK &&
          pSibling->pRight->mColor == RbColor::BLACK) {
        pSibling->mColor = RbColor::RED;
        pNode = pNode->pParent;
      } else {
        if (pSibling->pRight->mColor == RbColor::BLACK) {
          pSibling->pLeft->mColor = RbColor::BLACK;
          pSibling->mColor = RbColor::RED;

          rotateRight(pSibling);

          pSibling = pNode->pParent->pRight;
        }
        pSibling->mColor = pNode->pParent->mColor;
        pNode->pParent->mColor = RbColor::BLACK;
        pSibling->pRight->mColor = RbColor::BLACK;

        rotateLeft(pNode->pParent);

        pNode = mRoot;
      }
    } else {
      RbNode *pSibling = pNode->pParent->pLeft;

      if (pSibling->mColor == RbColor::RED) {
        pSibling->mColor = RbColor::BLACK;
        pNode->pParent->mColor = RbColor::RED;

        rotateRight(pNode->pParent);

        pSibling = pNode->pParent->pLeft;
      }

      if (pSibling->pRight->mColor == RbColor::BLACK &&
          pSibling->pLeft->mColor == RbColor::BLACK) {
        pSibling->mColor = RbColor::RED;
        pNode = pNode->pParent;
      } else {
        if (pSibling->pLeft->mColor == RbColor::BLACK) {
          pSibling->pRight->mColor = RbColor::BLACK;
          pSibling->mColor = RbColor::RED;

          rotateLeft(pSibling);

          pSibling = pNode->pParent->pLeft;
        }
        pSibling->mColor = pNode->pParent->mColor;
        pNode->pParent->mColor = RbColor::BLACK;
        pSibling->pLeft->mColor = RbColor::BLACK;

        rotateRight(pNode->pParent);

        pNode = mRoot;
      }
    }
  }

  pNode->mColor = RbColor::BLACK;
}

}  // namespace os_simulation_linux_scheduler
