//
// Created by Nathan on 6/24/2026.
//

#ifndef DCEL_H
#define DCEL_H

#include "opengl_include.h"
#include <random>
#include <map>
#include <set>
#include <climits>
#include <deque>

class IDManager {
    std::set<size_t> used_ids;
    std::mt19937 generator;
    std::uniform_int_distribution<size_t> id_range;
    size_t id_randomizer_time;
    size_t id_randomizer_rng;
public:
    IDManager();

    bool id_is_used(size_t id);
    bool add_id(size_t id);
    bool remove_id(size_t id);
    size_t get_unused_id();
    void reset();
    const std::set<size_t>& get_all_ids() { return used_ids; }
};

class DCEL {
    using VertexId = size_t;
    using EdgeId = size_t;
    using ParentEdgeId = size_t;
    using ConnectedSubComponentId = size_t;

    IDManager vertexIdMgr;
    IDManager edgeIdMgr;
    IDManager parentEdgeIdMgr;
    IDManager connectedSubComponentIdMgr;

    const std::set<size_t>& root_vertex_ids() { return root_vertices; }
    const std::set<size_t>& parent_edge_ids() { return parentEdgeIdMgr.get_all_ids(); }

    struct Edge {
        EdgeId id;
        ParentEdgeId parent_id;
        VertexId p;
        VertexId q;
    };

    struct Vertex {
        VertexId id;
        glm::vec2 pos;
        char edge_ct;
        EdgeId edges[4]; // TODO: Support more than 4 edges per vertex

        Vertex(size_t id, glm::vec2 pos): id(id), pos(pos), edge_ct(0),
        edges(UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX) {}
    };

    struct ParentEdge {
        ParentEdgeId id;
        VertexId root_p;
        VertexId root_q;
        std::set<EdgeId> child_edges;
        std::set<VertexId> child_intersecting_vertices;

        void add_edge_child(EdgeId child) { child_edges.emplace(child); }
        void remove_edge_child(EdgeId child) { child_edges.erase(child); }
        void add_intersecting_vertex_child(VertexId child) { child_intersecting_vertices.emplace(child); }
        void remove_intersecting_vertex_child(VertexId child) { child_intersecting_vertices.erase(child); }
    };

    struct ConnectedSubComponent {
        enum Type {
            Vertex = 0,
            EdgeConnecting = 1,
            IntersectionConnecting = 2
        };

        ConnectedSubComponentId id;
        Type ty;

        VertexId vertex_id = UINT64_MAX;
        ParentEdgeId connecting_parent_edge_1 = UINT64_MAX;
        ParentEdgeId connecting_parent_edge_2 = UINT64_MAX;
        ConnectedSubComponentId C1 = UINT64_MAX;
        ConnectedSubComponentId C2 = UINT64_MAX;
        ConnectedSubComponentId parent = UINT64_MAX;
        ConnectedSubComponentId root = UINT64_MAX;

        ConnectedSubComponent(ConnectedSubComponentId id, VertexId vtx): id(id), ty(Vertex), vertex_id(vtx), root(id) {}
        ConnectedSubComponent(ConnectedSubComponentId id, ParentEdgeId connector, ConnectedSubComponentId C1, ConnectedSubComponentId C2) :
        id(id), ty(EdgeConnecting), connecting_parent_edge_1(connector), C1(C1), C2(C2), root(id) {}
        ConnectedSubComponent(ConnectedSubComponentId id, VertexId intersecting_vtx, ParentEdgeId connector_1,
            ParentEdgeId connector_2, ConnectedSubComponentId C1, ConnectedSubComponentId C2) :
        id(id), ty(IntersectionConnecting), vertex_id(intersecting_vtx), connecting_parent_edge_1(connector_1),
        connecting_parent_edge_2(connector_2), C1(C1), C2(C2), root(id) {}
    };

    std::set<VertexId> root_vertices;
    std::set<VertexId> intersect_vertices;
    std::map<VertexId, Vertex> vertices;
    std::map<EdgeId, Edge> edges;
    std::map<ParentEdgeId, ParentEdge> parent_edges;
    std::map<VertexId, ConnectedSubComponentId> vertex_component_linking;
    std::map<ConnectedSubComponentId, ConnectedSubComponent> subcomponents;
    std::set<ConnectedSubComponentId> root_components;

public:
    struct InputGraphData {
        std::vector<glm::vec2> vertices;
        std::vector<std::pair<size_t, size_t>> edges;
    };
private:

    void init(const InputGraphData& graph_data);
    VertexId insert_root_vertex(glm::vec2 pos);
    VertexId insert_intersecting_vertex(glm::vec2 pos, EdgeId e1, EdgeId e2);

    ConnectedSubComponentId insert_root_vertex_connected_sub_component(VertexId vtx);
    ConnectedSubComponentId insert_edge_connected_sub_component(ParentEdgeId peid, ConnectedSubComponentId C1, ConnectedSubComponentId C2);
    ConnectedSubComponentId insert_intersect_connected_sub_component(VertexId intersect_vtx, ParentEdgeId peid_1,
        ParentEdgeId peid_2, ConnectedSubComponentId C1, ConnectedSubComponentId C2);
    ConnectedSubComponentId get_vertex_root_subcomponent(VertexId vtx);
    void update_roots(ConnectedSubComponentId C, ConnectedSubComponentId new_root);
    void set_subcomponent_parent(ConnectedSubComponentId child, ConnectedSubComponentId new_parent);

    EdgeId insert_child_edge(ParentEdgeId peid, VertexId p, VertexId q);
    std::pair<EdgeId, ParentEdgeId> insert_parent_edge(VertexId p, VertexId q);
    void delete_edge(EdgeId edge);
    void attach_edge_to_vertex(EdgeId edge, VertexId vertex);
    void detach_edge_from_vertex(EdgeId edge, VertexId vertex);
    Vertex& get_p(EdgeId edge);
    Vertex& get_q(EdgeId edge);
    Vertex& get_root_p(ParentEdgeId edge);
    Vertex& get_root_q(ParentEdgeId edge);
    ParentEdgeId parent_of(EdgeId edge);
    glm::vec2 get_norm(EdgeId edge);
    bool edges_intersect(EdgeId e1, EdgeId e2, bool include_border=false);
    bool parent_edges_intersect(ParentEdgeId e1, ParentEdgeId e2, bool include_border=false);
    glm::vec2 compute_edge_intersection(EdgeId e1, EdgeId e2);
    glm::vec2 compute_parent_edge_intersection(ParentEdgeId e1, ParentEdgeId e2);
    // void intersect_edges(EdgeId e1, EdgeId e2);
    void intersect_parent_edges(ParentEdgeId e1, ParentEdgeId e2);
    bool edge_bounding_box_contains_point(EdgeId id, glm::vec2 pos);
    EdgeId find_child_edge_of_parent_containing(ParentEdgeId peid, glm::vec2 pos);

public:
    DCEL() = default;
    explicit DCEL(const InputGraphData& graph_data);
    void reset();
    void set_graph(const InputGraphData& graph_data);
};



#endif //DCEL_H
