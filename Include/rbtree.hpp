#pragma once

#include <cstdio>
#include <stdexcept>
#include <string>

using namespace std;

namespace os_simulator {
enum class RbTreeNodeColor { RED, BLACK };

inline const string to_string(RbTreeNodeColor &rColor) {
  switch (rColor) {
    case RbTreeNodeColor::BLACK:
      return "BLACK";
    case RbTreeNodeColor::RED:
      return "RED";
  }
}

template <typename T>
struct RbTreeNode {
  T data;
  RbTreeNodeColor color;
  RbTreeNode *pParent;
  RbTreeNode *pLeftChild;
  RbTreeNode *pRightChild;

  RbTreeNode(T value)
      : data(value),
        color(RbTreeNodeColor::RED),
        pParent(nullptr),
        pLeftChild(nullptr),
        pRightChild(nullptr) {}
};

template <typename T>
class RbTree {
 private:
  RbTreeNode<T> *pMRootNode;
  RbTreeNode<T> *pMEmptyLeaf;

  void destroyTree(RbTreeNode<T> *pNode) {
    if (pNode != pMEmptyLeaf) {
      destroyTree(pNode->pLeftChild);
      destroyTree(pNode->pRightChild);

      delete pNode;
    }
  }

  /**
   * @brief Rotates a portion of the tree to the left.
   * * Imagine grabbing the node and pulling it down and to the left.
   * Its right child moves up to take its place.
   */
  void rotateLeft(RbTreeNode<T> *pNode) {
    RbTreeNode<T> *pRightChild = pNode->pRightChild;
    pNode->pRightChild = pRightChild->pLeftChild;

    if (pRightChild->pLeftChild != pMEmptyLeaf) {
      pRightChild->pLeftChild->pParent = pNode;
    }

    pRightChild->pParent = pNode->pParent;

    if (pNode->pParent == nullptr) {
      this->pMRootNode = pRightChild;
    } else if (pNode == pNode->pParent->pLeftChild) {
      pNode->pParent->pLeftChild = pRightChild;
    } else {
      pNode->pParent->pRightChild = pRightChild;
    }

    pRightChild->pLeftChild = pNode;
    pNode->pParent = pRightChild;
  }

  /**
   * @brief Rotates a portion of the tree to the right.
   * * Imagine grabbing the node and pulling it down and to the right.
   * Its left child moves up to take its place.
   */
  void rotateRight(RbTreeNode<T> *pNode) {
    RbTreeNode<T> *pLeftChild = pNode->pLeftChild;
    pNode->pLeftChild = pLeftChild->pRightChild;

    if (pLeftChild->pRightChild != pMEmptyLeaf) {
      pLeftChild->pRightChild->pParent = pNode;
    }

    pLeftChild->pParent = pNode->pParent;

    if (pNode->pParent == nullptr) {
      this->pMRootNode = pLeftChild;
    } else if (pNode == pNode->pParent->pRightChild) {
      pNode->pParent->pRightChild = pLeftChild;
    } else {
      pNode->pParent->pLeftChild = pLeftChild;
    }

    pLeftChild->pRightChild = pNode;
    pNode->pParent = pLeftChild;
  }

  void balanceAfterInsertion(RbTreeNode<T> *pNewNode) {
    RbTreeNode<T> *pUncleNode;

    while (pNewNode->pParent != nullptr &&
           pNewNode->pParent->color == RbTreeNodeColor::RED) {
      RbTreeNode<T> *pGrandparentNode = pNewNode->pParent->pParent;

      if (pNewNode->pParent == pGrandparentNode->pLeftChild) {
        pUncleNode = pGrandparentNode->pRightChild;

        switch (pUncleNode->color) {
          case RbTreeNodeColor::RED: {
            pNewNode->pParent->color = RbTreeNodeColor::BLACK;
            pUncleNode->color = RbTreeNodeColor::BLACK;
            pGrandparentNode->color = RbTreeNodeColor::RED;
            pNewNode = pGrandparentNode;

            break;
          }
          case RbTreeNodeColor::BLACK: {
            if (pNewNode == pNewNode->pParent->pRightChild) {
              pNewNode = pNewNode->pParent;

              rotateLeft(pNewNode);
            }

            pNewNode->pParent->color = RbTreeNodeColor::BLACK;
            pGrandparentNode->color = RbTreeNodeColor::RED;

            rotateRight(pGrandparentNode);

            break;
          }
        }
      } else {
        pUncleNode = pGrandparentNode->pLeftChild;

        switch (pUncleNode->color) {
          case RbTreeNodeColor::RED: {
            pNewNode->pParent->color = RbTreeNodeColor::BLACK;
            pUncleNode->color = RbTreeNodeColor::BLACK;
            pGrandparentNode->color = RbTreeNodeColor::RED;
            pNewNode = pGrandparentNode;

            break;
          }
          case RbTreeNodeColor::BLACK: {
            if (pNewNode == pNewNode->pParent->pLeftChild) {
              pNewNode = pNewNode->pParent;

              rotateRight(pNewNode);
            }

            pNewNode->pParent->color = RbTreeNodeColor::BLACK;
            pGrandparentNode->color = RbTreeNodeColor::RED;

            break;
          }
        }
      }

      if (pNewNode == pMRootNode) {
        break;
      }
    }

    pMRootNode->color = RbTreeNodeColor::BLACK;
  }

  /**
   * @brief Fixes the tree's colors and structure after a BLACK node is deleted.
   * * Deleting a black node reduces the "black height" of a path, violating
   * RB-Tree rules. We look at the node's sibling to figure out how to rotate
   * and recolor our way back to balance.
   */
  void balanceTreeAfterDeletion(RbTreeNode<T> *pCurrentNode) {
    while (pCurrentNode != pMRootNode &&
           pCurrentNode->color == RbTreeNodeColor::BLACK) {
      if (pCurrentNode == pCurrentNode->pParent->pLeftChild) {
        RbTreeNode<T> *pSiblingNode = pCurrentNode->pParent->pRightChild;

        if (pSiblingNode->color == RbTreeNodeColor::RED) {
          pSiblingNode->color = RbTreeNodeColor::BLACK;
          pCurrentNode->pParent->color = RbTreeNodeColor::RED;

          rotateLeft(pCurrentNode->pParent);

          pSiblingNode = pCurrentNode->pParent->pRightChild;
        }

        if (pSiblingNode->pLeftChild->color == RbTreeNodeColor::BLACK &&
            pSiblingNode->pRightChild->color == RbTreeNodeColor::BLACK) {
          pSiblingNode->color = RbTreeNodeColor::RED;
          pCurrentNode = pCurrentNode->pParent;
        } else {
          if (pSiblingNode->pRightChild->color == RbTreeNodeColor::BLACK) {
            pSiblingNode->pLeftChild->color = RbTreeNodeColor::BLACK;
            pSiblingNode->color = RbTreeNodeColor::RED;

            rotateRight(pSiblingNode);

            pSiblingNode = pCurrentNode->pParent->pRightChild;
          }

          pSiblingNode->color = pCurrentNode->pParent->color;
          pCurrentNode->pParent->color = RbTreeNodeColor::BLACK;
          pSiblingNode->pRightChild->color = RbTreeNodeColor::BLACK;

          rotateLeft(pCurrentNode->pParent);

          pCurrentNode = pMRootNode;
        }
      } else {
        RbTreeNode<T> *pSiblingNode = pCurrentNode->pParent->pLeftChild;

        if (pSiblingNode->color == RbTreeNodeColor::RED) {
          pSiblingNode->color = RbTreeNodeColor::BLACK;
          pCurrentNode->pParent->color = RbTreeNodeColor::RED;

          rotateRight(pCurrentNode->pParent);

          pSiblingNode = pCurrentNode->pParent->pLeftChild;
        }

        if (pSiblingNode->pRightChild->color == RbTreeNodeColor::BLACK &&
            pSiblingNode->pLeftChild->color == RbTreeNodeColor::BLACK) {
          pSiblingNode->color = RbTreeNodeColor::RED;
          pCurrentNode = pCurrentNode->pParent;
        } else {
          if (pSiblingNode->pLeftChild->color == RbTreeNodeColor::BLACK) {
            pSiblingNode->pRightChild->color = RbTreeNodeColor::BLACK;
            pSiblingNode->color = RbTreeNodeColor::RED;

            rotateLeft(pSiblingNode);

            pSiblingNode = pCurrentNode->pParent->pLeftChild;
          }

          pSiblingNode->color = pCurrentNode->pParent->color;
          pCurrentNode->pParent->color = RbTreeNodeColor::BLACK;
          pSiblingNode->pLeftChild->color = RbTreeNodeColor::BLACK;

          rotateRight(pCurrentNode->pParent);

          pCurrentNode = pMRootNode;
        }
      }
    }

    pCurrentNode->color = RbTreeNodeColor::BLACK;
  }

  /**
   * @brief Replaces one subtree as a child of its parent with another subtree.
   * * Think of this as unhooking a node from its parent and hooking up a
   * different node in its place.
   */
  void replaceNodeInParent(RbTreeNode<T> *pOutNode, RbTreeNode<T> *pInNode) {
    if (pOutNode->pParent == nullptr) {
      this->pMRootNode = pInNode;
    } else if (pOutNode == pOutNode->pParent->pLeftChild) {
      pOutNode->pParent->pLeftChild = pInNode;
    } else {
      pOutNode->pParent->pRightChild = pInNode;
    }

    pInNode->pParent = pOutNode->pParent;
  }

  /**
   * @brief Finds the smallest value in a given subtree (always the furthest
   * left).
   */
  RbTreeNode<T> *findMinimumNode(RbTreeNode<T> *pNode) {
    while (pNode->pLeftChild != pMEmptyLeaf) {
      pNode = pNode->pLeftChild;
    }

    return pNode;
  }

  void orint(RbTreeNode<T> pNode) const {
    if (pNode != pMEmptyLeaf) {
      printInOrderHelper(pNode->pLeftChild);

      std::string colorString = to_string(pNode->color);

      printf("%d (%s)\n", pNode->data, colorString.c_str());

      printInOrderHelper(pNode->rightChild);
    }
  }

 public:
  /**
   * @brief Creates a new, empty Red-Black Tree.
   */
  RbTree() {
    pMEmptyLeaf = new RbTreeNode<T>(T());
    pMEmptyLeaf->color = RbTreeNodeColor::BLACK;
    pMEmptyLeaf->pLeftChild = nullptr;
    pMEmptyLeaf->pRightChild = nullptr;
    pMEmptyLeaf->pParent = nullptr;

    pMRootNode = pMEmptyLeaf;
  }

  /**
   * @brief Cleans up memory when the tree goes out of scope.
   */
  ~RbTree() {
    destroyTree(pMRootNode);

    delete pMEmptyLeaf;
  }

  void insert(T value) {
    RbTreeNode<T> *pNewNode = new RbTreeNode<T>(value);

    pNewNode->pParent = nullptr;
    pNewNode->pLeftChild = pMEmptyLeaf;
    pNewNode->pRightChild = pMEmptyLeaf;
    pNewNode->color = RbTreeNodeColor::RED;

    RbTreeNode<T> *pCurrentParentNode = nullptr;
    RbTreeNode<T> *pCurrentNode = this->pMRootNode;

    while (pCurrentNode != pMEmptyLeaf) {
      pCurrentParentNode = pCurrentNode;

      if (pNewNode->data < pCurrentNode->data) {
        pCurrentNode = pCurrentNode->pLeftChild;
      } else {
        pCurrentNode = pCurrentNode->pRightChild;
      }
    }

    pNewNode->pParent = pCurrentParentNode;

    if (pCurrentParentNode == nullptr) {
      pMRootNode = pNewNode;
    } else if (pNewNode->data < pCurrentParentNode->data) {
      pCurrentParentNode->pLeftChild = pNewNode;
    } else {
      pCurrentParentNode->pRightChild = pNewNode;
    }

    if (pNewNode->pParent == nullptr) {
      pNewNode->color = RbTreeNodeColor::BLACK;

      return;
    }

    if (pNewNode->pParent->pParent == nullptr) {
      return;
    }

    balanceAfterInsertion(pNewNode);
  }

  void removeNode(RbTreeNode<T> *pOutNode) {
    if (pOutNode == pMEmptyLeaf) {
      return;
    }

    RbTreeNode<T> *pReplacementNode;
    RbTreeNode<T> *pMoveOrOutNode = pOutNode;
    RbTreeNodeColor originalColor = pMoveOrOutNode->color;

    if (pOutNode->pLeftChild == pMEmptyLeaf) {
      pReplacementNode = pOutNode->pRightChild;

      replaceNodeInParent(pOutNode, pOutNode->pRightChild);
    } else if (pOutNode->pRightChild == pMEmptyLeaf) {
      pReplacementNode = pOutNode->pLeftChild;

      replaceNodeInParent(pOutNode, pOutNode->pLeftChild);
    } else {
      pMoveOrOutNode = findMinimumNode(pOutNode->pRightChild);
      originalColor = pMoveOrOutNode->color;
      pReplacementNode = pMoveOrOutNode->pRightChild;

      if (pMoveOrOutNode->pParent == pOutNode) {
        pReplacementNode->pParent = pMoveOrOutNode;
      } else {
        replaceNodeInParent(pMoveOrOutNode, pMoveOrOutNode->pRightChild);

        pMoveOrOutNode->pRightChild = pOutNode->pRightChild;
        pMoveOrOutNode->pRightChild->pParent = pMoveOrOutNode;
      }

      replaceNodeInParent(pOutNode, pMoveOrOutNode);

      pMoveOrOutNode->pLeftChild = pOutNode->pLeftChild;
      pMoveOrOutNode->pLeftChild->pParent = pMoveOrOutNode;
      pMoveOrOutNode->color = pOutNode->color;
    }

    delete pOutNode;

    if (originalColor == RbTreeNodeColor::BLACK) {
      balanceTreeAfterDeletion(pReplacementNode);
    }
  }

  /**
   * @brief CFS specific: Finds the node with the absolute lowest value,
   * removes it from the tree, and returns its data.
   */
  T extractMinimum() {
    if (pMRootNode == pMEmptyLeaf) {
      throw std::runtime_error("Cannot extract minimum: The tree is empty.");
    }

    RbTreeNode<T> *pMinNode = findMinimumNode(pMRootNode);
    T minValue = pMinNode->data;

    removeNode(pMinNode);

    return minValue;
  }
};
}  // namespace os_simulator
