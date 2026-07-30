//
// Created by Nathan on 6/24/2026.
//

#include "dcel.h"
#include <climits>
#include <chrono>
#include <iostream>

IDManager::IDManager():
used_ids(),
generator((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count()),
id_range(0u, UINT64_MAX-1u),
id_randomizer_time((size_t)std::chrono::high_resolution_clock::now().time_since_epoch().count()),
id_randomizer_rng(id_range(generator)) {}

bool IDManager::id_is_used(size_t id) { return used_ids.contains(id); }

bool IDManager::add_id(size_t id) {
    if (id_is_used(id)) return false;
    used_ids.insert(id);
    return true;
}
bool IDManager::remove_id(size_t id) {
    if (!id_is_used(id)) return false;
    used_ids.erase(id);
    return true;
}

void IDManager::reset() { used_ids.clear(); }

size_t IDManager::get_unused_id() {
    // Change later if this runs into performance issues.
    size_t guess = (UINT_MAX>>1) - used_ids.size();
    guess ^= id_randomizer_rng;
    id_randomizer_rng *= id_randomizer_time;
    if ((guess == UINT64_MAX) || used_ids.contains(guess)) {
        id_randomizer_time = (size_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
        while ((guess == UINT64_MAX) || used_ids.contains(guess)) { guess = 2 * guess + 1; }
    }
    return guess;
}

void DCEL::init(const InputGraphData& graph_data) {
    // Insert vertices
    std::vector<VertexId> temp_internal_vertex_mapping(graph_data.vertices.size());
    for (const auto& vertex : graph_data.vertices) {
        VertexId v_id = insert_root_vertex(vertex);
        temp_internal_vertex_mapping.emplace_back(v_id);
        insert_root_vertex_connected_sub_component(v_id);
    }

    // Insert edges
    for (const auto& [p, q] : graph_data.edges) {
        auto [id, peid] = insert_parent_edge(temp_internal_vertex_mapping[p], temp_internal_vertex_mapping[q]);
        ConnectedSubComponentId p_component = get_vertex_root_subcomponent(p);
        ConnectedSubComponentId q_component = get_vertex_root_subcomponent(q);
        if (p_component != q_component) {
            insert_edge_connected_sub_component(peid, p_component, q_component);
        }
        // TODO: Improve intersection finding algorithm if performance is a problem
        for (const auto& other_peid: parent_edge_ids()) {
            if (other_peid == peid) continue;
            if (parent_edges_intersect(other_peid, peid)) {
                intersect_parent_edges(other_peid, peid);
            }
        }
    }

    // if (line_loop.empty()) return; // Early return if no vertices
    //
    // first_vertex = insert_root_vertex(line_loop[0]);
    // VertexId curr_p = UINT64_MAX;
    // VertexId curr_q = first_vertex;
    //
    // if (line_loop.size() == 1) return; // All that needs to be done if size is 1
    //
    // // Create root vertices and edges for other vertices
    // for (int i = 1; i < line_loop.size(); i++) {
    //     curr_p = curr_q;
    //     curr_q = insert_root_vertex(line_loop[i]);
    //     insert_parent_edge(curr_p, curr_q);
    // }
    //
    // first_edge = vertices[first_vertex].edges[0];
    //
    // if (line_loop.size() == 2) return; // Edge would be redundant if size is exactly 2
    //
    // // Connect last vertex to first vertex
    // curr_p = curr_q;
    // curr_q = first_vertex;
    // insert_parent_edge(curr_p, curr_q);
    // first_edge = vertices[first_vertex].edges[0]; // Redundant, but will help edge cases if parent edge insertion changes the first edge in the future
    //
    // // Intersect edges
    // // TODO: Replace with better algo if performance is an issue
    // const std::set<ParentEdgeId>& pe_ids = parent_edge_ids();
    //
    // auto peid_itt_low = pe_ids.begin();
    // auto peid_itt_high = std::next(peid_itt_low);
    //
    // // Brute force check intersections between all parent edges. Break down edges when they are found.
    // auto last_itt = std::prev(pe_ids.end());
    // while (peid_itt_low != last_itt) {
    //     while (peid_itt_high != pe_ids.end()) {
    //         if (parent_edges_intersect(*peid_itt_low, *peid_itt_high)) {
    //             intersect_parent_edges(*peid_itt_low, *peid_itt_high);
    //         }
    //         peid_itt_high++;
    //     }
    //     peid_itt_low++;
    //     peid_itt_high = std::next(peid_itt_low);
    // }
}

DCEL::DCEL(const InputGraphData& graph_data) {
    init(graph_data);
}

void DCEL::reset() {
    // Reset Id managers
    vertexIdMgr.reset();
    edgeIdMgr.reset();
    parentEdgeIdMgr.reset();
    connectedSubComponentIdMgr.reset();

    // Reset mappings
    root_vertices.clear();
    intersect_vertices.clear();
    vertices.clear();
    edges.clear();
    parent_edges.clear();
    vertex_component_linking.clear();
    subcomponents.clear();
    root_components.clear();
}

void DCEL::set_graph(const InputGraphData& graph_data) {
    reset();
    init(graph_data);
}

DCEL::VertexId DCEL::insert_root_vertex(glm::vec2 pos) {
    VertexId id = vertexIdMgr.get_unused_id();
    vertices.emplace(id, Vertex(id, pos));
    vertexIdMgr.add_id(id);
    root_vertices.emplace(id);
    return id;
}

DCEL::VertexId DCEL::insert_intersecting_vertex(glm::vec2 pos, EdgeId e1, EdgeId e2) {
    VertexId id = vertexIdMgr.get_unused_id();
    vertices.emplace(id, Vertex(id, pos));
    vertexIdMgr.add_id(id);
    parent_edges[parent_of(e1)].add_intersecting_vertex_child(id);
    parent_edges[parent_of(e2)].add_intersecting_vertex_child(id);
    intersect_vertices.emplace(id);
    return id;
}

DCEL::ConnectedSubComponentId DCEL::insert_root_vertex_connected_sub_component(VertexId vtx) {
    ConnectedSubComponentId id = connectedSubComponentIdMgr.get_unused_id();
    subcomponents.emplace(id, ConnectedSubComponent(id, vtx));
    connectedSubComponentIdMgr.add_id(id);
    vertex_component_linking.emplace(vtx, id);
    root_components.emplace(id);
    return id;
}

DCEL::ConnectedSubComponentId DCEL::insert_edge_connected_sub_component(ParentEdgeId peid, ConnectedSubComponentId C1, ConnectedSubComponentId C2) {
    ConnectedSubComponentId id = connectedSubComponentIdMgr.get_unused_id();
    subcomponents.emplace(id, ConnectedSubComponent(id, peid, C1, C2));
    connectedSubComponentIdMgr.add_id(id);
    root_components.erase(C1);
    root_components.erase(C2);
    set_subcomponent_parent(C1, id);
    set_subcomponent_parent(C2, id);
    update_roots(C1, id);
    update_roots(C2, id);
    root_components.emplace(id);
    return id;
}

DCEL::ConnectedSubComponentId DCEL::insert_intersect_connected_sub_component(VertexId intersect_vtx, ParentEdgeId peid_1,
        ParentEdgeId peid_2, ConnectedSubComponentId C1, ConnectedSubComponentId C2) {
    ConnectedSubComponentId id = connectedSubComponentIdMgr.get_unused_id();
    subcomponents.emplace(id, ConnectedSubComponent(id, intersect_vtx, peid_1, peid_2, C1, C2));
    connectedSubComponentIdMgr.add_id(id);
    root_components.erase(C1);
    root_components.erase(C2);
    set_subcomponent_parent(C1, id);
    set_subcomponent_parent(C2, id);
    update_roots(C1, id);
    update_roots(C2, id);
    root_components.emplace(id);
    return id;
}

DCEL::ConnectedSubComponentId DCEL::get_vertex_root_subcomponent(VertexId vtx) {
    return subcomponents[vertex_component_linking[vtx]].root;
}

void DCEL::update_roots(ConnectedSubComponentId C, ConnectedSubComponentId new_root) {
    std::deque<ConnectedSubComponentId> to_update;
    to_update.emplace_back(C);
    while (!to_update.empty()) {
        ConnectedSubComponent& comp = subcomponents[to_update.front()];
        comp.root = new_root;
        if (comp.ty != ConnectedSubComponent::Type::Vertex) {
            to_update.emplace_back(comp.C1);
            to_update.emplace_back(comp.C2);
        }
        to_update.pop_front();
    }
}

void DCEL::set_subcomponent_parent(ConnectedSubComponentId child, ConnectedSubComponentId new_parent) {
    subcomponents[child].parent = new_parent;
}

DCEL::EdgeId DCEL::insert_child_edge(ParentEdgeId peid, VertexId p, VertexId q) {
    EdgeId id = edgeIdMgr.get_unused_id();
    edges.emplace(id, Edge(id, peid, p, q));
    attach_edge_to_vertex(id, p);
    attach_edge_to_vertex(id, q);
    edgeIdMgr.add_id(id);
    parent_edges[peid].add_edge_child(id);
    return id;
}

std::pair<DCEL::EdgeId, DCEL::ParentEdgeId> DCEL::insert_parent_edge(VertexId p, VertexId q) {
    EdgeId id = edgeIdMgr.get_unused_id();
    ParentEdgeId peid = parentEdgeIdMgr.get_unused_id();

    edges.emplace(id, Edge(id, peid, p, q));

    parent_edges.emplace(peid, ParentEdge{peid, p, q});
    parent_edges[peid].add_edge_child(id);

    attach_edge_to_vertex(id, p);
    attach_edge_to_vertex(id, q);

    edgeIdMgr.add_id(id);
    parentEdgeIdMgr.add_id(peid);

    return {id, peid};
}

void DCEL::attach_edge_to_vertex(EdgeId edge, VertexId vertex) {
    Vertex& v = vertices[vertex];
    v.edges[v.edge_ct] = edge;
    v.edge_ct++;
}

void DCEL::detach_edge_from_vertex(EdgeId edge, VertexId vertex) {
    Vertex& vtx = vertices[vertex];
    size_t edge_idx =
        vtx.edges[0] == edge ? 0 :
        vtx.edges[1] == edge ? 1 :
        vtx.edges[2] == edge ? 2 : 3;
    switch (edge_idx) {
    case 0: vtx.edges[0] = vtx.edges[1];
    case 1: vtx.edges[1] = vtx.edges[2];
    case 2: vtx.edges[2] = vtx.edges[3];
    default: break;
    }
    vtx.edges[3] = UINT64_MAX;
    vtx.edge_ct--;
}

DCEL::Vertex& DCEL::get_p(EdgeId edge) { return vertices[edges[edge].p]; }
DCEL::Vertex& DCEL::get_q(EdgeId edge) { return vertices[edges[edge].q]; }
DCEL::Vertex& DCEL::get_root_p(ParentEdgeId edge) { return vertices[parent_edges[edge].root_p]; }
DCEL::Vertex& DCEL::get_root_q(ParentEdgeId edge) { return vertices[parent_edges[edge].root_q]; }

glm::vec2 DCEL::get_norm(EdgeId edge) {
    Edge& e = edges[edge];
    glm::vec2 dir = vertices[e.q].pos - vertices[e.p].pos;
    return glm::normalize(glm::vec2{dir.y, -dir.x});
}

bool DCEL::edges_intersect(EdgeId e1, EdgeId e2, bool include_border) {
    if (e1 == e2) return include_border;
    if (edges[e1].p == edges[e2].p || edges[e1].p == edges[e2].q ||
        edges[e1].q == edges[e2].p || edges[e1].q == edges[e2].q) {
        return include_border;
    }

    glm::vec2 p1 = get_p(e1).pos;
    glm::vec2 p2 = get_p(e2).pos;

    glm::vec2 q1 = get_q(e1).pos;
    glm::vec2 q2 = get_q(e2).pos;

    glm::vec2 norm1 = get_norm(e1);
    glm::vec2 norm2 = get_norm(e2);

    bool e2_crosses_e1_line = glm::sign(glm::dot(norm1, p2-p1)) != glm::sign(glm::dot(norm1, q2-p1));
    bool e1_crosses_e2_line = glm::sign(glm::dot(norm2, p1-p2)) != glm::sign(glm::dot(norm2, q1-p2));
    return e2_crosses_e1_line && e1_crosses_e2_line;
}

bool DCEL::parent_edges_intersect(ParentEdgeId e1, ParentEdgeId e2, bool include_border) {
    if (e1 == e2) return include_border;
    if (parent_edges[e1].root_p == parent_edges[e2].root_p || parent_edges[e1].root_p == parent_edges[e2].root_q ||
        parent_edges[e1].root_q == parent_edges[e2].root_p || parent_edges[e1].root_q == parent_edges[e2].root_q) {
        return include_border;
        }

    glm::vec2 p1 = get_root_p(e1).pos;
    glm::vec2 p2 = get_root_p(e2).pos;

    glm::vec2 q1 = get_root_q(e1).pos;
    glm::vec2 q2 = get_root_q(e2).pos;

    glm::vec2 norm1 = get_norm(e1);
    glm::vec2 norm2 = get_norm(e2);

    bool e2_crosses_e1_line = glm::sign(glm::dot(norm1, p2-p1)) != glm::sign(glm::dot(norm1, q2-p1));
    bool e1_crosses_e2_line = glm::sign(glm::dot(norm2, p1-p2)) != glm::sign(glm::dot(norm2, q1-p2));
    return e2_crosses_e1_line && e1_crosses_e2_line;
}

glm::vec2 DCEL::compute_edge_intersection(EdgeId e1, EdgeId e2) {
    glm::vec2 p1 = get_p(e1).pos;
    glm::vec2 p2 = get_p(e2).pos;

    glm::vec2 q1 = get_q(e1).pos;
    glm::vec2 q2 = get_q(e2).pos;

    glm::vec2 d1 = q1 - p1;
    glm::vec2 d2 = q2 - p2;
    glm::vec2 d3 = p2 - p1;

    float s_num = (d1.y * d3.x) - (d1.x * d3.y);
    float s_den = (d1.x * d2.y) - (d1.y * d2.x);

    return (q2 - p2) * (s_num / s_den) + p2;
}

glm::vec2 DCEL::compute_parent_edge_intersection(ParentEdgeId e1, ParentEdgeId e2) {
    glm::vec2 p1 = get_root_p(e1).pos;
    glm::vec2 p2 = get_root_p(e2).pos;

    glm::vec2 q1 = get_root_q(e1).pos;
    glm::vec2 q2 = get_root_q(e2).pos;

    glm::vec2 d1 = q1 - p1;
    glm::vec2 d2 = q2 - p2;
    glm::vec2 d3 = p2 - p1;

    float s_num = (d1.y * d3.x) - (d1.x * d3.y);
    float s_den = (d1.x * d2.y) - (d1.y * d2.x);

    return (q2 - p2) * (s_num / s_den) + p2;
}

// void DCEL::intersect_edges(EdgeId e1, EdgeId e2) {
//     // TODO: If it lands on an existing vertex, add edges to that one instead
//     VertexId new_vtx = insert_intersecting_vertex(compute_edge_intersection(e1, e2), e1, e2);
//
//     // Backup edge info because we're about to delete
//     Edge edge_1_cpy = edges[e1];
//     Edge edge_2_cpy = edges[e2];
//
//     // Delete old two edges
//     delete_edge(e1);
//     delete_edge(e2);
//
//     // Insert new edges
//     insert_child_edge(edge_1_cpy.parent_id, edge_1_cpy.p, new_vtx);
//     insert_child_edge(edge_1_cpy.parent_id, new_vtx, edge_1_cpy.q);
//     insert_child_edge(edge_2_cpy.parent_id, edge_2_cpy.p, new_vtx);
//     insert_child_edge(edge_2_cpy.parent_id, new_vtx, edge_2_cpy.q);
//
//     // // In case the original first edge was deleted.
//     // first_edge = vertices[first_edge].edges[0];
// }

void DCEL::intersect_parent_edges(ParentEdgeId e1, ParentEdgeId e2) {
    glm::vec2 parent_intersection = compute_parent_edge_intersection(e1, e2);
    EdgeId intersecting_edge_1 = find_child_edge_of_parent_containing(e1, parent_intersection);
    EdgeId intersecting_edge_2 = find_child_edge_of_parent_containing(e2, parent_intersection);

    // TODO: If it lands on an existing vertex, add edges to that one instead
    VertexId new_vtx = insert_intersecting_vertex(parent_intersection, intersecting_edge_1, intersecting_edge_2);

    // Backup edge info because we're about to delete
    Edge edge_1_cpy = edges[intersecting_edge_1];
    Edge edge_2_cpy = edges[intersecting_edge_2];

    // Delete old two edges
    delete_edge(intersecting_edge_1);
    delete_edge(intersecting_edge_2);

    // Insert new edges
    insert_child_edge(e1, edge_1_cpy.p, new_vtx);
    insert_child_edge(e1, new_vtx, edge_1_cpy.q);
    insert_child_edge(e2, edge_2_cpy.p, new_vtx);
    insert_child_edge(e2, new_vtx, edge_2_cpy.q);

    // Connect components if relevant
    ConnectedSubComponentId root_component_e1 = get_vertex_root_subcomponent(get_root_p(e1).id);
    ConnectedSubComponentId root_component_e2 = get_vertex_root_subcomponent(get_root_p(e2).id);
    if (root_component_e1 != root_component_e2) {
        insert_intersect_connected_sub_component(new_vtx, e1, e2, root_component_e1, root_component_e2);
    }

    // // In case the original first edge was deleted.
    // first_edge = vertices[first_edge].edges[0];
}

void DCEL::delete_edge(EdgeId edge) {
    detach_edge_from_vertex(edge, edges[edge].p);
    detach_edge_from_vertex(edge, edges[edge].q);
    parent_edges[parent_of(edge)].remove_edge_child(edge);
    edges.erase(edge);
    edgeIdMgr.remove_id(edge);
}

DCEL::ParentEdgeId DCEL::parent_of(EdgeId edge) {
    return edges[edge].parent_id;
}

bool DCEL::edge_bounding_box_contains_point(EdgeId id, glm::vec2 pos) {
    const glm::vec2& p = get_p(id).pos;
    const glm::vec2& q = get_q(id).pos;
    // TODO: Fix possible floating point error resulting in false negatives for small/skinny bounding boxes
    return ((std::min(p.x, q.x) <= pos.x) && (pos.x <= std::max(p.x, q.x)) &&
            (std::min(p.y, q.y) <= pos.y) && (pos.y <= std::max(p.y, q.y)));
}

DCEL::EdgeId DCEL::find_child_edge_of_parent_containing(ParentEdgeId peid, glm::vec2 pos) {
    const ParentEdge& parent = parent_edges[peid];
    for (const auto& child_id: parent.child_edges) {
        if (edge_bounding_box_contains_point(child_id, pos)) return child_id;
    }
    return UINT64_MAX;
}
