#ifndef BCCLEAN_POLYLINEDIS_H
#define BCCLEAN_POLYLINEDIS_H
#include <Eigen/Dense>
#include <vector>
namespace bcclean{
namespace Eval{
        double hausdorff1d(
            const Eigen::MatrixXd & VA,
            const std::vector<int> & pathA,
            const Eigen::MatrixXd & VB,
            const std::vector<int> & pathB
        );  
        
        double hausdorff1d(
            const std::vector<Eigen::RowVector3d> & VA,
            const std::vector<int> & pathA,
            const Eigen::MatrixXd & VB,
            const std::vector<int> & pathB
        );

        
        double single_sample_trial(
            const Eigen::MatrixXd & VA,
            const std::vector<int> & pathA,
            const Eigen::MatrixXd & VB,
            const int & idxB
        );   
}
}
#endif