#include "MatchMakerTree.h"
#include "Kruskal.h"
#include "backtrack_diff.h"
#include "polyline_distance.h"
#include "loop_colorize.h"
#include "params.h"
#include <igl/Timer.h>
#include <climits>
#include <list>
#include <deque>
#include <igl/bounding_box_diagonal.h>
#include <igl/list_to_matrix.h>
#include <igl/matrix_to_list.h>
#include <igl/barycentric_to_global.h>
#include <igl/writePLY.h>
#include <igl/per_vertex_normals.h>
#include <algorithm>
#include "Edge_Dijkstra.h"
#include "CellularGraph.h"
#include "MatchMakerDynamic.h"
#include "TraceComplex.h"
#include "TransferCellGraph.h"
#include "Patch_Bijection.h"
#include "helper.h"

/* BTCMM means backtracking cellular matchmaker */
namespace bcclean{
namespace MatchMaker{
    using json = nlohmann::json;

    void writePlyColor(std::string name,const Eigen::MatrixXd & Vsoup, const Eigen::MatrixXi & Fsoup, const Eigen::MatrixXd & Nsoup, const Eigen::MatrixXi Csoup)

    {
        std::ofstream file;
        file.open(name);
        file << "ply\n";
        file << "format ascii 1.0\n";
        file << "element vertex " <<Vsoup.rows() << "\n";
        file << "property double x\n";
        file <<"property double y\n";
        file << "property double z\n";
        file <<"property double nx\n";
        file <<"property double ny\n";
        file <<"property double nz\n";
        file <<"property uchar red\n";
        file <<"property uchar green\n";
        file <<"property uchar blue\n";
        file <<"element face " << Fsoup.rows() << "\n";
        file <<"property list uchar int vertex_indices\n";
        file <<"end_header\n";
        for(int vidx =0 ; vidx < Vsoup.rows(); ++vidx)
        {
            file << Vsoup(vidx,0) <<" ";
            file << Vsoup(vidx,1) <<" ";
            file << Vsoup(vidx,2) <<" ";
            file << Nsoup(vidx,0) <<" ";
            file << Nsoup(vidx,1) <<" ";
            file << Nsoup(vidx,2) <<" ";
            file << Csoup(vidx,0) <<" ";
            file << Csoup(vidx,1) <<" ";
            file << Csoup(vidx,2) <<"\n";
        }
        for(int fidx = 0; fidx < Fsoup.rows(); ++fidx)
        {
            file << 3 << " ";
            file << Fsoup(fidx,0) << " ";
            file << Fsoup(fidx,1) << " ";
            file << Fsoup(fidx,2) << "\n";
        }
    }

    void build_dual_frame_graph(
        const std::vector<bcclean::edge> edge_list,
        std::vector<std::pair<int, std::pair<int, int> > > & dual_frame_graph
    )
    {
        dual_frame_graph.clear();
        int edge_idx = 0;
        for(auto edge: edge_list)
        {
            dual_frame_graph.push_back(std::make_pair(edge_idx, edge._label_pair));
            edge_idx++;
        }
        return;
    }


    bool MatchMakerPatch_for_edge(
        const CellularGraph & cg,
        TraceComplex & tc,
        const int edge_idx,
        std::map<int, std::map<int, bool> > & node_edge_visit_dict,
        json & path_json,
        const params param
    )
    {
        
        edge edg = cg._edge_list[edge_idx];
        int target =-1;
        int target_bad = -1;
        int source_bad = -1;
        int source = tc._node_image_map.at(edg.head);     
        source_bad = edg.head;
        target = tc._node_image_map.at(edg.tail);
        target_bad = edg.tail;
        assert(target != -1 && source != -1); 
        // update local sector
        // (a) create CC_node_face_list
        std::map<int, std::vector<int> > CC_node_face_dict;
        std::vector<std::vector<int > > VV_temp,VV_temp1;
        CCfaces_per_node(tc._V,tc._F, tc._VF,{source,target},CC_node_face_dict);
        {
            std::map<int, std::map<int, bool> > node_edge_visit_dict_temp;
            std::map<int, std::vector<int> > node_edge_dict_temp;
            for(auto item: cg._node_edge_dict)
            {
                if(tc._node_image_map.find(item.first)!=tc._node_image_map.end()){
                    node_edge_dict_temp[tc._node_image_map.at(item.first)] = cg._node_edge_dict.at(item.first);
                    node_edge_visit_dict_temp[tc._node_image_map.at(item.first)] = node_edge_visit_dict.at(item.first);
                }
            }
            update_local_sector(
                tc._VV,
                tc._F,
                node_edge_visit_dict_temp, 
                node_edge_dict_temp,
                tc._TEdges,
                CC_node_face_dict,
                source,
                target,
                edge_idx,
                VV_temp1
            );
        }
        // the Weights is edge based
        Eigen::SparseMatrix<double> SpWeight;
        Trace::setWeight1(tc._V, tc._F, cg._vertices,edg,SpWeight);
        std::vector<int> path;
        Trace::Edge_Dijkstra(VV_temp1, target, source, SpWeight,path);// because the path will be inverted, we switch the position of source and target
        // dijkstra_trace(VV_temp, source, target, Weights, path);
        if(param.debug)
        {
            Eigen::VectorXd source_target=Eigen::VectorXd::Constant(6,0);
            for(int xx: {0,1,2})
            {
                source_target(xx)=tc._V[source](xx);
                source_target(xx+3)=tc._V[target](xx);
            }
            
            igl::writeDMAT(param.data_root+"/source_target.dmat",source_target);
        }
        // assert(path.size()>=2);
        if(path.size()<2)
        {
            std::cout << "for path" << edge_idx << std::endl;
            std::cout << "start" << source  << "target" << target << std::endl;
            std::cout << "path of size only "<< path.size() << std::endl;
            return false;
        }
        std::vector<int> path_records(path.size()-2);
        std::printf("for edge %d, find a path:\n",edge_idx);
        for(auto rec : path)
        {
            std::cout << rec<<", ";
        }
        std::cout << "\n";
        for(int p =0 ; p < path.size()-2; ++p)
        {
            path_records[p] = path[p+1];
        }
        // path update VV
        silence_vertices(tc._VV, path_records);
        for(auto rec: path_records)
        {
            tc._total_silence_list.push_back(rec);
        }
        //path update VEdges
        for(auto vidx:path_records){
            tc._VEdges[vidx].push_back(edge_idx);
        }

        // path updates TEdges
        // set the triangle edges in cuts to be true
        for(int rc_idx=0; rc_idx < path.size()-1; ++rc_idx){
            int uidx = path[rc_idx];
            int vidx = path[(rc_idx+1)%path.size()];
            std::vector<int> vtr = tc._VF[vidx];
            std::vector<int> utr = tc._VF[uidx];
            std::sort(vtr.begin(), vtr.end());
            std::sort(utr.begin(), utr.end());
            std::vector<int> inter(vtr.size()+utr.size());
            auto it = std::set_intersection(vtr.begin(), vtr.end(), utr.begin(), utr.end(), inter.begin());
            inter.resize(it-inter.begin());
            // there should be only one comman adjacent triangle for boundary vertices
            for(auto trg: inter){
                for(int edgpos =0; edgpos < 3 ; ++edgpos){
                    int uuidx = tc._F[trg]( edgpos);
                    int vvidx = tc._F[trg]( (edgpos+1)% 3);
                    if(uuidx == uidx && vvidx == vidx){
                        tc._TEdges[trg][edgpos] = edge_idx;
                    }
                    if(uuidx == vidx && vvidx == uidx){
                        tc._TEdges[trg][edgpos] = edge_idx;
                    }
                }
            }
        }

        // splits_detect

        std::vector<std::pair<int, int> > splits;
        tc.splits_detect(splits);
        for(auto split : splits)
        {
            // splits_update
            tc.split_update(split);
        }
        silence_vertices(tc._VV, tc._total_silence_list);
        tc._edge_path_map[edge_idx] = path;
        if(param.debug)
        {
            path_json[std::to_string(edge_idx)] = path;   
            std::ofstream file;
            file.open(param.data_root+"/debug_paths.json");
            file << path_json;
            Eigen::MatrixXi F;
            Eigen::MatrixXd V;
            Helper::to_matrix(tc._F, F);
            Helper::to_matrix(tc._V, V);
            igl::writeOBJ(param.data_root+"/debug_mesh.obj", V, F);
            igl::writeDMAT(param.data_root+"/FL_loop.dmat", tc._FL);
        }
        // update visit_dict or loop condition update
        node_edge_visit_dict[target_bad][edge_idx]=true;
        node_edge_visit_dict[source_bad][edge_idx]=true;
        return true;
    }






    bool MatchMakerPatch(
        const CellularGraph & cg,
        Eigen::MatrixXd & V_good,
        Eigen::MatrixXi & F_good,
        Eigen::VectorXi & FL_good,
        const params param,
        std::shared_ptr<spdlog::logger> logger,
        result_measure & rm
    )
    {


        // datas dump to file for debug
        std::map<int, int> edge_order_map; // store and maintain the order of added edges {order: edge_dx}
        int total_label_num = cg.label_num;
        // PART 0 GET THE FRAME ON BAD MESH
        std::vector<bcclean::edge> edge_list = cg._edge_list;
        // std::unordered_map<int, std::vector<int> > patch_edge_dict;
        // std::unordered_map<int, std::vector<bool> > patch_edge_direction_dict;
        if(param.debug)
        {
            igl::writeOBJ(param.data_root+"/debug_mesh_bad.obj", cg.V, cg.F);
            igl::writeDMAT(param.data_root+"/FL_bad.dmat",cg.FL);
        }
        int largest_patch = cg.root_cell;
        // build_edge_list_loop(V_bad, F_bad, FL_bad, total_label_num, edge_list, patch_edge_dict, patch_edge_direction_dict,largest_patch);
        {
            json path_json_bad;
            int edg_idx =0;
            for(auto edg: cg._edge_list)
            {   


                std::vector<int> path_raw;
                for(auto vidx: edg._edge_vertices)
                {
                    path_raw.push_back(cg._ivmap.at(vidx));
                }
                path_json_bad[std::to_string(edg_idx)]= path_raw;
                edg_idx += 1;
            }
            std::ofstream file;
            file.open(param.data_root+"/debug_path_bad.json");
            file << path_json_bad;
        }
        std::vector<std::pair<int, std::pair<int, int> > > dual_frame_graph;

        build_dual_frame_graph(cg._edge_list, dual_frame_graph);

        std::vector<int> patch_order = Algo::Graph_BFS(dual_frame_graph, largest_patch);
        rm.remain_patch_num = patch_order.size();

        TraceComplex tc;
        tc.initialize(V_good,F_good);

        json path_json;


        std::map<int, std::map<int, bool> >  node_edge_visit_dict;
        for(auto nd: cg._nodes)
        {
            for(auto q: cg._node_edge_dict.at(nd))
            {
                node_edge_visit_dict[nd][q]=false;
            }
        }
        std::map<int, bool> edge_visit_dict;
        std::map<int, double> edge_len_dict;
        int kkk = 0;
        for(auto edg: cg._edge_list)
        {
            edge_visit_dict[kkk]= false;
            kkk++;
        }
        std::list<int> patch_queue(patch_order.begin(), patch_order.end());
        std::list<int> recycle;
        int switch_count  = 0;
        double bcthreshold = param.backtrack_threshold;
        double arthreshold = param.area_threshold;
        while(! patch_queue.empty())
        {
            bool curpatch_succ = true;  
            /* copy part */
            TraceComplex tc_copy = tc;
            result_measure rm_copy = rm;
            
            
            std::map<int, bool> edge_visit_dict_copy = edge_visit_dict;
            std::map<int, double> edge_len_dict_copy = edge_len_dict;
            std::map<int , std::map<int, bool> > node_edge_visit_dict_copy = node_edge_visit_dict; 
            
             
            json path_json_copy = path_json;

            // copy everything in advance , if backtrack happens 
            // replace the origin object with the copies



            int patch_idx = patch_queue.front();
            patch_queue.pop_front();
            double target_len =0;
            double cur_len = 0;
            for(auto edge_idx: cg._patch_edge_dict.at(patch_idx))
            {
                target_len += path_len(cg._vertices, cg._edge_list.at(edge_idx)._edge_vertices);
            }

            if(param.debug)
            {
                igl::writeDMAT(param.data_root+"/cur_patch.dmat", Eigen::VectorXi::Constant(1,patch_idx));

            }
            bool all_edge_traced = true;
            for(auto edge_idx: cg._patch_edge_dict.at(patch_idx))
            {
                if(edge_visit_dict[edge_idx])
                {
                    cur_len += path_len(tc._V, tc._edge_path_map.at(edge_idx));
                    continue;
                }
                all_edge_traced = false;

                edge_visit_dict[edge_idx] = true;
                int source_bad = cg._edge_list.at(edge_idx).head;
                int target_bad = cg._edge_list.at(edge_idx).tail;
                if(param.debug)
                {
                    Eigen::VectorXd source_target_bad = Eigen::VectorXd::Constant(6,0);
                    for(int xx: {0,1,2})
                    {
                        source_target_bad(xx)=cg._vertices[source_bad](xx);
                        source_target_bad(xx+3)=cg._vertices[target_bad](xx);
                    }
                    
                    igl::writeDMAT(param.data_root+"/source_target_bad.dmat",source_target_bad);
                }
                //
                //
                int source = -1;
                int target  = -1;

                
                
                if(tc._node_image_map.find(source_bad)==tc._node_image_map.end())
                {
                    proj_node_loop(cg, source_bad, tc, source); 
                    // updated the tc._node_listd and tc._node_image_map;
                    
                    assert(source!= -1);
                    
                    

                    std::vector<std::pair<int, int> > splits;
                    tc.splits_detect(splits);
                    for(auto split : splits)
                    {
                        // splits_update
                        tc.split_update(split);
                    }
                    
                   
                }
                if(tc._node_image_map.find(target_bad)==tc._node_image_map.end())
                {
        
                    proj_node_loop(cg, target_bad, tc, target);
                    assert(target!= -1);
                    std::vector<std::pair<int, int> > splits;
                    tc.splits_detect(splits);
                    for(auto split : splits)
                    {
                        // splits_update
                        tc.split_update(split);
                    }

                }




                //
                if(!MatchMakerPatch_for_edge(
                    cg,
                    tc,
                    edge_idx, 
                    node_edge_visit_dict, 
                    path_json,
                    param)) 
                    {
                        // roll back current patch
                        curpatch_succ = false;
                        break;
                    }
                else {
                    cur_len += path_len(tc._V, tc._edge_path_map.at(edge_idx));
                    if(cur_len/target_len -1 > bcthreshold && switch_count < 5)
                    {
                        curpatch_succ = false;
                        break;
                    }
                }
            
            }
            
            bool withinthreshold= false; 
            if(curpatch_succ){
                // if the current is traced successfully, we go on to compare the difference
                int ff_in = _locate_seed_face(cg, tc, patch_idx);
                double newarea = (loop_colorize(tc._V, tc._F, tc._TEdges, ff_in, patch_idx, tc._FL)).second;
                double target_area = cg._patch_area_dict.at(patch_idx);
                double area_rel_diff = std::abs(newarea/target_area-1);
                bool area_withinthreshold = ( area_rel_diff< arthreshold);
                double perimeter_rel_diff=backtrack_diff(
                    tc._V,
                    cg,
                    patch_idx,
                    tc._edge_path_map
                );
                rm.area_rel_diff = std::max(rm.area_rel_diff, area_rel_diff);
                rm.peri_rel_diff = std::max(rm.peri_rel_diff, perimeter_rel_diff);
                rm.remain_patch_num -= 1;
                withinthreshold = ((perimeter_rel_diff < bcthreshold) && area_withinthreshold) || (switch_count > 5);
            }
            if(!withinthreshold && !(all_edge_traced))
            {

                // if all_edge_traced, we don't care about the thresholds
                // the traced patch error is larget than the backtrack_threshold
                // abort the result in this loop
                recycle.push_back(patch_idx);
                // reverse copy everything
                std::cout << "patch" << patch_idx << "postponed" << std::endl;

                /* copy part*/
                /* copy part */
                tc = tc_copy;
                rm = rm_copy;
                edge_visit_dict = edge_visit_dict_copy;
                node_edge_visit_dict = node_edge_visit_dict_copy;
                path_json = path_json_copy;
            }
            else
            {
                if(param.debug)
                {
                    Eigen::MatrixXi F;
                    Helper::to_matrix(tc._F, F);
                    Eigen::MatrixXd V;
                    Helper::to_matrix(tc._V, V);
                    igl::writeOBJ(param.data_root+"/debug_mesh.obj", V, F);
                    igl::writeDMAT(param.data_root+"/FL_loop.dmat", tc._FL);
                }

                // loop over patch_queue and  recycle to find patches that where all edges has been traced
                // and place them at the front of patch_queue
                std::vector<int> q_relocate_list;
                std::vector<int> r_relocate_list;
                for(auto pidx: patch_queue)
                {
                    bool p_finished = true;
                    for(auto eidx: cg._patch_edge_dict.at(pidx))
                    {
                        if(!edge_visit_dict.at(eidx))
                        {
                            p_finished = false;
                            break;
                        }
                    }
                    if(p_finished)
                    {
                        q_relocate_list.push_back(pidx);
                    }
                }
               for(auto pidx: recycle)
                {
                    bool p_finished = true;
                    for(auto eidx: cg._patch_edge_dict.at(pidx))
                    {
                        if(!edge_visit_dict.at(eidx))
                        {
                            p_finished = false;
                            break;
                        }
                    }
                    if(p_finished)
                    {
                        r_relocate_list.push_back(pidx);
                    }
                } 
                for(auto pidx: q_relocate_list)
                {
                    patch_queue.remove(pidx);
                }
                for(auto pidx: r_relocate_list)
                {
                    recycle.remove(pidx);
                }
                for(auto pidx: q_relocate_list)
                {
                    patch_queue.push_front(pidx);
                }
                for(auto pidx: r_relocate_list)
                {
                    patch_queue.push_front(pidx);
                }

            }
            if(patch_queue.empty() && !recycle.empty())
            {
                // if it is still empty after relocation 
                // switch recycle and patch_queue
                if(switch_count < 5)
                {
                    std::list<int> temp = recycle;
                    recycle = patch_queue;
                    patch_queue = temp;
                    bcthreshold = 1.5 * bcthreshold;
                    arthreshold = 1.5 * arthreshold;
                    switch_count +=1;
                }
                else if (switch_count < 10)
                {
                    std::list<int> temp = recycle;
                    recycle = patch_queue;
                    patch_queue = temp;
                    switch_count +=1;
                }
            }

        }
        Helper::to_matrix(tc._F,F_good);
        Helper::to_matrix(tc._V, V_good);
        igl::list_to_matrix(tc._FL,FL_good);
        if(param.debug)
        { 
            std::ofstream file;
            file.open(param.data_root+"/debug_paths.json");
            file << path_json;
            Eigen::MatrixXi F;
            igl::writeOBJ(param.data_root+"/debug_mesh.obj", V_good, F_good);
            igl::writeDMAT(param.data_root+"/FL_loop.dmat", FL_good);
            igl::writeDMAT(param.data_root+"/splits_record.dmat", tc._splits_record, false);
            std::ofstream split_file;
            split_file.open(param.data_root+"/splits_record.txt");
            for(auto & vec: tc._splits_record)
            { 
                for(auto & val: vec)
                {
                    split_file << val;
                    split_file << " ";
                }
                split_file << "\n";
            }
        }
        if(recycle.empty()){
            try{
                CellularGraph cgt;
                Bijection::TransferCellGraph(cg, tc, cgt);
                Eigen::MatrixXd M_t2s;
                Bijection::BijGlobal(cgt,cg, M_t2s);
                Eigen::MatrixXd textureC = Eigen::MatrixXd::Constant(cg.V.rows(),3,0);
                Eigen::MatrixXd textureCt = Eigen::MatrixXd::Constant(cgt.V.rows(),3,0);
                double h = igl::bounding_box_diagonal(cg.V);
                int p = 20;
                for(int vidx = 0; vidx < cg.V.rows(); ++ vidx)
                {
                    textureC.row(vidx) = 255 * Eigen::RowVector3d(std::pow(std::sin(p* cg.V(vidx,0)/h),2), std::pow(std::sin(p* cg.V(vidx,1)),2), std::pow(std::sin(p * cg.V(vidx, 2)),2));
                }

                textureCt = igl::barycentric_to_global(textureC, cg.F, M_t2s);
                Eigen::MatrixXd Nt;
                igl::per_vertex_normals(cgt.V,cgt.F, Nt);
                Eigen::MatrixXi textureCtI = Eigen::MatrixXi::Constant(textureCt.rows(),3,0);
                for(int vidx = 0 ; vidx < textureCtI.rows(); ++vidx)
                {
                    textureCtI.row(vidx) = Eigen::RowVector3i((int)textureCt(vidx,0), (int)textureCt(vidx,1),(int)textureCt(vidx,2));
                }
                writePlyColor(param.data_root+"/map.ply",cgt.V, cgt.F,Nt, textureCtI);
                // Eigen::MatrixXd Vmap=igl::barycentric_to_global(cgt.V, cgt.F, M_s2t);
                // igl::writeOBJ(param.data_root+"/map.obj", Vmap, cg.F);

            }
            catch(...)
            {
                std::cout << " textture gen failed" << std::endl;
            }
            rm.remain_patch_num = 0;
            return true;
        }
        else {
            return false;
            rm.remain_patch_num = recycle.size();
        }

    }

}
}