#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <memory>
#include <ctime>
#include <climits>
#include <cstdlib>

using std::vector;
using std::string;
using kvp = std::pair<string, int>;
using std::unique_ptr;

/*********************************************************************/
/* C++ kd-tree hashmap implementation
 *********************************************************************/

int choosePivot(int left, int right) {
    return left + rand() % (right - left + 1);
}

int partition(vector<kvp>& arr, int p, int left, int right, bool axis) {
  if (axis) {
    swap(arr[p], arr[right]);
    std::string v = arr[right].first;
    int j = left;
    int i = left;
    while (i < right) {
        if (arr[i].first < v) {
            swap(arr[i], arr[j]);
            j += 1;
        }
        i += 1;
    }
    swap(arr[j], arr[right]);
    return j;
  } else {
    swap(arr[p], arr[right]);
    int v = arr[right].second;
    int j = left;
    int i = left;
    while (i < right) {
        if (arr[i].second < v) {
            swap(arr[i], arr[j]);
            j += 1;
        }
        i += 1;
    }
    swap(arr[j], arr[right]);
    return j;
  }
}

kvp quickSelect(vector<kvp>& arr, int k, int left, int right, bool axis) {
    if (left == right) {
        return arr[left];
    }
    
    int p = choosePivot(left, right);
    int i = partition(arr, p, left, right, axis);
    if (k == i) {
        return arr[k];
    } else if (i > k) {
        return quickSelect(arr, k, left, i - 1, axis);
    } else {
        return quickSelect(arr, k, i + 1, right, axis);
    }
}

bool kvpInBox(kvp point, kvp start, kvp end) {
  if (point.first >= start.first && point.first < end.first) {
    if (point.second >= start.second && point.second < end.second) {
      return true;
    }
  }
  return false;
}

class KDMap {
    struct kdNode {
        kvp point;
        unique_ptr<kdNode> left;
        unique_ptr<kdNode> right;
        bool axis;
        kdNode(kvp p, bool d) : point{p}, axis{d} { 
            left = nullptr;
            right = nullptr; 
        }
    };

  private:
    unique_ptr<kdNode> root;

    unique_ptr<kdNode> buildTree(vector<kvp>& points, int left, int right, bool axis) {
        if (left == right) {
            return unique_ptr<kdNode>(new kdNode(points[right], axis));
        } else if (left > right) {
          return nullptr;
        }
        int median_index = ((left + right + 1) / 2);
        kvp median_item = quickSelect(points, median_index, left, right, axis);
        unique_ptr<kdNode> node = unique_ptr<kdNode>(new kdNode(median_item, axis));

        node->left = buildTree(points, left, median_index - 1, !axis);
        node->right = buildTree(points, median_index, right, !axis);

        return node;
    }

    void traverse(unique_ptr<kdNode>& node, vector<kvp>& kvps) {
        if (node == nullptr) {
            return;
        }
        if (node->left == nullptr && node->right == nullptr) {
            kvps.push_back(node->point);
        }
        traverse(node->left, kvps);
        traverse(node->right, kvps);
    }

    std::optional<int> search(unique_ptr<kdNode>& node, string key) { 
        if (node == nullptr) {
            return std::nullopt;
        }

        if (node->point.first == key) {
            return node->point.second;
        }

        if (node->axis) {
            if (key < node->point.first) {
                return search(node->left, key);
            } else {
                return search(node->right, key);
            }
        } else {
            std::optional<int> l = search(node->left, key);
            std::optional<int> r = search(node->right, key);
            if (l) {
                return l;
            } else if (r) {
                return r;
            } else {
                return std::nullopt;
            }
        }
    }

    void findKeySplits(vector<string>& splits, unique_ptr<kdNode>& node) {
        if (node == nullptr) {
            return;
        }
        if (node->left == nullptr && node->right == nullptr) {
          return;
        }
        if (node->axis == true) {
            splits.push_back(node->point.first);
        }
        findKeySplits(splits, node->left);
        findKeySplits(splits, node->right);
    }

    void findValueSplits(vector<int>& splits, unique_ptr<kdNode>& node) {
        if (node == nullptr) {
            return;
        }
        if (node->left == nullptr && node->right == nullptr) {
          return;
        }
        if (node->axis == false) {
            splits.push_back(node->point.second);
        }
        findValueSplits(splits, node->left);
        findValueSplits(splits, node->right);
    }

    void rangeSearch(unique_ptr<kdNode>& root, kvp start, kvp end, vector<kvp>& kvps) {
      if (root == nullptr) {
        return;
      }
      if (root->left == nullptr && root->right == nullptr) {
        if (kvpInBox(root->point, start, end)) {
          kvps.push_back(root->point);
        }
        return;
      }
      if (root->axis == true) {
        if (root->point.first >= start.first && root->point.first < end.first) {
          rangeSearch(root->left, start, end, kvps);
          rangeSearch(root->right, start, end, kvps);
        } else if (root->point.first < start.first) {
          rangeSearch(root->right, start, end, kvps);
        } else if (root->point.first >= end.first) {
          rangeSearch(root->left, start, end, kvps);
        }
      } else {
        if (root->point.second >= start.second && root->point.second < end.second) {
          rangeSearch(root->left, start, end, kvps);
          rangeSearch(root->right, start, end, kvps);
        } else if (root->point.second < start.second) {
          rangeSearch(root->right, start, end, kvps);
        } else if (root->point.second >= end.second) {
          rangeSearch(root->left, start, end, kvps);
        }
      }
    }

  public:
    // Constructor: Builds a new tree given a vector of <string, int> key-value pairs.
    //              O(nlogn)
    KDMap(const vector<kvp>& kvps) {
        vector<kvp> points;
        for (auto kvp : kvps) {
            points.push_back(kvp);
        }
        root = nullptr;
        root = buildTree(points, 0, kvps.size() - 1, true);
    }

    // all_points: Returns all KVPs in the tree
    vector<kvp> all_points() {
      vector<kvp> kvps;
      traverse(root, kvps);
      return kvps;
    }

    // get: Given a key, returns the value or std::nullopt if the key is not in the tree.
    //      O(sqrt(n))
    std::optional<int> get(string key) {
        return search(root, key);
    }

    // range: Performs range search for a given rectangle defined by two key-value pairs.
    //        returns all points in the range
    //              [key_start, key_end) x [value_start, value_end)
    //        O(sqrt(n)) time
    vector<kvp> range(kvp start, kvp end) {
      vector<kvp> found;
      rangeSearch(root, start, end, found);
      return found;
    }

    // key_splits: Return all keys (strings) that are used for splits,
    //             O(n)
    vector<string> key_splits() {
        vector<string> splits;
        findKeySplits(splits, root);
        return splits;
    }

    // value_splits: Return all values (ints) that are used for splits,
    //               O(n)
    vector<int> value_splits() {
        vector<int> splits;
        findValueSplits(splits, root);
        return splits;
    }

    // Destructor
    ~KDMap() {}
};
