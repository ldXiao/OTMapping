
#ifndef BCCLEAN_PROJ_NODE_H
#define BCCLEAN_PROJ_NODE_H
#include <vector>
#include <Eigen/Core>
#include <map>
namespace bcclean {
    void proj_node(
        const Eigen::MatrixXd & Vbad,
        const Eigen::MatrixXi & Fbad,
        const std::vector<int> & node_list_bad,
        Eigen::MatrixXd & Vgood,
        Eigen::MatrixXi & Fgood,
        std::map<int, int> & node_map
    );
    // try to project the nodes into good mesh and modify the good mesh correspondingly
    // if it can not find intersection use nearest neighbor instead 
    // always gaurantee that node_map is injective
}
#endif //BCCLEAN_PROJ_NODE_H